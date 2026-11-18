#include "AI4PMCode_Embedded3D.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace std;

static int embedded3dMsgCount = 0;
int AI4PMCode_Embedded3D::currentPhase = 0;

AI4PMCode_Embedded3D::AI4PMCode_Embedded3D(BlinkyBlocksBlock *host)
    : BlinkyBlocksBlockCode(host), module(host)
{
    if (!host) return;
    addMessageEventFunc2(FEATURE_MSG_ID,
        std::bind(&AI4PMCode_Embedded3D::handleFeatureMsg, this,
                  std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(TREE_MSG_ID,
        std::bind(&AI4PMCode_Embedded3D::handleTreeMsg, this,
                  std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(TREE_REJECT_MSG_ID,
        std::bind(&AI4PMCode_Embedded3D::handleTreeRejectMsg, this,
                  std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(AGGREGATE_MSG_ID,
        std::bind(&AI4PMCode_Embedded3D::handleAggregateMsg, this,
                  std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(RESULT_MSG_ID,
        std::bind(&AI4PMCode_Embedded3D::handleResultMsg, this,
                  std::placeholders::_1, std::placeholders::_2));

    // Initialize all state
    memset(occupancy, 0, sizeof(occupancy));
    memset(conv1_out, 0, sizeof(conv1_out));
    numConnected = 0;
    featuresReceived = 0;
    memset(neighbor_features, 0, sizeof(neighbor_features));
    memset(neighborPresent, 0, sizeof(neighborPresent));
    memset(conv2_out, 0, sizeof(conv2_out));
    conv2Done = false;

    role = ROLE_UNDETERMINED;
    inTree = false;
    parentDirection = -1;
    numExpectedChildren = 0;
    childrenDone = 0;
    rejectsReceived = 0;
    memset(isChild, 0, sizeof(isChild));

    memset(aggSum, 0, sizeof(aggSum));
    aggCount = 0;
    aggregationSent = false;

    predictedClass = -1;
    memset(probabilities, 0, sizeof(probabilities));
}

// HELPER: Identify which direction (0-5) an interface corresponds to

int AI4PMCode_Embedded3D::getDirection(P2PNetworkInterface* iface) {
    for (int d = 0; d < 6; d++) {
        if (module->getInterface((SCLattice::Direction)d) == iface) {
            return d;
        }
    }
    return -1;
}

// PHASE 1: Local 3D Feature Extraction (ZERO messages)
// Each block queries its 3×3×3 lattice neighborhood and applies Conv1.

void AI4PMCode_Embedded3D::computeConv1() {
    currentPhase = 1;

    auto* lattice = BaseSimulator::getWorld()->lattice;
    int mx = module->position[0];
    int my = module->position[1];
    int mz = module->position[2];

    // Build 3×3×3 occupancy tensor from lattice queries
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dz = -1; dz <= 1; dz++) {
                Cell3DPosition pos(mx + dx, my + dy, mz + dz);
                bool hasBlock = lattice->cellHasBlock(pos);
                occupancy[dx+1][dy+1][dz+1] = hasBlock ? 1.0f : 0.0f;
                
                // DEBUG: Prove Receptive Field Expansion
                // If I am at (1,0,0), and I query (2,0,0)...
                if (mx == 1 && my == 0 && mz == 0 && dx == 1 && dy == 0 && dz == 0) {
                    cout << "Block (1,0,0) successfully scanned neighbor at (2,0,0). Result: "
                         << (hasBlock ? "BLOCK" : "EMPTY") << endl;
                }
            }
        }
    }

    // Apply Conv1: 3×3×3 convolution → K=8 features
    for (int f = 0; f < EMB_K; f++) {
        float sum = conv1_3d_bias[f];
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                for (int k = 0; k < 3; k++)
                {
                    sum += occupancy[i][j][k] * conv1_3d_kernel[f][0][i][j][k];
                    module->setColor(Color(GREEN));
                }
        conv1_out[f] = (sum > 0.0f) ? sum : 0.0f;  // ReLU
    }

    // Store own features in slot 0
    for (int i = 0; i < EMB_K; i++)
        neighbor_features[0][i] = conv1_out[i];
}

// STARTUP: Compute features, send to neighbors, initiate tree

void AI4PMCode_Embedded3D::startup() {
    // Color blocks white initially
    module->setColor(Color(1.0f, 1.0f, 1.0f));
    //console << "Module " << module->blockId << " initialized. Starting Phase 1\n";

    // Phase 1: Local feature extraction (0 messages)
    computeConv1();
    //console << "Module " << module->blockId << " Phase 1 complete (Local Features extracted)\n";

    // Count connected face neighbors
    numConnected = 0;
    for (int d = 0; d < 6; d++) {
        auto itf = module->getInterface((SCLattice::Direction)d);
        neighborPresent[d] = (itf && itf->connectedInterface);
        if (neighborPresent[d]) numConnected++;
    }

    // Color blocks Light Blue when they start sharing features (Phase 2)
    //module->setColor(Color(0.2f, 0.4f, 1.0f));
    //
    // if (module->blockId == 1) {
    //     currentPhase = 2;
    //     cout << ">>> PHASE 2: Feature Spreading (Face-Gather) Initiated..." << endl;
    // }

    FeatureMsgData fdata(conv1_out);
    for (int d = 0; d < 6; d++) {
        if (neighborPresent[d]) {
            auto itf = module->getInterface((SCLattice::Direction)d);
            uint64_t commTime = uint64_t(89.350 * sizeof(FeatureMsgData) + 1516.0);
            sendMessage(new MessageOf<FeatureMsgData>(FEATURE_MSG_ID, fdata),
                        itf, commTime, 0);
        }
    }

    // Phase 3: Tree construction
    // Block with blockId==1 initiates the spanning tree.
    // All other blocks wait for a TREE message.
    if (module->blockId == 1) {
        //currentPhase = 3;
        cout << ">>> PHASE 3: Spanning Tree (BFS) Construction Started" << endl;
        inTree = true;
        role = ROLE_ROOT;
        parentDirection = -1;

        TreeMsgData tdata(module->blockId);
        numExpectedChildren = 0;
        for (int d = 0; d < 6; d++) {
            if (neighborPresent[d]) {
                auto itf = module->getInterface((SCLattice::Direction)d);
                uint64_t commTime = uint64_t(89.350 * sizeof(TreeMsgData) + 1516.0);
                sendMessage(new MessageOf<TreeMsgData>(TREE_MSG_ID, tdata),
                            itf, commTime, 0);
                numExpectedChildren++;
            }
        }

        // Lone block edge case
        if (numConnected == 0) {
            computeConv2();
            classify();
        }
    }

    // If isolated block that isn't blockId 1 — handle specially
    if (numConnected == 0 && module->blockId != 1) {
        conv2Done = true;
        // This block won't participate in inference
    }
}

// PHASE 2: Feature Spreading — receive neighbor Conv1 features

void AI4PMCode_Embedded3D::handleFeatureMsg(
    std::shared_ptr<Message> _msg, P2PNetworkInterface *sender)
{
    currentPhase = 2;

    MessageOf<FeatureMsgData>* msg =
        static_cast<MessageOf<FeatureMsgData>*>(_msg.get());
    FeatureMsgData data = *msg->getData();
    embedded3dMsgCount++;

    int dir = getDirection(sender);
    if (dir < 0) return;

    // Store features in the correct FaceGather slot
    int slot = DIR_TO_SLOT[dir];
    for (int i = 0; i < EMB_K; i++)
        neighbor_features[slot][i] = data.features[i];

    featuresReceived++;

    // All face neighbors reported → compute Conv2
    if (featuresReceived >= numConnected) {
        console << "Module " << module->blockId << " received all neighbor features. Computing Phase 2\n";
        int mx = module->position[0];
        int my = module->position[1];
        int mz = module->position[2];
        if (mx == 0 && my == 0 && mz == 0) {
            cout << "Block (0,0,0) has received features from all neighbors. "
                 << "It now indirectly knows about X=2 via its neighbor at (1,0,0)" << endl;
        }
        computeConv2();
    }
}

// CONV2: Combine own features with face-neighbor features
// Dense(56 → 8, ReLU) — exactly matches training pipeline

void AI4PMCode_Embedded3D::computeConv2() {
    //currentPhase = 2;
    console << "Module " << module->blockId << " computing Conv2\n";
    // Build combined input: [own(8), slot1(8), ..., slot6(8)] = 56 values
    float combined[EMB_COMBINED];
    for (int s = 0; s < 7; s++)
        for (int i = 0; i < EMB_K; i++)
            combined[s * EMB_K + i] = neighbor_features[s][i];

    // Apply Dense(56 → 8) with ReLU
    for (int f = 0; f < EMB_K; f++) {
        float sum = conv2_face_bias[f];
        for (int i = 0; i < EMB_COMBINED; i++){
            sum += combined[i] * conv2_face_weight[i][f];
            module->setColor(ORANGE);
        }
        conv2_out[f] = (sum > 0.0f) ? sum : 0.0f;
    }

    conv2Done = true;

    checkReadyToAggregate();
}

// PHASE 3: Spanning Tree Construction
// Uses BFS from block 1. First TREE message to reach a block claims it.

void AI4PMCode_Embedded3D::handleTreeMsg(
    std::shared_ptr<Message> _msg, P2PNetworkInterface *sender)
{
    currentPhase = 3;

    MessageOf<TreeMsgData>* msg =
        static_cast<MessageOf<TreeMsgData>*>(_msg.get());
    TreeMsgData data = *msg->getData();
    embedded3dMsgCount++;

    int dir = getDirection(sender);
    if (dir < 0) return;

    if (!inTree) {
        // Join this tree
        inTree = true;
        parentDirection = dir;
        role = ROLE_INNER;
        console << "Module " << module->blockId << " joined the tree (Parent dir: " << dir << ")\n";

        // Color block Magenta when it joins the spanning tree (Phase 3)
        module->setColor(Color(1.0f, 0.0f, 1.0f)); 

        // Forward TREE to all other connected neighbors
        numExpectedChildren = 0;
        TreeMsgData tdata(data.rootId);
        for (int d = 0; d < 6; d++) {
            if (neighborPresent[d] && d != dir) {
                auto itf = module->getInterface((SCLattice::Direction)d);
                uint64_t commTime = uint64_t(89.350 * sizeof(TreeMsgData) + 1516.0);
                sendMessage(new MessageOf<TreeMsgData>(TREE_MSG_ID, tdata),
                            itf, commTime, 0);
                numExpectedChildren++;
            }
        }

        // If no other neighbors → this is a leaf
        if (numExpectedChildren == 0) {
            role = ROLE_LEAF;
            checkReadyToAggregate();
        }
    } else {
        // Already in tree → reject
        auto itf = module->getInterface((SCLattice::Direction)dir);
        if (itf) {
            TreeRejectMsgData rdata;
            uint64_t commTime = uint64_t(89.350 * sizeof(TreeRejectMsgData) + 1516.0);
            sendMessage(new MessageOf<TreeRejectMsgData>(TREE_REJECT_MSG_ID, rdata),
                        itf, commTime, 0);
        }
    }
}

void AI4PMCode_Embedded3D::handleTreeRejectMsg(
    std::shared_ptr<Message> _msg, P2PNetworkInterface *sender)
{
    embedded3dMsgCount++;
    rejectsReceived++;

    // Check if all children resolved
    if (childrenDone + rejectsReceived >= numExpectedChildren) {
        // If ALL potential children rejected, we're actually a leaf
        if (childrenDone == 0) {
            role = ROLE_LEAF;
        }
        checkReadyToAggregate();
    }
}

// PHASE 4: Tree-Based Aggregation (sum + count)

void AI4PMCode_Embedded3D::checkReadyToAggregate() {
    if (!conv2Done)
    {
        //currentPhase = 2;
        return;
    }
    if (!inTree)
    {
        //currentPhase = 3;
        return;
    }
    if (aggregationSent) return;

    if (role == ROLE_LEAF) {
        sendAggregateToParent();
    } else if (role == ROLE_INNER) {
        // Wait for all children to report
        if (childrenDone + rejectsReceived >= numExpectedChildren) {
            sendAggregateToParent();
        }
    } else if (role == ROLE_ROOT) {
        // Root waits for all children to report
        if (childrenDone + rejectsReceived >= numExpectedChildren) {
            currentPhase = 4;
            cout << ">>> PHASE 4: Global Feature Aggregation (Pooling) Completed." << endl;
            // Add own features
            for (int i = 0; i < EMB_K; i++)
                aggSum[i] += conv2_out[i];
            aggCount++;
            console << "Module " << module->blockId << " (Root) aggregation complete. Identifying shape\n";
            classify();
        }
    }
}

void AI4PMCode_Embedded3D::sendAggregateToParent() {
    currentPhase = 4;

    if (aggregationSent) return;
    aggregationSent = true;

    // Add own features to running aggregate
    for (int i = 0; i < EMB_K; i++)
        aggSum[i] += conv2_out[i];
    aggCount++;

    // Color block LightBlue when it sends its aggregate data (Phase 4)
    module->setColor(Color(LIGHTBLUE));
    console << "Module " << module->blockId << " sending aggregated features to parent\n";

    // Send to parent
    if (parentDirection >= 0) {
        auto itf = module->getInterface((SCLattice::Direction)parentDirection);
        if (itf) {
            AggregateMsgData adata(aggSum, aggCount);
            uint64_t commTime = uint64_t(89.350 * sizeof(AggregateMsgData) + 1516.0);
            sendMessage(new MessageOf<AggregateMsgData>(AGGREGATE_MSG_ID, adata),
                        itf, commTime, 0);
        }
    }
}

void AI4PMCode_Embedded3D::handleAggregateMsg(
    std::shared_ptr<Message> _msg, P2PNetworkInterface *sender)
{
    MessageOf<AggregateMsgData>* msg =
        static_cast<MessageOf<AggregateMsgData>*>(_msg.get());
    AggregateMsgData data = *msg->getData();
    embedded3dMsgCount++;

    // Track this direction as a child
    int dir = getDirection(sender);
    if (dir >= 0) isChild[dir] = true;

    // Accumulate
    for (int i = 0; i < EMB_K; i++)
        aggSum[i] += data.sum[i];
    aggCount += data.count;
    childrenDone++;

    // Check if all children are done
    if (childrenDone + rejectsReceived >= numExpectedChildren) {
        checkReadyToAggregate();
    }
}

// CLASSIFICATION (at root only)

void AI4PMCode_Embedded3D::classify() {
    // Compute average features from all blocks
    float avg[EMB_K];
    for (int i = 0; i < EMB_K; i++)
        avg[i] = aggSum[i] / (float)max(aggCount, 1);

    // FC layer: Dense(K → NUM_CLASSES)
    float logits[EMB_NUM_CLASSES];
    float maxLogit = -1e9f;
    for (int c = 0; c < EMB_NUM_CLASSES; c++) {
        float sum = fc_emb_bias[c];
        for (int i = 0; i < EMB_K; i++)
            sum += avg[i] * fc_emb_weight[i][c];
        logits[c] = sum;
        if (sum > maxLogit) maxLogit = sum;
    }

    // Softmax
    float sumExp = 0.0f;
    for (int c = 0; c < EMB_NUM_CLASSES; c++) {
        probabilities[c] = expf(logits[c] - maxLogit);
        sumExp += probabilities[c];
    }
    for (int c = 0; c < EMB_NUM_CLASSES; c++)
        probabilities[c] /= sumExp;

    // Find predicted class
    predictedClass = 0;
    for (int c = 1; c < EMB_NUM_CLASSES; c++)
        if (probabilities[c] > probabilities[predictedClass])
            predictedClass = c;

    // Console output
    const char* names[] = {"Cube", "Sphere", "Cylinder"};
    cout << endl;
    cout << "==========================================" << endl;
    cout << "  EMBEDDED 3D DISTRIBUTED CNN" << endl;
    cout << "==========================================" << endl;
    cout << "  Predicted: " << names[predictedClass]
         << " (" << fixed << setprecision(1)
         << probabilities[predictedClass] * 100 << "%)" << endl;
    cout << "  Blocks aggregated: " << aggCount << endl;
    cout << "  Total messages: " << embedded3dMsgCount << endl;
    cout << "------------------------------------------" << endl;
    for (int c = 0; c < EMB_NUM_CLASSES; c++) {
        cout << "  " << names[c] << ": " << fixed << setprecision(2)
             << probabilities[c] * 100 << "%" << endl;
    }
    cout << "===========================================" << endl;

    {
        ofstream rfile("cnn_result.txt", ios::trunc);
        if (rfile.is_open()) {
            rfile << "Predicted: " << names[predictedClass]
                  << " (" << fixed << setprecision(1)
                  << probabilities[predictedClass] * 100 << "%)" << endl;
            rfile << "Blocks aggregated: " << aggCount << endl;
            rfile << "Cube: " << fixed << setprecision(2) << probabilities[0] * 100 << "%" << endl;
            rfile << "Sphere: " << fixed << setprecision(2) << probabilities[1] * 100 << "%" << endl;
            rfile << "Cylinder: " << fixed << setprecision(2) << probabilities[2] * 100 << "%" << endl;
            rfile.flush(); 
        }
    }
    //exit(0);
    module->setColor(Color(1.0f, 0.0f, 0.0f));

    currentPhase = 5;
    cout << ">>> PHASE 5: Classification Verdict & Global Broadcast" << endl;
    broadcastResult();
}

// RESULT BROADCAST: Root sends result down the spanning tree

void AI4PMCode_Embedded3D::broadcastResult() {
    ResultMsgData rdata(predictedClass, probabilities);
    for (int d = 0; d < 6; d++) {
        if (isChild[d]) {
            auto itf = module->getInterface((SCLattice::Direction)d);
            if (itf && itf->connectedInterface) {
                uint64_t commTime = uint64_t(89.350 * sizeof(ResultMsgData) + 1516.0);
                sendMessage(new MessageOf<ResultMsgData>(RESULT_MSG_ID, rdata),
                            itf, commTime, 0);
            }
        }
    }
}

void AI4PMCode_Embedded3D::handleResultMsg(
    std::shared_ptr<Message> _msg, P2PNetworkInterface *sender)
{
    MessageOf<ResultMsgData>* msg =
        static_cast<MessageOf<ResultMsgData>*>(_msg.get());
    ResultMsgData data = *msg->getData();
    embedded3dMsgCount++;

    predictedClass = data.predictedClass;
    for (int c = 0; c < EMB_NUM_CLASSES; c++)
        probabilities[c] = data.probabilities[c];

    if (role == ROLE_ROOT) {
        module->setColor(Color(1.0f, 0.0f, 0.0f));       
    } else if (numConnected == 6) {
        module->setColor(Color(1.0f, 0.5f, 0.0f));       
    } else {
        if (probabilities[predictedClass] >= 0.80f) {
            switch (predictedClass) {
                case 0: module->setColor(Color(0.2f, 0.8f, 0.2f)); break; 
                case 1: module->setColor(Color(0.2f, 0.6f, 1.0f)); break; 
                case 2: module->setColor(Color(1.0f, 1.0f, 0.2f)); break; 
            }
        } else {
            module->setColor(Color(0.9f, 0.9f, 0.9f));
        }
    }

    // Forward result to children in tree
    for (int d = 0; d < 6; d++) {
        if (isChild[d]) {
            auto itf = module->getInterface((SCLattice::Direction)d);
            if (itf && itf->connectedInterface) {
                ResultMsgData fwd(data.predictedClass, data.probabilities);
                uint64_t commTime = uint64_t(89.350 * sizeof(ResultMsgData) + 1516.0);
                sendMessage(new MessageOf<ResultMsgData>(RESULT_MSG_ID, fwd),
                            itf, commTime, 0);
            }
        }
    }
}

void AI4PMCode_Embedded3D::parseUserBlockElements(TiXmlElement *config) {}

string AI4PMCode_Embedded3D::onInterfaceDraw() {
    stringstream ss;
    ss << fixed << setprecision(2);
    
    ss << "GLOBAL STATUS:\n";
    switch (currentPhase) {
        case 1: ss << "Phase 1: Local Conv1\n"; break;
        case 2: ss << "Phase 2: Spread Features\n"; break;
        case 3: ss << "Phase 3: Building Tree\n"; break;
        case 4: ss << "Phase 4: Aggregating\n"; break;
        case 5: ss << "Phase 5: Result Ready\n"; break;
        default: ss << "Initializing...\n"; break;
    }

    // Selected Block Info
    // Check if this module is the one currently clicked by the user
    GlBlock *sel = BlinkyBlocksWorld::getWorld()->getselectedGlBlock();
    if (sel && sel->blockId == module->blockId) {
        ss << "\nSELECTED BLOCK (ID " << module->blockId << "):\n";
        
        string status = "Status: ";
        if (!conv2Done) status = "Status: Conv1/Conv2...";
        else if (!aggregationSent) status = "Status: Waiting in Tree";
        else status = "Status: Data Reported";
        ss << status << "\n";

        string roleStr = "Role: ";
        if (role == ROLE_ROOT) roleStr = "Role: Root (Leader)";
        else if (role == ROLE_INNER) roleStr = "Role: Inner Node";
        else if (role == ROLE_LEAF) roleStr = "Role: Leaf Node";
        ss << roleStr << "\n";

        ss << "Children: " << childrenDone << "/" << (numExpectedChildren - rejectsReceived) << "\n";
    }

    if (currentPhase == 5) {
        const char* names[] = {"Cube", "Sphere", "Cylinder"};
        ss << "\nVERDICT: " << names[predictedClass] << "\n"
           << "Cube: " << probabilities[0] * 100 << "%\n"
           << "Sphere: " << probabilities[1] * 100 << "%\n"
           << "Cylinder: " << probabilities[2] * 100 << "%";
    }

    return ss.str();
}

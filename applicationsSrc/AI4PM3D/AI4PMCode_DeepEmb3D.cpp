#include "AI4PMCode_DeepEmb3D.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <fstream>

using namespace std;
static int deepEmb3dMsgCount = 0;
int AI4PMCode_DeepEmb3D::currentPhase = 0;
bool AI4PMCode_DeepEmb3D::hollowMode = false;
int AI4PMCode_DeepEmb3D::strategyMode = 3;
static int deepEmb3dBandwidthBytes = 0;

AI4PMCode_DeepEmb3D::AI4PMCode_DeepEmb3D(BlinkyBlocksBlock *host)
    : BlinkyBlocksBlockCode(host), module(host)
{
    if (!host) return;
    addMessageEventFunc2(INIT_MSG_ID,
        std::bind(&AI4PMCode_DeepEmb3D::handleInitMsg, this,
                  std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(FEATURE1_MSG_ID,
        std::bind(&AI4PMCode_DeepEmb3D::handleFeature1Msg, this,
                  std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(FEATURE2_MSG_ID,
        std::bind(&AI4PMCode_DeepEmb3D::handleFeature2Msg, this,
                  std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(TREE_MSG_ID,
        std::bind(&AI4PMCode_DeepEmb3D::handleTreeMsg, this,
                  std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(TREE_REJECT_MSG_ID,
        std::bind(&AI4PMCode_DeepEmb3D::handleTreeRejectMsg, this,
                  std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(AGGREGATE_MSG_ID,
        std::bind(&AI4PMCode_DeepEmb3D::handleAggregateMsg, this,
                  std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(RESULT_MSG_ID,
        std::bind(&AI4PMCode_DeepEmb3D::handleResultMsg, this,
                  std::placeholders::_1, std::placeholders::_2));

    // Initialize all state
    memset(occupancy, 0, sizeof(occupancy));
    memset(conv1_out, 0, sizeof(conv1_out));
    numConnected = 0;
    memset(neighborPresent, 0, sizeof(neighborPresent));

    isInterior = false;
    activeNeighbors = 0;
    initMsgsReceived = 0;
    memset(neighborIsInterior, 0, sizeof(neighborIsInterior));

    features1Received = 0;
    memset(neighbor_features1, 0, sizeof(neighbor_features1));
    memset(conv2_out, 0, sizeof(conv2_out));
    conv2Done = false;

    features2Received = 0;
    memset(neighbor_features2, 0, sizeof(neighbor_features2));
    memset(conv3_out, 0, sizeof(conv3_out));
    conv3Done = false;

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

int AI4PMCode_DeepEmb3D::getDirection(P2PNetworkInterface* iface) {
    for (int d = 0; d < 6; d++) {
        if (module->getInterface((SCLattice::Direction)d) == iface) {
            return d;
        }
    }
    return -1;
}

// ── PHASE 1: Local 3D Feature Extraction (0 messages) ──

void AI4PMCode_DeepEmb3D::computeConv1() {
    currentPhase = 1;
    auto* lattice = BaseSimulator::getWorld()->lattice;
    int mx = module->position[0];
    int my = module->position[1];
    int mz = module->position[2];

    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dz = -1; dz <= 1; dz++) {
                Cell3DPosition pos(mx + dx, my + dy, mz + dz);
                bool hasBlock = lattice->cellHasBlock(pos);
                occupancy[dx+1][dy+1][dz+1] = hasBlock ? 1.0f : 0.0f;
            }
        }
    }

    // Apply Conv1
    for (int f = 0; f < DEEP_K; f++) {
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

    // Store own Conv1 features in slot 0 for Spread 1
    for (int i = 0; i < DEEP_K; i++)
        neighbor_features1[0][i] = conv1_out[i];

    std::cout << "BLOCK_FEATURES_CONV1: " << module->blockId << " "
              << mx << " " << my << " " << mz << " ";
    for (int i = 0; i < DEEP_K; i++) {
        std::cout << conv1_out[i] << (i == DEEP_K - 1 ? "" : ",");
    }
    std::cout << std::endl;
}

void AI4PMCode_DeepEmb3D::startup() {
    // Check if hollowMode is enabled via config file
    strategyMode = 1;
    hollowMode = false;
    std::ifstream cfgFile("hollow_config.txt");
    if (cfgFile.good()) {
        std::string line;
        if (std::getline(cfgFile, line)) {
            if (line.find("strategy=2") != std::string::npos) {
                strategyMode = 2;
                hollowMode = false;
            } else if (line.find("strategy=3") != std::string::npos || line.find("hollow=true") != std::string::npos) {
                strategyMode = 3;
                hollowMode = true;
            }
        }
    }

    numConnected = 0;
    for (int d = 0; d < 6; d++) {
        auto itf = module->getInterface((SCLattice::Direction)d);
        neighborPresent[d] = (itf && itf->connectedInterface);
        if (neighborPresent[d]) numConnected++;
    }

    if (module->blockId == 1) {
        std::cout << "\n==================================================" << std::endl;
        std::cout << "  TINYML HARDWARE COMPATIBILITY DIAGNOSTIC        " << std::endl;
        std::cout << "==================================================" << std::endl;
        std::cout << "  RAM footprint per BlinkyBlock: " << sizeof(AI4PMCode_DeepEmb3D) << " bytes" << std::endl;
        std::cout << "  Flash weights data size      : " << (9815 * sizeof(float)) << " bytes (38.3 KB)" << std::endl;
        std::cout << "  Mode Selected                : ";
        if (strategyMode == 1) std::cout << "SOLID (Standard)" << std::endl;
        else if (strategyMode == 2) std::cout << "INDEXING (Strategy 2)" << std::endl;
        else if (strategyMode == 3) std::cout << "HOLLOW (Strategy 3)" << std::endl;
        std::cout << "==================================================\n" << std::endl;
    }

    if (hollowMode) {
        isInterior = (numConnected == 6 && module->blockId != 1);
        computeDefaultInteriorFeatures();
    } else {
        isInterior = false;
    }

    if (isInterior) {
        module->setColor(Color(0.15f, 0.15f, 0.15f));
    } else {
        module->setColor(Color(1.0f, 1.0f, 1.0f));
    }

    if (hollowMode) {
        // Handshake: Send INIT_MSG to all neighbors
        InitMsgData idata(isInterior);
        for (int d = 0; d < 6; d++) {
            if (neighborPresent[d]) {
                auto itf = module->getInterface((SCLattice::Direction)d);
                uint64_t commTime = uint64_t(89.350 * sizeof(InitMsgData) + 1516.0);
                sendDeepMessage(new MessageOf<InitMsgData>(INIT_MSG_ID, idata),
                            itf, commTime, 0);
            }
        }
        if (numConnected == 0) {
            startCNNPhases();
        }
    } else {
        activeNeighbors = numConnected;
        startCNNPhases();
    }
}

void AI4PMCode_DeepEmb3D::handleInitMsg(
    std::shared_ptr<Message> _msg, P2PNetworkInterface *sender)
{
    MessageOf<InitMsgData>* msg = static_cast<MessageOf<InitMsgData>*>(_msg.get());
    InitMsgData data = *msg->getData();
    deepEmb3dMsgCount++;

    int dir = getDirection(sender);
    if (dir < 0) return;

    neighborIsInterior[dir] = data.isInterior;
    initMsgsReceived++;

    if (data.isInterior) {
        int slot = DIR_TO_SLOT[dir];
        // Pre-populate slots with pre-computed default interior activations
        for (int i = 0; i < DEEP_K; i++) {
            neighbor_features1[slot][i] = default_interior_conv1[i];
            neighbor_features2[slot][i] = default_interior_conv2[i];
        }
    }

    if (initMsgsReceived >= numConnected) {
        // Calculate active neighbors count
        activeNeighbors = 0;
        for (int d = 0; d < 6; d++) {
            if (neighborPresent[d] && !neighborIsInterior[d]) {
                activeNeighbors++;
            }
        }

        // Pre-count features received from interior neighbors
        features1Received = 0;
        features2Received = 0;
        for (int d = 0; d < 6; d++) {
            if (neighborPresent[d] && neighborIsInterior[d]) {
                features1Received++;
                features2Received++;
            }
        }

        startCNNPhases();
    }
}

void AI4PMCode_DeepEmb3D::startCNNPhases() {
    if (isInterior) return;

    computeConv1();

    // Spread 1: Send Conv1 features to active neighbors
    Feature1MsgData fdata1(conv1_out);
    for (int d = 0; d < 6; d++) {
        if (neighborPresent[d]) {
            if (!hollowMode || !neighborIsInterior[d]) {
                auto itf = module->getInterface((SCLattice::Direction)d);
                uint64_t commTime = uint64_t(89.350 * sizeof(Feature1MsgData) + 1516.0);
                sendDeepMessage(new MessageOf<Feature1MsgData>(FEATURE1_MSG_ID, fdata1),
                            itf, commTime, 0);
            }
        }
    }

    // Spanning tree construction initiated by root (block 1)
    if (module->blockId == 1) {
        cout << ">>> PHASE 4: Spanning Tree (BFS) Construction Started" << endl;
        inTree = true;
        role = ROLE_ROOT;
        parentDirection = -1;

        TreeMsgData tdata(module->blockId);
        numExpectedChildren = 0;
        for (int d = 0; d < 6; d++) {
            // Root only connects to active neighbors in the spanning tree!
            if (neighborPresent[d] && (!hollowMode || !neighborIsInterior[d])) {
                auto itf = module->getInterface((SCLattice::Direction)d);
                uint64_t commTime = uint64_t(89.350 * sizeof(TreeMsgData) + 1516.0);
                sendDeepMessage(new MessageOf<TreeMsgData>(TREE_MSG_ID, tdata),
                            itf, commTime, 0);
                numExpectedChildren++;
            }
        }

        if (numExpectedChildren == 0) {
            computeConv2();
            computeConv3();
            classify();
        }
    }

    if (numConnected == 0 && module->blockId != 1) {
        conv2Done = true;
        conv3Done = true;
    } else if (features1Received >= numConnected) {
        computeConv2();
    }
}

// ── PHASE 2: Spread 1 (Conv2) ──

void AI4PMCode_DeepEmb3D::handleFeature1Msg(
    std::shared_ptr<Message> _msg, P2PNetworkInterface *sender)
{
    currentPhase = 2;
    MessageOf<Feature1MsgData>* msg = static_cast<MessageOf<Feature1MsgData>*>(_msg.get());
    Feature1MsgData data = *msg->getData();
    deepEmb3dMsgCount++;

    int dir = getDirection(sender);
    if (dir < 0) return;

    int slot = DIR_TO_SLOT[dir];
    for (int i = 0; i < DEEP_K; i++)
        neighbor_features1[slot][i] = data.features[i];

    features1Received++;

    if (features1Received >= numConnected) {
        computeConv2();
    }
}

void AI4PMCode_DeepEmb3D::computeConv2() {
    float combined[DEEP_COMBINED];
    for (int s = 0; s < 7; s++)
        for (int i = 0; i < DEEP_K; i++)
            combined[s * DEEP_K + i] = neighbor_features1[s][i];

    // Apply Dense(56 → 8) for Conv2
    for (int f = 0; f < DEEP_K; f++) {
        float sum = conv2_face_bias[f];
        for (int i = 0; i < DEEP_COMBINED; i++){
            sum += combined[i] * conv2_face_weight[i][f];
            module->setColor(ORANGE);
        }
        // ReLU + residual (training: c2_res = c2 + c1); conv2_out holds c2_res
        conv2_out[f] = ((sum > 0.0f) ? sum : 0.0f) + conv1_out[f];
    }

    conv2Done = true;

    int mx = module->position[0];
    int my = module->position[1];
    int mz = module->position[2];
    std::cout << "BLOCK_FEATURES_CONV2: " << module->blockId << " "
              << mx << " " << my << " " << mz << " ";
    for (int i = 0; i < DEEP_K; i++) {
        std::cout << conv2_out[i] << (i == DEEP_K - 1 ? "" : ",");
    }
    std::cout << std::endl;
    
    // Store own Conv2 features in slot 0 for Spread 2
    for (int i = 0; i < DEEP_K; i++)
        neighbor_features2[0][i] = conv2_out[i];

    // Spread 2: Send Conv2 to neighbors
    Feature2MsgData fdata2(conv2_out);
    for (int d = 0; d < 6; d++) {
        if (neighborPresent[d]) {
            auto itf = module->getInterface((SCLattice::Direction)d);
            uint64_t commTime = uint64_t(89.350 * sizeof(Feature2MsgData) + 1516.0);
            sendDeepMessage(new MessageOf<Feature2MsgData>(FEATURE2_MSG_ID, fdata2),
                        itf, commTime, 0);
        }
    }
    
    if (features2Received >= numConnected) {
        computeConv3();
    }
}

// ── PHASE 3: Spread 2 (Conv3) ──

void AI4PMCode_DeepEmb3D::handleFeature2Msg(
    std::shared_ptr<Message> _msg, P2PNetworkInterface *sender)
{
    currentPhase = 3;
    MessageOf<Feature2MsgData>* msg = static_cast<MessageOf<Feature2MsgData>*>(_msg.get());
    Feature2MsgData data = *msg->getData();
    deepEmb3dMsgCount++;

    int dir = getDirection(sender);
    if (dir < 0) return;

    int slot = DIR_TO_SLOT[dir];
    for (int i = 0; i < DEEP_K; i++)
        neighbor_features2[slot][i] = data.features[i];

    features2Received++;

    if (features2Received >= numConnected && conv2Done) {
        computeConv3();
    }
}

void AI4PMCode_DeepEmb3D::computeConv3() {
    float combined[DEEP_COMBINED];
    for (int s = 0; s < 7; s++)
        for (int i = 0; i < DEEP_K; i++)
            combined[s * DEEP_K + i] = neighbor_features2[s][i];

    // Apply Dense(56 → 8) for Conv3
    for (int f = 0; f < DEEP_K; f++) {
        float sum = conv3_face_bias[f];
        for (int i = 0; i < DEEP_COMBINED; i++){
            sum += combined[i] * conv3_face_weight[i][f];
            module->setColor(RED);
        }
        // ReLU + residual (training: c3_res = c3 + c2_res); conv3_out holds c3_res, which is pooled
        conv3_out[f] = ((sum > 0.0f) ? sum : 0.0f) + conv2_out[f];
    }

    conv3Done = true;

    int mx = module->position[0];
    int my = module->position[1];
    int mz = module->position[2];
    std::cout << "BLOCK_FEATURES_CONV3: " << module->blockId << " "
              << mx << " " << my << " " << mz << " ";
    for (int i = 0; i < DEEP_K; i++) {
        std::cout << conv3_out[i] << (i == DEEP_K - 1 ? "" : ",");
    }
    std::cout << std::endl;

    checkReadyToAggregate();
}

// ── PHASE 4: Spanning Tree Construction ──

void AI4PMCode_DeepEmb3D::handleTreeMsg(
    std::shared_ptr<Message> _msg, P2PNetworkInterface *sender)
{
    currentPhase = 4;
    MessageOf<TreeMsgData>* msg = static_cast<MessageOf<TreeMsgData>*>(_msg.get());
    TreeMsgData data = *msg->getData();
    deepEmb3dMsgCount++;

    int dir = getDirection(sender);
    if (dir < 0) return;

    if (isInterior) {
        auto itf = module->getInterface((SCLattice::Direction)dir);
        if (itf) {
            TreeRejectMsgData rdata;
            uint64_t commTime = uint64_t(89.350 * sizeof(TreeRejectMsgData) + 1516.0);
            sendDeepMessage(new MessageOf<TreeRejectMsgData>(TREE_REJECT_MSG_ID, rdata),
                        itf, commTime, 0);
        }
        return;
    }

    if (!inTree) {
        inTree = true;
        parentDirection = dir;
        role = ROLE_INNER;
        module->setColor(Color(1.0f, 0.0f, 1.0f)); 

        numExpectedChildren = 0;
        TreeMsgData tdata(data.rootId);
        for (int d = 0; d < 6; d++) {
            if (neighborPresent[d] && d != dir && (!hollowMode || !neighborIsInterior[d])) {
                auto itf = module->getInterface((SCLattice::Direction)d);
                uint64_t commTime = uint64_t(89.350 * sizeof(TreeMsgData) + 1516.0);
                sendDeepMessage(new MessageOf<TreeMsgData>(TREE_MSG_ID, tdata),
                            itf, commTime, 0);
                numExpectedChildren++;
            }
        }

        if (numExpectedChildren == 0) {
            role = ROLE_LEAF;
            checkReadyToAggregate();
        }
    } else {
        auto itf = module->getInterface((SCLattice::Direction)dir);
        if (itf) {
            TreeRejectMsgData rdata;
            uint64_t commTime = uint64_t(89.350 * sizeof(TreeRejectMsgData) + 1516.0);
            sendDeepMessage(new MessageOf<TreeRejectMsgData>(TREE_REJECT_MSG_ID, rdata),
                        itf, commTime, 0);
        }
    }
}

void AI4PMCode_DeepEmb3D::handleTreeRejectMsg(
    std::shared_ptr<Message> _msg, P2PNetworkInterface *sender)
{
    deepEmb3dMsgCount++;
    rejectsReceived++;

    if (childrenDone + rejectsReceived >= numExpectedChildren) {
        if (childrenDone == 0) {
            role = ROLE_LEAF;
        }
        checkReadyToAggregate();
    }
}

// ── PHASE 5: Aggregation ──

void AI4PMCode_DeepEmb3D::checkReadyToAggregate() {
    if (!conv3Done) return;
    if (!inTree) return;
    if (aggregationSent) return;

    if (role == ROLE_LEAF) {
        sendAggregateToParent();
    } else if (role == ROLE_INNER) {
        if (childrenDone + rejectsReceived >= numExpectedChildren) {
            sendAggregateToParent();
        }
    } else if (role == ROLE_ROOT) {
        if (childrenDone + rejectsReceived >= numExpectedChildren) {
            currentPhase = 5;
            for (int i = 0; i < DEEP_K; i++)
                aggSum[i] += conv3_out[i];
            aggCount++;
            classify();
        }
    }
}

void AI4PMCode_DeepEmb3D::sendAggregateToParent() {
    currentPhase = 5;
    if (aggregationSent) return;
    aggregationSent = true;

    for (int i = 0; i < DEEP_K; i++)
        aggSum[i] += conv3_out[i];
    aggCount++;

    module->setColor(Color(LIGHTBLUE));

    if (parentDirection >= 0) {
        auto itf = module->getInterface((SCLattice::Direction)parentDirection);
        if (itf) {
            AggregateMsgData adata(aggSum, aggCount);
            uint64_t commTime = uint64_t(89.350 * sizeof(AggregateMsgData) + 1516.0);
            sendDeepMessage(new MessageOf<AggregateMsgData>(AGGREGATE_MSG_ID, adata),
                        itf, commTime, 0);
        }
    }
}

void AI4PMCode_DeepEmb3D::handleAggregateMsg(
    std::shared_ptr<Message> _msg, P2PNetworkInterface *sender)
{
    MessageOf<AggregateMsgData>* msg = static_cast<MessageOf<AggregateMsgData>*>(_msg.get());
    AggregateMsgData data = *msg->getData();
    deepEmb3dMsgCount++;

    int dir = getDirection(sender);
    if (dir >= 0) isChild[dir] = true;

    for (int i = 0; i < DEEP_K; i++)
        aggSum[i] += data.sum[i];
    aggCount += data.count;
    childrenDone++;

    if (childrenDone + rejectsReceived >= numExpectedChildren) {
        checkReadyToAggregate();
    }
}

// ── PHASE 6: Classification ──
void AI4PMCode_DeepEmb3D::classify() {
    float avg[DEEP_K];
    if (hollowMode) {
        int totalBlocks = BlinkyBlocksWorld::getWorld()->getNbBlocks();
        int interiorCount = totalBlocks - aggCount;
        std::cout << "DEBUG_RECONSTRUCT: totalBlocks=" << totalBlocks 
                  << ", aggCount=" << aggCount 
                  << ", interiorCount=" << interiorCount << std::endl;
        std::cout << "DEBUG_RECONSTRUCT: default_interior_conv3=";
        for (int i = 0; i < DEEP_K; i++) {
            std::cout << default_interior_conv3[i] << (i == DEEP_K - 1 ? "" : ",");
        }
        std::cout << std::endl;
        std::cout << "DEBUG_RECONSTRUCT: aggSum=";
        for (int i = 0; i < DEEP_K; i++) {
            std::cout << aggSum[i] << (i == DEEP_K - 1 ? "" : ",");
        }
        std::cout << std::endl;
        for (int i = 0; i < DEEP_K; i++) {
            float reconstructedSum = aggSum[i] + (float)interiorCount * default_interior_conv3[i];
            avg[i] = reconstructedSum / (float)max(totalBlocks, 1);
        }
        std::cout << "DEBUG_RECONSTRUCT: reconstructed_avg=";
        for (int i = 0; i < DEEP_K; i++) {
            std::cout << avg[i] << (i == DEEP_K - 1 ? "" : ",");
        }
        std::cout << std::endl;
    } else {
        for (int i = 0; i < DEEP_K; i++)
            avg[i] = aggSum[i] / (float)max(aggCount, 1);
        std::cout << "DEBUG_SOLID: avg=";
        for (int i = 0; i < DEEP_K; i++) {
            std::cout << avg[i] << (i == DEEP_K - 1 ? "" : ",");
        }
        std::cout << std::endl;
    }

    float fc1_out[DEEP_K_FC1];
    for (int c = 0; c < DEEP_K_FC1; c++) {
        float sum = fc1_deep_bias[c];
        for (int i = 0; i < DEEP_K; i++)
            sum += avg[i] * fc1_deep_weight[i][c];
        fc1_out[c] = (sum > 0.0f) ? sum : 0.0f; // ReLU
    }

    float logits[DEEP_NUM_CLASSES];
    float maxLogit = -1e9f;
    for (int c = 0; c < DEEP_NUM_CLASSES; c++) {
        float sum = fc2_deep_bias[c];
        for (int i = 0; i < DEEP_K_FC1; i++)
            sum += fc1_out[i] * fc2_deep_weight[i][c];
        logits[c] = sum;
        if (sum > maxLogit) maxLogit = sum;
    }

    float sumExp = 0.0f;
    for (int c = 0; c < DEEP_NUM_CLASSES; c++) {
        probabilities[c] = expf(logits[c] - maxLogit);
        sumExp += probabilities[c];
    }
    for (int c = 0; c < DEEP_NUM_CLASSES; c++)
        probabilities[c] /= sumExp;

    predictedClass = 0;
    for (int c = 1; c < DEEP_NUM_CLASSES; c++)
        if (probabilities[c] > probabilities[predictedClass])
            predictedClass = c;

    const char* names[] = {"Cube", "Sphere", "Cylinder", "Chair", "Bridge", "Humanoid"};
    cout << endl;
    cout << "==========================================" << endl;
    cout << "  DEEP EMBEDDED 3D DISTRIBUTED CNN" << endl;
    cout << "==========================================" << endl;
    cout << "  Predicted: " << names[predictedClass]
         << " (" << fixed << setprecision(1)
         << probabilities[predictedClass] * 100 << "%)" << endl;
    cout << "  Blocks aggregated: " << aggCount << endl;
    cout << "  Total messages: " << deepEmb3dMsgCount << endl;
    cout << "------------------------------------------" << endl;
    for (int c = 0; c < DEEP_NUM_CLASSES; c++) {
        cout << "  " << names[c] << ": " << fixed << setprecision(2)
             << probabilities[c] * 100 << "%" << endl;
    }
    cout << "  Total bandwidth: " << deepEmb3dBandwidthBytes << " bytes" << endl;
    cout << "===========================================" << endl;

    {
        ofstream rfile("cnn_result.txt", ios::trunc);
        if (rfile.is_open()) {
            rfile << "Predicted: " << names[predictedClass]
                   << " (" << fixed << setprecision(1)
                   << probabilities[predictedClass] * 100 << "%)" << endl;
            rfile << "Blocks aggregated: " << aggCount << endl;
            rfile << "Total messages: " << deepEmb3dMsgCount << endl;
            rfile << "Total bandwidth: " << deepEmb3dBandwidthBytes << " bytes" << endl;
            for (int c = 0; c < DEEP_NUM_CLASSES; c++) {
                rfile << names[c] << ": " << fixed << setprecision(2) << probabilities[c] * 100 << "%" << endl;
            }
            rfile.flush(); 
        }
    }
    
    module->setColor(Color(1.0f, 0.0f, 0.0f));
    currentPhase = 6;
    broadcastResult();
}

void AI4PMCode_DeepEmb3D::broadcastResult() {
    ResultMsgData rdata(predictedClass, probabilities);
    for (int d = 0; d < 6; d++) {
        if (isChild[d]) {
            auto itf = module->getInterface((SCLattice::Direction)d);
            if (itf && itf->connectedInterface) {
                uint64_t commTime = uint64_t(89.350 * sizeof(ResultMsgData) + 1516.0);
                sendDeepMessage(new MessageOf<ResultMsgData>(RESULT_MSG_ID, rdata),
                            itf, commTime, 0);
            }
        }
    }
}

void AI4PMCode_DeepEmb3D::handleResultMsg(
    std::shared_ptr<Message> _msg, P2PNetworkInterface *sender)
{
    MessageOf<ResultMsgData>* msg = static_cast<MessageOf<ResultMsgData>*>(_msg.get());
    ResultMsgData data = *msg->getData();
    deepEmb3dMsgCount++;

    predictedClass = data.predictedClass;
    for (int c = 0; c < DEEP_NUM_CLASSES; c++)
        probabilities[c] = data.probabilities[c];

    if (role == ROLE_ROOT) {
        module->setColor(Color(1.0f, 0.0f, 0.0f));       
    } else if (numConnected == 6) {
        module->setColor(Color(1.0f, 0.5f, 0.0f));       
    } else {
        if (probabilities[predictedClass] >= 0.80f) {
            switch (predictedClass) {
                case 0: module->setColor(Color(0.2f, 0.8f, 0.2f)); break;  // Cube = green
                case 1: module->setColor(Color(0.2f, 0.6f, 1.0f)); break;  // Sphere = blue
                case 2: module->setColor(Color(1.0f, 1.0f, 0.2f)); break;  // Cylinder = yellow
                case 3: module->setColor(Color(0.8f, 0.2f, 0.8f)); break;  // Chair = purple
                case 4: module->setColor(Color(0.5f, 0.5f, 0.5f)); break;  // Bridge = gray
                case 5: module->setColor(Color(1.0f, 0.0f, 0.0f)); break;  // Humanoid = red
                default: break;
            }
        } else {
            module->setColor(Color(0.9f, 0.9f, 0.9f));
        }
    }

    for (int d = 0; d < 6; d++) {
        if (isChild[d]) {
            auto itf = module->getInterface((SCLattice::Direction)d);
            if (itf && itf->connectedInterface) {
                ResultMsgData fwd(data.predictedClass, data.probabilities);
                uint64_t commTime = uint64_t(89.350 * sizeof(ResultMsgData) + 1516.0);
                sendDeepMessage(new MessageOf<ResultMsgData>(RESULT_MSG_ID, fwd),
                            itf, commTime, 0);
            }
        }
    }
}

void AI4PMCode_DeepEmb3D::parseUserBlockElements(TiXmlElement *config) {
    const char *attr = config->Attribute("hollow");
    if (attr && strcmp(attr, "true") == 0) {
        hollowMode = true;
    }
}

void AI4PMCode_DeepEmb3D::computeDefaultInteriorFeatures() {
    // Conv1 for solid occupancy (all 1.0f)
    for (int f = 0; f < DEEP_K; f++) {
        float sum = conv1_3d_bias[f];
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                for (int k = 0; k < 3; k++)
                    sum += 1.0f * conv1_3d_kernel[f][0][i][j][k];
        default_interior_conv1[f] = (sum > 0.0f) ? sum : 0.0f;
    }
    
    // Conv2 for combined default_interior_conv1 (7 slots filled with default_interior_conv1)
    float combined2[DEEP_COMBINED];
    for (int s = 0; s < 7; s++)
        for (int i = 0; i < DEEP_K; i++)
            combined2[s * DEEP_K + i] = default_interior_conv1[i];
            
    for (int f = 0; f < DEEP_K; f++) {
        float sum = conv2_face_bias[f];
        for (int i = 0; i < DEEP_COMBINED; i++)
            sum += combined2[i] * conv2_face_weight[i][f];
        default_interior_conv2[f] = ((sum > 0.0f) ? sum : 0.0f) + default_interior_conv1[f];  // + residual
    }

    // Conv3 for combined default_interior_conv2
    float combined3[DEEP_COMBINED];
    for (int s = 0; s < 7; s++)
        for (int i = 0; i < DEEP_K; i++)
            combined3[s * DEEP_K + i] = default_interior_conv2[i];
            
    for (int f = 0; f < DEEP_K; f++) {
        float sum = conv3_face_bias[f];
        for (int i = 0; i < DEEP_COMBINED; i++)
            sum += combined3[i] * conv3_face_weight[i][f];
        default_interior_conv3[f] = ((sum > 0.0f) ? sum : 0.0f) + default_interior_conv2[f];  // + residual
    }
}

string AI4PMCode_DeepEmb3D::onInterfaceDraw() {
    stringstream ss;
    ss << fixed << setprecision(2);
    
    ss << "GLOBAL STATUS:\n";
    switch (currentPhase) {
        case 1: ss << "Phase 1: Local Conv1\n"; break;
        case 2: ss << "Phase 2: Spread Conv1\n"; break;
        case 3: ss << "Phase 3: Spread Conv2\n"; break;
        case 4: ss << "Phase 4: Building Tree\n"; break;
        case 5: ss << "Phase 5: Aggregating\n"; break;
        case 6: ss << "Phase 6: Result Ready\n"; break;
        default: ss << "Initializing...\n"; break;
    }

    GlBlock *sel = BlinkyBlocksWorld::getWorld()->getselectedGlBlock();
    if (sel && sel->blockId == module->blockId) {
        ss << "\nSELECTED BLOCK (ID " << module->blockId << "):\n";
        
        string status = "Status: ";
        if (!conv3Done) status = "Status: Computing Convs...";
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

    if (currentPhase == 6) {
        const char* names[] = {"Cube", "Sphere", "Cylinder", "Chair", "Bridge", "Humanoid"};
        ss << "\nVERDICT: " << names[predictedClass] << "\n";
        for (int c = 0; c < DEEP_NUM_CLASSES; c++) {
            ss << names[c] << ": " << probabilities[c] * 100 << "%\n";
        }
    }

    return ss.str();
}

void AI4PMCode_DeepEmb3D::sendDeepMessage(Message* msg, P2PNetworkInterface* itf, uint64_t commTime, uint64_t delay) {
    int payloadSize = 0;
    switch (msg->type) {
        case INIT_MSG_ID:
            payloadSize = 1;
            break;
        case FEATURE1_MSG_ID:
            payloadSize = (strategyMode == 2) ? 1 : 96;
            break;
        case FEATURE2_MSG_ID:
            payloadSize = (strategyMode == 2) ? 2 : 96;
            break;
        case TREE_MSG_ID:
            payloadSize = 4;
            break;
        case TREE_REJECT_MSG_ID:
            payloadSize = 1;
            break;
        case AGGREGATE_MSG_ID:
            payloadSize = 100;
            break;
        case RESULT_MSG_ID:
            payloadSize = 32;
            break;
        default:
            payloadSize = 0;
            break;
    }
    deepEmb3dBandwidthBytes += payloadSize;
    sendMessage(msg, itf, commTime, delay);
}

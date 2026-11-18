#ifndef AI4PMCode_Embedded3D_H_
#define AI4PMCode_Embedded3D_H_

/**
 * @file AI4PMCode_Embedded3D.hpp
 * Embedded 3D Distributed CNN — The Shape Recognizes Itself
 *
 * NO base layer needed. Every block in the shape participates in inference.
 * Roles are assigned dynamically (no hardcoded positions).
 *
 * Phases:
 *   1. Each block queries its 3×3×3 neighborhood → Conv1 features [0 messages]
 *   2. Each block sends Conv1 features to 6 face neighbors → Conv2 [6N messages]
 *   3. Spanning tree aggregation → root computes global average [~N messages]
 *   4. Root applies FC layer → classification → broadcasts result [~N messages]
 *
 * Classes: 0=Cube, 1=Sphere, 2=Cylinder
 */

#include "robots/blinkyBlocks/blinkyBlocksSimulator.h"
#include "robots/blinkyBlocks/blinkyBlocksWorld.h"
#include "robots/blinkyBlocks/blinkyBlocksBlockCode.h"
#include "neuralNetworkData_embedded3d.h"
#include <vector>
#include <cmath>
#include <cstring>

using namespace BlinkyBlocks;

// ── Message Type IDs ──
static const int FEATURE_MSG_ID      = 3001;
static const int TREE_MSG_ID         = 3002;
static const int TREE_REJECT_MSG_ID  = 3003;
static const int AGGREGATE_MSG_ID    = 3004;
static const int RESULT_MSG_ID       = 3005;

// ── Message Data Structures ──

struct FeatureMsgData {
    float features[EMB_K];
    FeatureMsgData() { memset(features, 0, sizeof(features)); }
    FeatureMsgData(const float* f) {
        for (int i = 0; i < EMB_K; i++) features[i] = f[i];
    }
};

struct TreeMsgData {
    int rootId;
    TreeMsgData() : rootId(-1) {}
    TreeMsgData(int r) : rootId(r) {}
};

struct TreeRejectMsgData {
    TreeRejectMsgData() {}
};

struct AggregateMsgData {
    float sum[EMB_K];
    int count;
    AggregateMsgData() : count(0) { memset(sum, 0, sizeof(sum)); }
    AggregateMsgData(const float* s, int c) : count(c) {
        for (int i = 0; i < EMB_K; i++) sum[i] = s[i];
    }
};

struct ResultMsgData {
    int predictedClass;
    float probabilities[EMB_NUM_CLASSES];
    ResultMsgData() : predictedClass(-1) { memset(probabilities, 0, sizeof(probabilities)); }
    ResultMsgData(int cls, const float* probs) : predictedClass(cls) {
        for (int i = 0; i < EMB_NUM_CLASSES; i++) probabilities[i] = probs[i];
    }
};

/**
 * Direction-to-FaceGather-slot mapping.
 * Must match the training pipeline's concat order:
 *   [own=0, x_minus=1, x_plus=2, y_minus=3, y_plus=4, z_minus=5, z_plus=6]
 *
 * SCLattice directions (from existing code analysis):
 *   North=0 → -x → slot 1 (x_minus)
 *   South=1 → +x → slot 2 (x_plus)
 *   East=2  → +y → slot 4 (y_plus)
 *   West=3  → -y → slot 3 (y_minus)
 *   Top=4   → +z → slot 6 (z_plus)
 *   Bottom=5→ -z → slot 5 (z_minus)
 */
static const int DIR_TO_SLOT[6] = {1, 2, 4, 3, 6, 5};

enum Embedded3DRole {
    ROLE_UNDETERMINED,
    ROLE_LEAF,
    ROLE_INNER,
    ROLE_ROOT
};

class AI4PMCode_Embedded3D : public BlinkyBlocksBlockCode {
private:
    BlinkyBlocksBlock *module = nullptr;
    static int currentPhase; 

    // ── Phase 1: Local Features ──
    float occupancy[3][3][3];
    float conv1_out[EMB_K];

    // ── Phase 2: Feature Spreading ──
    int numConnected;
    int featuresReceived;
    float neighbor_features[7][EMB_K];  // [own, slot1..slot6]
    bool neighborPresent[6];
    float conv2_out[EMB_K];
    bool conv2Done;

    // ── Phase 3: Spanning Tree ──
    Embedded3DRole role;
    bool inTree;
    int parentDirection;
    int numExpectedChildren;
    int childrenDone;
    int rejectsReceived;
    bool isChild[6];     // Track which directions are actual tree children

    // ── Phase 4: Aggregation ──
    float aggSum[EMB_K];
    int aggCount;
    bool aggregationSent;

    // ── Result ──
    int predictedClass;
    float probabilities[EMB_NUM_CLASSES];

    // ── Helpers ──
    int getDirection(P2PNetworkInterface* iface);
    void computeConv1();
    void computeConv2();
    void checkReadyToAggregate();
    void sendAggregateToParent();
    void classify();
    void broadcastResult();

public:
    AI4PMCode_Embedded3D(BlinkyBlocksBlock *host);
    ~AI4PMCode_Embedded3D() {};

    void startup() override;
    void parseUserBlockElements(TiXmlElement *config) override;

    // Message handlers
    void handleFeatureMsg(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    void handleTreeMsg(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    void handleTreeRejectMsg(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    void handleAggregateMsg(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    void handleResultMsg(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);

    string onInterfaceDraw() override;

    static BlockCode *buildNewBlockCode(BuildingBlock *host) {
        return (new AI4PMCode_Embedded3D((BlinkyBlocksBlock*)host));
    }
};

#endif /* AI4PMCode_Embedded3D_H_ */

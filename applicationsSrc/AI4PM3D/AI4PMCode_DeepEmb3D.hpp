#ifndef AI4PMCode_DeepEmb3D_H_
#define AI4PMCode_DeepEmb3D_H_

/**
 * @file AI4PMCode_DeepEmb3D.hpp
 * Deep Embedded 3D Distributed CNN — 3-Layer Architecture
 *
 * NO base layer needed. Every block in the shape participates in inference.
 * Roles are assigned dynamically (no hardcoded positions).
 *
 * Phases:
 *   1. Local Conv1: 3×3×3 query → K features [0 messages]
 *   2. Spread 1: Send Conv1 to 6 neighbors → Conv2 [6N messages]
 *   3. Spread 2: Send Conv2 to 6 neighbors → Conv3 [6N messages]
 *   4. Spanning Tree: Build BFS tree [~N messages]
 *   5. Aggregation: Tree sum + count [~N messages]
 *   6. Classification: Root FC → Broadcast [~N messages]
 */

#include "robots/blinkyBlocks/blinkyBlocksSimulator.h"
#include "robots/blinkyBlocks/blinkyBlocksWorld.h"
#include "robots/blinkyBlocks/blinkyBlocksBlockCode.h"
#include "neuralNetworkData_deep_embedded3d.h"
#include <vector>
#include <cmath>
#include <cstring>

using namespace BlinkyBlocks;

// ── Message Type IDs ──
static const int FEATURE1_MSG_ID     = 4001;
static const int FEATURE2_MSG_ID     = 4002;
static const int TREE_MSG_ID         = 4003;
static const int TREE_REJECT_MSG_ID  = 4004;
static const int AGGREGATE_MSG_ID    = 4005;
static const int RESULT_MSG_ID       = 4006;
static const int INIT_MSG_ID         = 4007;

// ── Message Data Structures ──

struct InitMsgData {
    bool isInterior;
    InitMsgData() : isInterior(false) {}
    InitMsgData(bool b) : isInterior(b) {}
};

struct Feature1MsgData {
    float features[DEEP_K];
    Feature1MsgData() { memset(features, 0, sizeof(features)); }
    Feature1MsgData(const float* f) {
        for (int i = 0; i < DEEP_K; i++) features[i] = f[i];
    }
};

struct Feature2MsgData {
    float features[DEEP_K];
    Feature2MsgData() { memset(features, 0, sizeof(features)); }
    Feature2MsgData(const float* f) {
        for (int i = 0; i < DEEP_K; i++) features[i] = f[i];
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
    float sum[DEEP_K];
    int count;
    AggregateMsgData() : count(0) { memset(sum, 0, sizeof(sum)); }
    AggregateMsgData(const float* s, int c) : count(c) {
        for (int i = 0; i < DEEP_K; i++) sum[i] = s[i];
    }
};

struct ResultMsgData {
    int predictedClass;
    float probabilities[DEEP_NUM_CLASSES];
    ResultMsgData() : predictedClass(-1) { memset(probabilities, 0, sizeof(probabilities)); }
    ResultMsgData(int cls, const float* probs) : predictedClass(cls) {
        for (int i = 0; i < DEEP_NUM_CLASSES; i++) probabilities[i] = probs[i];
    }
};

/**
 * Direction-to-FaceGather-slot mapping.
 * Python FaceGatherLayer concat order:
 *   [own=0, x_minus=1, x_plus=2, y_minus=3, y_plus=4, z_minus=5, z_plus=6]
 *
 * SCLattice directions (from lattice.cpp nCells):
 *   South =0 → (1,0,0)  → +X → slot 2 (x_plus)
 *   East  =1 → (0,1,0)  → +Y → slot 4 (y_plus)
 *   North =2 → (-1,0,0) → -X → slot 1 (x_minus)
 *   West  =3 → (0,-1,0) → -Y → slot 3 (y_minus)
 *   Bottom=4 → (0,0,-1) → -Z → slot 5 (z_minus)
 *   Top   =5 → (0,0,1)  → +Z → slot 6 (z_plus)
 */
static const int DIR_TO_SLOT[6] = {2, 4, 1, 3, 5, 6};

enum DeepEmbedded3DRole {
    ROLE_UNDETERMINED,
    ROLE_LEAF,
    ROLE_INNER,
    ROLE_ROOT
};

class AI4PMCode_DeepEmb3D : public BlinkyBlocksBlockCode {
private:
    BlinkyBlocksBlock *module = nullptr;
    static int currentPhase; 

    // ── Phase 1: Local Features ──
    float occupancy[3][3][3];
    float conv1_out[DEEP_K];
    int numConnected;
    bool neighborPresent[6];

    // ── Phase 2: Feature Spreading 1 (Conv2) ──
    int features1Received;
    float neighbor_features1[7][DEEP_K];  // [own, slot1..slot6]
    float conv2_out[DEEP_K];
    bool conv2Done;

    // ── Phase 3: Feature Spreading 2 (Conv3) ──
    int features2Received;
    float neighbor_features2[7][DEEP_K];
    float conv3_out[DEEP_K];
    bool conv3Done;

    // ── Phase 4: Spanning Tree ──
    DeepEmbedded3DRole role;
    bool inTree;
    int parentDirection;
    int numExpectedChildren;
    int childrenDone;
    int rejectsReceived;
    bool isChild[6];     // Track which directions are actual tree children

    // ── Phase 5: Aggregation ──
    float aggSum[DEEP_K];
    int aggCount;
    bool aggregationSent;

    // ── Phase 6: Result ──
    int predictedClass;
    float probabilities[DEEP_NUM_CLASSES];

    // ── Hollow-Out Optimization (Strategy 3) ──
    bool isInterior;
    int activeNeighbors;
    int initMsgsReceived;
    bool neighborIsInterior[6];
    static bool hollowMode;
    static int strategyMode; // 1 = Solid, 2 = Indexing (Strategy 2), 3 = Hollow (Strategy 3)
    float default_interior_conv1[DEEP_K];
    float default_interior_conv2[DEEP_K];
    float default_interior_conv3[DEEP_K];

    // ── Helpers ──
    int getDirection(P2PNetworkInterface* iface);
    void computeConv1();
    void computeConv2();
    void computeConv3();
    void checkReadyToAggregate();
    void sendAggregateToParent();
    void classify();
    void broadcastResult();
    void startCNNPhases();
    void computeDefaultInteriorFeatures();
    void sendDeepMessage(Message* msg, P2PNetworkInterface* itf, uint64_t commTime, uint64_t delay);

public:
    AI4PMCode_DeepEmb3D(BlinkyBlocksBlock *host);
    ~AI4PMCode_DeepEmb3D() {};

    void startup() override;
    void parseUserBlockElements(TiXmlElement *config) override;

    // Message handlers
    void handleInitMsg(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    void handleFeature1Msg(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    void handleFeature2Msg(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    void handleTreeMsg(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    void handleTreeRejectMsg(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    void handleAggregateMsg(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);
    void handleResultMsg(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender);

    string onInterfaceDraw() override;

    static BlockCode *buildNewBlockCode(BuildingBlock *host) {
        return (new AI4PMCode_DeepEmb3D((BlinkyBlocksBlock*)host));
    }
};

#endif /* AI4PMCode_DeepEmb3D_H_ */

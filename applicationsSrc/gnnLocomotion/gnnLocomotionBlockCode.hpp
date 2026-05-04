#pragma once
#include "robots/slidingCubes/slidingCubesSimulator.h"
#include "robots/slidingCubes/slidingCubesWorld.h"
#include "robots/slidingCubes/slidingCubesBlockCode.h"
#include "robots/slidingCubes/slidingCubesBlock.h"
#include "events/events.h"
#include "gymServer.hpp"
#include "gcnPolicy.hpp"
#include <array>
#include <cstdint>
#include <map>
#include <queue>
#include <string>
#include <utility>
#include <vector>

namespace GNNLocomotion {

// Payload values for InterruptionEvent<int>
static constexpr int GYM_TICK              = 1; // periodic check for a pending step request
static constexpr int MOVE_DONE             = 2; // a block finished its motion
static constexpr int DEPLOY_STEP_START     = 3;
// Phase watchdog ids — fire if not all neighbour messages arrived in time.
static constexpr int DEPLOY_TIMEOUT_TOPO1  = 10;
static constexpr int DEPLOY_TIMEOUT_TOPO2  = 11;
static constexpr int DEPLOY_TIMEOUT_LAYER1 = 20;
static constexpr int DEPLOY_TIMEOUT_LAYER2 = 21;
static constexpr int DEPLOY_TIMEOUT_LAYER3 = 22;

// Deploy P2P message type ids
static constexpr int MSG_TOPO_1HOP   = 100;
static constexpr int MSG_TOPO_2HOP   = 101;
static constexpr int MSG_GNN_LAYER1  = 200;
static constexpr int MSG_GNN_LAYER2  = 201;
static constexpr int MSG_GNN_LAYER3  = 202;

static constexpr int MAX_STEPS  = 200;
static constexpr int GYM_PORT   = 9999;

// Tick period used while waiting for a step request from Python (microseconds)
static constexpr Time TICK_IDLE_US  = 5'000;
// Tick period after sending an obs (gives Python time to respond)
static constexpr Time TICK_AFTER_US = 1'000;

// Deploy-mode timing: each "wireless round" is this long.
static constexpr Time ROUND_DT_US   = 2'000;

// Deploy-mode message payloads
struct TopoPayload {
    uint32_t step;
    bID      id;
    uint8_t  mask6;       // bit d set iff direction d has a connected neighbour
};

struct Topo2Payload {
    uint32_t step;
    bID      id;
    uint8_t  mask6;
    bID      nbIds[6];    // -1 (0) sentinel for missing
};

struct GNNMsgPayload {
    uint32_t step;
    uint8_t  fromDir;     // direction index (0..5) of receiver as seen by sender
    float    h[HIDDEN];
};

class GNNLocomotionCode : public SlidingCubes::SlidingCubesBlockCode {
private:
    SlidingCubes::SlidingCubesBlock* module = nullptr;
    bool isLeader = false;

    // ---- Training/leader runtime state ----
    int    currentStep    = 0;
    int    prevInTarget   = 0;
    bool   episodeDone    = false;
    double pendingPenalty = 0.0;

    std::queue<std::pair<SlidingCubes::SlidingCubesBlock*, Cell3DPosition>> moveQueue;

    std::vector<Cell3DPosition> targetCells;
    bool targetCacheBuilt = false;

    // ---- Deploy-mode runtime state ----
    bool          deployMode    = false;
    std::string   weightsPath;
    uint32_t      globalSeed    = 42;
    uint32_t      deployStep    = 0;
    bool          motionPending    = false;
    Cell3DPosition pendingDest     = {};  // destination reserved in claimedDests
    // Anti-oscillation: short circular history of recently occupied positions.
    // If the next action would revisit any of them, the action is suppressed.
    static constexpr int HIST_LEN = 4;
    std::array<Cell3DPosition, HIST_LEN> posHistory{};
    int posHistoryLen = 0;

    // Phase A buffers
    uint8_t                       nb1mask = 0;
    std::array<bID, 6>            nb1ids{};
    int                           topo1Received = 0;
    int                           topo2Received = 0;
    std::array<TopoPayload, 6>    topo1Buf{};   // indexed by direction
    std::array<Topo2Payload, 6>   topo2Buf{};
    std::array<bool, 6>           topo1Got{};
    std::array<bool, 6>           topo2Got{};

    // GNN forward state
    Eigen::VectorXf               h_self;
    std::array<std::vector<Eigen::VectorXf>, N_LAYERS> incoming;
    int                           layerMsgReceived[N_LAYERS] = {0, 0, 0};

    // Idempotency flags: each phase only advances once per step.
    bool topo2Sent      = false;
    bool topo2Advanced  = false;
    bool layerAdvanced[N_LAYERS] = {false, false, false};

    uint64_t                      rngState = 1;

public:
    explicit GNNLocomotionCode(SlidingCubes::SlidingCubesBlock* host);
    ~GNNLocomotionCode() override = default;

    void startup() override;
    void onMotionEnd() override;
    void onInterruptionEvent(std::shared_ptr<Event> event) override;

    static BlockCode* buildNewBlockCode(BaseSimulator::BuildingBlock* host) {
        return new GNNLocomotionCode(
            static_cast<SlidingCubes::SlidingCubesBlock*>(host));
    }

private:
    // ---- Training (gym) path ----
    void scheduleGymTick(Time delayUs = TICK_IDLE_US);
    void onGymTick();
    void executeActions(std::map<bID, int> actions);
    void executeNextMove();
    void onAllMovesComplete();

    bool isArticulationPoint(const SlidingCubes::SlidingCubesBlock* sc) const;

    GymObs buildObs();
    std::vector<Cell3DPosition>& getTargetCells();
    int countBlocksInTarget() const;
    double computeReward(int prevIn, int newIn, bool done) const;

    // ---- Deploy path ----
    void parseDeployConfig();
    void onDeployStartup();
    void onDeployStepStart();
    void clearDeployStepBuffers();
    int  expectedNeighborCount() const;

    // Phase A: topology gathering
    void onTopo1Msg(std::shared_ptr<Message> m, P2PNetworkInterface* sender);
    void onTopo2Msg(std::shared_ptr<Message> m, P2PNetworkInterface* sender);
    void maybeAdvanceFromTopo1();
    void maybeAdvanceFromTopo2();
    void emitTopo2();
    bool localIsArticulationPoint() const;

    // Phase B: GNN forward
    void emitLayerMessages(int layer);
    void onGnnLayerMsg(int layer,
                       std::shared_ptr<Message> m,
                       P2PNetworkInterface* sender);
    void maybeAdvanceLayer(int layer);

    // Phase C: act
    void runActorAndMove();
};

} // namespace GNNLocomotion

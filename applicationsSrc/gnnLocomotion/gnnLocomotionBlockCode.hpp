#pragma once
#include "robots/slidingCubes/slidingCubesSimulator.h"
#include "robots/slidingCubes/slidingCubesWorld.h"
#include "robots/slidingCubes/slidingCubesBlockCode.h"
#include "robots/slidingCubes/slidingCubesBlock.h"
#include "events/events.h"
#include "gymServer.hpp"
#include <map>
#include <queue>
#include <utility>
#include <vector>

namespace GNNLocomotion {

// Payload values for InterruptionEvent<int>
static constexpr int GYM_TICK   = 1; // periodic check for a pending step request
static constexpr int MOVE_DONE  = 2; // a block finished its motion

static constexpr int MAX_STEPS  = 200;
static constexpr int GYM_PORT   = 9999;

// Tick period used while waiting for a step request from Python (microseconds)
static constexpr Time TICK_IDLE_US  = 5'000;
// Tick period after sending an obs (gives Python time to respond)
static constexpr Time TICK_AFTER_US = 1'000;

class GNNLocomotionCode : public SlidingCubes::SlidingCubesBlockCode {
private:
    SlidingCubes::SlidingCubesBlock* module = nullptr;
    bool isLeader = false;

    // Leader-only runtime state
    int    currentStep    = 0;
    int    prevInTarget   = 0;
    bool   episodeDone    = false;
    double pendingPenalty = 0.0;  // added to reward on the next onAllMovesComplete()

    std::queue<std::pair<SlidingCubes::SlidingCubesBlock*, Cell3DPosition>> moveQueue;

    // Cached once on first use (target doesn't change during an episode)
    std::vector<Cell3DPosition> targetCells;
    bool targetCacheBuilt = false;

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
    // Scheduling helpers
    void scheduleGymTick(Time delayUs = TICK_IDLE_US);
    void scheduleMoveNotify();

    // Gym step logic (leader only)
    void onGymTick();
    void executeActions(std::map<bID, int> actions);
    void executeNextMove();
    void onAllMovesComplete();

    // Connectivity check
    bool isArticulationPoint(const SlidingCubes::SlidingCubesBlock* sc) const;

    // Observation helpers
    GymObs buildObs();
    std::vector<Cell3DPosition>& getTargetCells();
    int countBlocksInTarget() const;
    double computeReward(int prevIn, int newIn, bool done) const;
};

} // namespace GNNLocomotion

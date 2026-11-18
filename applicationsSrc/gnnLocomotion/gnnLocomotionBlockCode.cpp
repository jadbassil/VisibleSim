#include "gnnLocomotionBlockCode.hpp"
#include "robots/slidingCubes/slidingCubesBlock.h"
#include "grid/lattice.h"
#include "base/world.h"
#include <algorithm>
#include <iostream>
#include <queue>
#include <unordered_set>

using namespace BaseSimulator;

namespace GNNLocomotion {

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

GNNLocomotionCode::GNNLocomotionCode(SlidingCubes::SlidingCubesBlock* host)
    : SlidingCubes::SlidingCubesBlockCode(host), module(host) {}

// ---------------------------------------------------------------------------
// startup  — called once at t=0 for every block
// ---------------------------------------------------------------------------

void GNNLocomotionCode::startup() {
    // Leader = block with the smallest ID in the world
    bID minId = module->blockId;
    for (auto& [id, _] : BaseSimulator::getWorld()->getMap())
        if (id < minId) minId = id;
    isLeader = (module->blockId == minId);

    if (!isLeader) return;

    std::cout << "[GNN] Leader block " << module->blockId
              << " — starting gym server on port " << GYM_PORT << "\n";

    GymServer* gs = GymServer::getInstance();

    // When any block finishes a move, schedule a MOVE_DONE interruption on the
    // leader so we can drive the move queue from the scheduler thread.
    gs->setMoveCompleteCallback([this] {
        scheduler->schedule(
            new InterruptionEvent<int>(scheduler->now(), module, MOVE_DONE));
    });

    gs->start(GYM_PORT);

    // Post the initial observation; the socket thread will buffer it until
    // the Python client connects (cv_obs predicate stays true).
    gs->setObs(buildObs());

    scheduleGymTick(TICK_IDLE_US);
}

// ---------------------------------------------------------------------------
// onMotionEnd  — called on the block that just finished moving
// ---------------------------------------------------------------------------

void GNNLocomotionCode::onMotionEnd() {
    GymServer* gs = GymServer::getInstance();
    if (gs) gs->notifyMoveDone();
}

// ---------------------------------------------------------------------------
// onInterruptionEvent
// ---------------------------------------------------------------------------

void GNNLocomotionCode::onInterruptionEvent(std::shared_ptr<Event> event) {
    if (!isLeader) return;

    auto* ie = dynamic_cast<InterruptionEvent<int>*>(event.get());
    if (!ie) return;

    switch (ie->data) {
        case GYM_TICK:  onGymTick();      break;
        case MOVE_DONE: executeNextMove(); break;
        default: break;
    }
}

// ---------------------------------------------------------------------------
// Scheduling helpers
// ---------------------------------------------------------------------------

void GNNLocomotionCode::scheduleGymTick(Time delayUs) {
    scheduler->schedule(
        new InterruptionEvent<int>(scheduler->now() + delayUs, module, GYM_TICK));
}

// ---------------------------------------------------------------------------
// Gym step logic
// ---------------------------------------------------------------------------

void GNNLocomotionCode::onGymTick() {
    if (episodeDone) return;

    if (!GymServer::getInstance()->hasStep()) {
        scheduleGymTick(TICK_IDLE_US);
        return;
    }

    auto actions = GymServer::getInstance()->getAndClearStep();
    prevInTarget = countBlocksInTarget();
    executeActions(std::move(actions));
}

void GNNLocomotionCode::executeActions(std::map<bID, int> actions) {
    // Drain previous queue (safety)
    while (!moveQueue.empty()) moveQueue.pop();

    auto& worldMap = BaseSimulator::getWorld()->getMap();

    // Process blocks in ascending ID order for determinism
    std::vector<bID> ids;
    ids.reserve(actions.size());
    for (auto& [id, _] : actions) ids.push_back(id);
    std::sort(ids.begin(), ids.end());

    for (bID blockId : ids) {
        int moveIdx = actions[blockId];
        if (moveIdx == 0) continue; // 0 = stay

        auto it = worldMap.find(blockId);
        if (it == worldMap.end()) continue;

        auto* sc = dynamic_cast<SlidingCubes::SlidingCubesBlock*>(it->second);
        if (!sc) continue;

        auto moves = sc->getAllMotions();
        int idx = moveIdx - 1; // actions are 1-indexed; 0 = stay
        if (idx >= 0 && idx < static_cast<int>(moves.size()))
            moveQueue.push({sc, moves[idx].first});
    }

    executeNextMove();
}

bool GNNLocomotionCode::isArticulationPoint(const SlidingCubes::SlidingCubesBlock* sc) const {
    auto& worldMap = BaseSimulator::getWorld()->getMap();
    if (worldMap.size() <= 2) return false;

    bID removedId = sc->blockId;

    // Collect all block IDs except the one being tested
    std::unordered_set<bID> remaining;
    for (auto& [id, _] : worldMap)
        if (id != removedId) remaining.insert(id);

    if (remaining.empty()) return false;

    // BFS from an arbitrary remaining block
    auto* lattice = BaseSimulator::getWorld()->lattice;
    bID startId   = *remaining.begin();
    std::unordered_set<bID> visited;
    std::queue<bID> bfsQ;
    visited.insert(startId);
    bfsQ.push(startId);

    while (!bfsQ.empty()) {
        bID cur = bfsQ.front();
        bfsQ.pop();
        auto* block = dynamic_cast<SlidingCubes::SlidingCubesBlock*>(worldMap.at(cur));
        if (!block) continue;
        for (int d = 0; d < 6; d++) {
            auto* iface = block->getInterface(SCLattice2::Direction(d));
            if (!iface || !iface->isConnected()) continue;
            bID nbId = iface->getConnectedBlockBId();
            if (remaining.count(nbId) && !visited.count(nbId)) {
                visited.insert(nbId);
                bfsQ.push(nbId);
            }
        }
    }

    return visited.size() < remaining.size();
}

void GNNLocomotionCode::executeNextMove() {
    if (episodeDone) return;

    // Execute at most ONE block move per gym step so the Python policy always
    // observes the updated state before choosing the next action.  This avoids
    // intermediate connectivity states that were never visible to the policy.
    while (!moveQueue.empty()) {
        auto [sc, dest] = moveQueue.front();
        moveQueue.pop();

        // Skip if the block no longer exists
        auto& worldMap = BaseSimulator::getWorld()->getMap();
        if (!worldMap.count(sc->blockId)) continue;

        // Connectivity guard: penalise and end episode if this block is a cut vertex
        if (isArticulationPoint(sc)) {
            while (!moveQueue.empty()) moveQueue.pop();
            pendingPenalty = -10.0;
            episodeDone    = true;
            break;
        }

        if (sc->moveTo(dest)) {
            // One block started moving — discard the rest for this step.
            // The policy will re-evaluate the updated state on the next step.
            while (!moveQueue.empty()) moveQueue.pop();
            return; // wait for MOVE_DONE interruption
        }
        // moveTo failed (destination occupied) — try the next block in the queue
    }

    onAllMovesComplete();
}

void GNNLocomotionCode::onAllMovesComplete() {
    currentStep++;
    int newInTarget = countBlocksInTarget();
    int totalTarget = static_cast<int>(getTargetCells().size());

    bool shapeDone = (totalTarget > 0 && newInTarget >= totalTarget);
    bool timeDone  = (currentStep >= MAX_STEPS);
    episodeDone    = episodeDone || shapeDone || timeDone;

    GymObs obs   = buildObs();
    obs.reward   = computeReward(prevInTarget, newInTarget, episodeDone) + pendingPenalty;
    obs.done     = episodeDone;
    pendingPenalty = 0.0;

    GymServer::getInstance()->setObs(obs);

    if (!episodeDone)
        scheduleGymTick(TICK_AFTER_US);
    // If done, we let the scheduler drain naturally; Python will kill/restart.
}

// ---------------------------------------------------------------------------
// Observation builder
// ---------------------------------------------------------------------------

GymObs GNNLocomotionCode::buildObs() {
    GymObs obs;
    obs.step     = currentStep;
    obs.maxSteps = MAX_STEPS;

    auto* latt = BaseSimulator::getWorld()->lattice;
    obs.gridSize[0] = latt->gridSize[0];
    obs.gridSize[1] = latt->gridSize[1];
    obs.gridSize[2] = latt->gridSize[2];

    for (auto& [id, bb] : BaseSimulator::getWorld()->getMap()) {
        auto* sc = dynamic_cast<SlidingCubes::SlidingCubesBlock*>(bb);
        if (!sc) continue;

        BlockObs b;
        b.id       = id;
        b.pos      = sc->position;
        b.inTarget = (target != nullptr && target->isInTarget(sc->position));

        for (int d = 0; d < 6; d++) {
            auto* iface = sc->getInterface(SCLattice2::Direction(d));
            b.neighbors[d] = iface->isConnected()
                ? static_cast<int>(iface->getConnectedBlockBId())
                : -1;
        }

        for (auto& [pos, orient] : sc->getAllMotions())
            b.moves.push_back(pos);

        obs.blocks.push_back(std::move(b));
    }

    obs.target = getTargetCells();
    return obs;
}

// ---------------------------------------------------------------------------
// Target cell cache — built once and reused for the episode
// ---------------------------------------------------------------------------

std::vector<Cell3DPosition>& GNNLocomotionCode::getTargetCells() {
    if (!targetCacheBuilt) {
        targetCacheBuilt = true;
        if (target) {
            auto* latt = BaseSimulator::getWorld()->lattice;
            for (int x = 0; x < latt->gridSize[0]; x++)
                for (int y = 0; y < latt->gridSize[1]; y++)
                    for (int z = 0; z < latt->gridSize[2]; z++) {
                        Cell3DPosition pos(x, y, z);
                        if (target->isInTarget(pos))
                            targetCells.push_back(pos);
                    }
        }
    }
    return targetCells;
}

int GNNLocomotionCode::countBlocksInTarget() const {
    if (!target) return 0;
    int count = 0;
    for (auto& [id, bb] : BaseSimulator::getWorld()->getMap())
        if (target->isInTarget(bb->position)) count++;
    return count;
}

double GNNLocomotionCode::computeReward(int prevIn, int newIn, bool done) const {
    double r = static_cast<double>(newIn - prevIn); // +1 per newly placed block
    r -= 0.01;                                       // time penalty
    if (done && newIn >= static_cast<int>(targetCells.size()))
        r += 10.0; // completion bonus
    return r;
}

} // namespace GNNLocomotion

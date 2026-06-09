/**
 * @file   millionModuleMarchBlockCode.cpp
 * @brief  Implementation of the Million Module March locomotion algorithm
 *         (Fitch & Butler, IJRR 2008) for VisibleSim 3D catoms.
 */

#include <queue>
#include <utility>

#include "millionModuleMarchBlockCode.hpp"

using namespace std;

/// Period of a module's self "attempt-to-move" liveness tick (us). Motion is
/// mostly event-driven (motion-end / neighbour-change); this is a safety net
/// that retries modules that are momentarily blocked.
static const Time TICK_PERIOD = 20000;
/// Period of the master's goal-completion check (us).
static const Time MASTER_PERIOD = 5000;
/// Bounds for the local connecting-cycle search.
static const int CC_MAX_DEPTH = 8;
static const int CC_MAX_EXPAND = 80;

/// Destination cells currently claimed by a mover. The single-threaded event
/// loop makes "test, then claim" atomic, which is exactly the mutual-exclusion
/// outcome the paper's distributed test-and-set achieves among the neighbours
/// of a destination cell. In a physically distributed system this set would be
/// replaced by the locking message round on those neighbours (acknowledged so
/// no lock can leak); modelling it as an atomic claim keeps the demo robust.
static std::set<Cell3DPosition> g_lockedCells;
/// Modules currently reserved as a rotation pivot. A reserved pivot will not
/// move (so it stays put under the module rotating around it), and no second
/// mover may use it at the same time -- the paper's pivot/test-and-set locking.
static std::set<bID> g_busyPivots;

/// Shared value function: distance-to-goal of every relevant lattice cell
/// (the converged dynamic-programming result; see computeField).
static std::map<Cell3DPosition, int> g_cellDist;
/// Set when the structure or goal changes; triggers a lazy field refresh.
static bool g_fieldDirty = true;
/// Horizontal centroid of the current goal box (keeps the robot compact).
static double g_goalCx = 0.0, g_goalCy = 0.0;
/// Simulation time of the most recent completed motion (explicit-box mode:
/// the goal advances once the robot has settled on the current box).
static Time g_lastMotionTime = 0;
/// March mode: furthest-forward trailing edge of the robot, and when it last
/// advanced -- used to nudge the goal past an occasional tail that gets stuck.
static int  g_trailMetric  = INT_MIN;
static Time g_lastProgressT = 0;

// --- Self-generated marching goal --------------------------------------------
// Instead of hand-authored target boxes, the goal can be the robot's own
// bounding box translated forward by one body length along a chosen axis. Each
// time it is reached the goal steps forward again, so the robot walks by
// repeatedly re-forming its shape one body-length ahead -- the paper's
// flat-ground experiment ("move forward N module widths and reform the cube",
// Section 5.2). This mode is used when the configuration has no <targetList>;
// the axis / direction / step count come from <world> attributes marchAxis
// ("x"|"y"|"z"), marchDir (+1/-1) and marchSteps. When a <targetList> is present
// the app instead marches through those explicit boxes (g_dynamicMode == false).
static bool g_modeResolved = false;
static bool g_dynamicMode  = false;
static int  g_marchAxis    = 0;       // 0=x, 1=y (horizontal), 2=z (vertical)
static int  g_marchDir     = 1;       // +1 / -1
static int  g_marchSteps   = 3;       // number of advances to walk
static int  g_marchStride  = 1;       // cells the goal slides per advance
static bool g_marchParsed  = false;

static Cell3DPosition g_originMin, g_originMax; // robot bbox captured at start
static Cell3DPosition g_goalMin, g_goalMax;     // current goal window (inclusive)
static int  g_progress  = 0;          // cells advanced from the origin so far
static int  g_stepIndex = 0;          // which step we are on (0 == not started)
static bool g_goalReady = false;

/// Return c with component `axis` shifted by `d`.
static Cell3DPosition shiftAxis(const Cell3DPosition &c, int axis, int d) {
    Cell3DPosition r = c;
    r.pt[axis] = static_cast<short>(c[axis] + d);
    return r;
}

MillionModuleMarchCode::MillionModuleMarchCode(Catoms3DBlock *host)
    : Catoms3DBlockCode(host), module(host) {
    if (host == nullptr) return;

    // The lowest-id module acts as the "higher-level process" that shifts the
    // goal bounding box once the current one is filled (paper, Section 4/5).
    isMaster = (host->blockId == 1);
}

// ===========================================================================
//  Lifecycle
// ===========================================================================

void MillionModuleMarchCode::startup() {
    lattice = Catoms3DWorld::getWorld()->lattice;

    // Resolve the goal mode once: a self-generated marching box if the config
    // supplied no <targetList> (target stays null), otherwise the XML boxes.
    if (!g_modeResolved) {
        g_dynamicMode = (target == nullptr);
        g_modeResolved = true;
    }

    if (isObstacle) {
        // Fixed terrain: occupies a lattice cell (a barrier for the motion
        // engine), but never moves and never joins the DP field.
        distanceToGoal = INT_MAX;
        refreshColor();
        return;
    }

    recomputeDistance();
    refreshColor();

    // Stagger the initial tick a little so modules desynchronise.
    scheduleMoveTick((module->blockId % 16) * 30);

    if (isMaster) {
        if (g_dynamicMode && !g_goalReady) initMarchGoal();
        g_lastMotionTime = getScheduler()->now();   // fair first quiescence window
        getScheduler()->schedule(
            new InterruptionEvent<int>(getScheduler()->now() + MASTER_PERIOD,
                                       module, TICK_MASTER));
    }
}

void MillionModuleMarchCode::onMotionEnd() {
    moving = false;
    releaseClaims();                  // free our destination cell and pivot
    g_lastMotionTime = getScheduler()->now();
    g_fieldDirty = true;              // the structure changed: refresh the field
    recomputeDistance();
    refreshColor();
    attemptMove();                    // immediately try the next step (cascade)
}

void MillionModuleMarchCode::onInterruptionEvent(std::shared_ptr<Event> event) {
    int code = std::static_pointer_cast<InterruptionEvent<int>>(event)->data;

    if (code == TICK_MOVE) {
        if (!isObstacle) attemptMove();
        scheduleMoveTick();                 // keep the single periodic chain alive
    } else if (code == TICK_MASTER) {
        masterCheckGoalCompletion();
        getScheduler()->schedule(
            new InterruptionEvent<int>(getScheduler()->now() + MASTER_PERIOD,
                                       module, TICK_MASTER));
    }
}

void MillionModuleMarchCode::onNeighborChanged(uint64_t /*face*/, int /*action*/) {
    // A neighbour appeared or left: the connectivity graph changed, so the
    // distance field must be re-relaxed. Continuous replanning falls out of
    // reacting to every topology change (paper, Section 3).
    g_fieldDirty = true;
    if (!isObstacle) {
        attemptMove();                // a neighbour moved: re-evaluate our options
    }
    refreshColor();
}

void MillionModuleMarchCode::parseUserBlockElements(TiXmlElement *config) {
    const char *attr = config->Attribute("obstacle");
    if (attr != nullptr) {
        string str(attr);
        if (str == "true" || str == "1" || str == "yes") {
            isObstacle = true;
            isMaster = false;     // obstacles never coordinate the goal
        }
    }
}

void MillionModuleMarchCode::parseUserElements(TiXmlDocument *config) {
    if (g_marchParsed || config == nullptr) return;
    g_marchParsed = true;

    // Optional <world> attributes drive the self-generated marching goal:
    //   marchAxis="x|y|z"  marchDir="1|-1"  marchSteps="<n>"
    TiXmlElement *world = config->FirstChildElement("world");
    if (world == nullptr) return;
    const char *ax = world->Attribute("marchAxis");
    if (ax != nullptr) {
        string s(ax);
        g_marchAxis = (s == "y" || s == "Y") ? 1 : (s == "z" || s == "Z") ? 2 : 0;
    }
    int v;
    if (world->Attribute("marchDir", &v) != nullptr) g_marchDir = (v < 0) ? -1 : 1;
    if (world->Attribute("marchSteps", &v) != nullptr) g_marchSteps = v;
    if (world->Attribute("marchStride", &v) != nullptr) g_marchStride = (v < 1) ? 1 : v;
}

void MillionModuleMarchCode::onEndOfSimulation() {
    if (!isMaster) return;
    int nRobot = 0, nInTarget = 0;
    short minx = SHRT_MAX, maxx = SHRT_MIN;
    for (auto &kv : Catoms3DWorld::getWorld()->getMap()) {
        auto *bc = static_cast<MillionModuleMarchCode *>(kv.second->blockCode);
        if (bc == nullptr || bc->isObstacle) continue;
        ++nRobot;
        short x = kv.second->position[0];
        minx = std::min(minx, x);
        maxx = std::max(maxx, x);
        if (inGoal(kv.second->position)) ++nInTarget;
    }
    std::cout << "[MMM] END: robot modules=" << nRobot
              << " inCurrentTarget=" << nInTarget
              << " x-range=[" << minx << "," << maxx << "]"
              << " goalFinished=" << finished << std::endl;
}

void MillionModuleMarchCode::onBlockSelected() {
    console << "module " << module->blockId << " pos=" << module->position
            << " dist=" << distanceToGoal
            << " moving=" << moving
            << " pivotReserved=" << (g_busyPivots.count(module->blockId) ? 1 : 0) << "\n";
}

// ===========================================================================
//  Dynamic program (parallel path planning)
//
//  The MDP value function under the paper's deterministic -1/step reward is the
//  feasible-path distance from each lattice position to the goal region. We
//  compute exactly that field by a multi-source breadth-first search seeded at
//  the goal cells, with obstacle cells as barriers -- the gridworld value
//  function of the paper (Section 3.1). This is the *converged* result of the
//  distributed Bellman backups; computing it as a shared refresh on every
//  structural change avoids the count-to-infinity oscillation that asynchronous
//  Bellman-Ford suffers on a constantly-changing structure, while giving each
//  module the same optimal value it would converge to in the distributed scheme.
// ===========================================================================

bool MillionModuleMarchCode::inGoal(const Cell3DPosition &c) {
    if (g_dynamicMode) {
        if (!g_goalReady) return false;
        return c[0] >= g_goalMin[0] && c[0] <= g_goalMax[0]
            && c[1] >= g_goalMin[1] && c[1] <= g_goalMax[1]
            && c[2] >= g_goalMin[2] && c[2] <= g_goalMax[2];
    }
    return target != nullptr && target->isInTarget(c);
}

bool MillionModuleMarchCode::robotBBox(Cell3DPosition &mn, Cell3DPosition &mx) {
    int mnx = INT_MAX, mny = INT_MAX, mnz = INT_MAX;
    int mxx = INT_MIN, mxy = INT_MIN, mxz = INT_MIN;
    for (auto &kv : Catoms3DWorld::getWorld()->getMap()) {
        auto *bc = static_cast<MillionModuleMarchCode *>(kv.second->blockCode);
        if (bc != nullptr && bc->isObstacle) continue;          // robot only
        const Cell3DPosition &p = kv.second->position;
        mnx = std::min(mnx, (int)p[0]); mxx = std::max(mxx, (int)p[0]);
        mny = std::min(mny, (int)p[1]); mxy = std::max(mxy, (int)p[1]);
        mnz = std::min(mnz, (int)p[2]); mxz = std::max(mxz, (int)p[2]);
    }
    if (mxx < mnx) return false;                                 // no robot modules
    mn = Cell3DPosition(mnx, mny, mnz);
    mx = Cell3DPosition(mxx, mxy, mxz);
    return true;
}

void MillionModuleMarchCode::initMarchGoal() {
    if (!robotBBox(g_originMin, g_originMax)) return;
    // The goal is the robot's shape used as a window that slides forward by
    // g_marchStride cells per step. A small stride keeps each goal heavily
    // overlapping the current robot, so the robot flows forward continuously
    // (the paper's overlapping goal boxes) instead of having to teleport a
    // whole body length at once.
    g_progress = g_marchStride;
    const int shift = g_marchDir * g_progress;
    g_goalMin = shiftAxis(g_originMin, g_marchAxis, shift);
    g_goalMax = shiftAxis(g_originMax, g_marchAxis, shift);
    g_stepIndex = 1;
    g_goalReady = true;
}

bool MillionModuleMarchCode::advanceGoalRegion() {
    if (g_dynamicMode) {
        if (g_stepIndex >= g_marchSteps) return false;
        g_progress += g_marchStride;
        const int shift = g_marchDir * g_progress;
        g_goalMin = shiftAxis(g_originMin, g_marchAxis, shift);
        g_goalMax = shiftAxis(g_originMax, g_marchAxis, shift);
        ++g_stepIndex;
        return true;
    }
    return loadNextTarget();
}

void MillionModuleMarchCode::computeField() {
    g_cellDist.clear();
    if (g_dynamicMode) {
        if (!g_goalReady) return;          // goal is set at the master's startup
    } else if (target == nullptr) {
        return;
    }

    Lattice *lat = Catoms3DWorld::getWorld()->lattice;
    const Cell3DPosition gs = lat->gridSize;

    auto isObstacleCell = [&](const Cell3DPosition &c) {
        BuildingBlock *bb = lat->getBlock(c);
        if (bb == nullptr) return false;
        auto *bc = static_cast<MillionModuleMarchCode *>(bb->blockCode);
        return bc != nullptr && bc->isObstacle;
    };

    // Multi-source BFS seeded at every goal cell (reward 0), propagating over
    // the whole free+structure space with obstacle cells as barriers. This is
    // the gridworld value function of the paper (Section 3.1); covering the full
    // bounded grid -- rather than only structure-adjacent cells -- lets the
    // robot follow the gradient toward a goal box that is not yet touching it
    // (e.g. across a gap or beyond an obstacle).
    std::queue<Cell3DPosition> q;
    for (short x = 0; x < gs[0]; ++x)
        for (short y = 0; y < gs[1]; ++y)
            for (short z = 0; z < gs[2]; ++z) {
                const Cell3DPosition c(x, y, z);
                if (inGoal(c) && !isObstacleCell(c)) {
                    g_cellDist[c] = 0;
                    q.push(c);
                }
            }
    while (!q.empty()) {
        const Cell3DPosition c = q.front();
        q.pop();
        const int dc = g_cellDist[c];
        for (const Cell3DPosition &nb : lat->getNeighborhood(c)) {
            if (!lat->isInGrid(nb) || isObstacleCell(nb)) continue;  // barrier
            if (g_cellDist.find(nb) == g_cellDist.end()) {
                g_cellDist[nb] = dc + 1;
                q.push(nb);
            }
        }
    }

    // Horizontal centroid of the goal box, used as a soft tiebreak that keeps
    // the robot compact (prefer equal-distance moves toward the box centre).
    double sx = 0, sy = 0;
    long n = 0;
    for (short x = 0; x < gs[0]; ++x)
        for (short y = 0; y < gs[1]; ++y)
            for (short z = 0; z < gs[2]; ++z)
                if (inGoal(Cell3DPosition(x, y, z))) { sx += x; sy += y; ++n; }
    if (n > 0) { g_goalCx = sx / n; g_goalCy = sy / n; }
}

void MillionModuleMarchCode::recomputeDistance() {
    if (isObstacle) return;
    auto it = g_cellDist.find(module->position);
    int newD = (it != g_cellDist.end()) ? it->second : INT_MAX;
    if (newD != distanceToGoal) {
        distanceToGoal = newD;
        refreshColor();
    }
}

int MillionModuleMarchCode::estimatedDistanceOfCell(const Cell3DPosition &c) const {
    auto it = g_cellDist.find(c);
    return (it != g_cellDist.end()) ? it->second : INT_MAX;
}

// ===========================================================================
//  Action selection (greedy descent of the value function)
// ===========================================================================

bool MillionModuleMarchCode::selectBestMove(Cell3DPosition &bestDest,
                                            Catoms3DBlock *&bestPivot) const {
    const bool inTargetSelf = inGoal(module->position);
    const int D = distanceToGoal;

    int bestKey = INT_MAX;
    short bestZ = SHRT_MAX;
    double bestH = 1e18;
    bool found = false;
    bool foundStrict = false;   //!< prefer a strictly-closer move over a lateral one

    auto hdist = [](const Cell3DPosition &p) {
        const double dx = p[0] - g_goalCx, dy = p[1] - g_goalCy;
        return dx * dx + dy * dy;                      // horizontal dist^2 to goal box
    };

    for (const Cell3DPosition &c : Catoms3DMotionEngine::getAllReachablePositions(module)) {
        if (!lattice->isInGrid(c) || !lattice->isFree(c)) continue;

        const bool cInTarget = inGoal(c);
        int key;        //!< primary sort key (lower is better)
        bool strict;    //!< true == strict progress toward the goal

        if (inTargetSelf) {
            // Already placed: only settle deeper into the goal -- fill holes and
            // hug the ground (the reward's height term, paper Section 6).
            if (!(cInTarget && c[2] < module->position[2])) continue;
            key = 0;
            strict = true;
        } else {
            int ed = estimatedDistanceOfCell(c);
            if (ed == INT_MAX || D == INT_MAX) continue;
            if (ed < D) {
                strict = true;                        // genuinely closer to the goal
            } else if (ed == D &&
                       std::find(recentCells.begin(), recentCells.end(), c)
                           == recentCells.end()) {   // lateral surface flow (no loop)
                strict = false;
            } else {
                continue;                             // away, revisiting, or drifting
            }
            key = ed;
        }

        Catoms3DBlock *piv = Catoms3DMotionEngine::findMotionPivot(module, c);
        if (piv == nullptr) continue;       // no valid catom rotation reaches c

        // A strict-progress option always beats any lateral one.
        if (strict && !foundStrict) {
            foundStrict = true;
            found = false;
            bestKey = INT_MAX;
            bestH = 1e18;
            bestZ = SHRT_MAX;
        }
        if (!strict && foundStrict) continue;

        // Rank: closest to goal (key), then most goal-centroid-ward (compact),
        // then lowest (ground preference / dynamic stability).
        const double hc = hdist(c);
        const bool better = !found || key < bestKey ||
            (key == bestKey && (hc < bestH ||
             (hc == bestH && c[2] < bestZ)));
        if (better) {
            bestKey = key;
            bestH = hc;
            bestZ = c[2];
            bestDest = c;
            bestPivot = piv;
            found = true;
        }
    }
    return found;
}

void MillionModuleMarchCode::attemptMove() {
    if (isObstacle || moving) return;
    if (g_busyPivots.count(module->blockId)) return;  // reserved as someone's pivot
    if (module->getState() != BuildingBlock::State::ALIVE) return;

    // Refresh the converged value function if the structure/goal changed.
    if (g_fieldDirty) { computeField(); g_fieldDirty = false; }
    recomputeDistance();                              // adopt our own value

    Cell3DPosition dest;
    Catoms3DBlock *pivot = nullptr;
    if (!selectBestMove(dest, pivot)) return;         // no beneficial move right now

    // Parallel-actuation safety: never move if it would disconnect the robot.
    if (!isMobileByConnectingCycle()) return;

    // Test-and-set locking (atomic in this single-process event loop):
    //  - the destination cell, against any other mover targeting it;
    //  - the pivot, so it stays put and serves only one mover at a time.
    const bID pid = pivot->blockId;
    if (g_lockedCells.count(dest) || g_busyPivots.count(pid)) return;

    g_lockedCells.insert(dest);
    g_busyPivots.insert(pid);
    haveCellLock = true;
    pendingDest = dest;
    pendingPivotId = pid;

    try {
        moving = true;
        refreshColor();
        // Remember the cell we are leaving so lateral flow does not loop back.
        recentCells.push_back(module->position);
        if (recentCells.size() > 12) recentCells.pop_front();
        getScheduler()->schedule(new Catoms3DRotationStartEvent(
            getScheduler()->now() + 1, module, pivot, dest));
    } catch (std::exception const &e) {
        // The rotation turned out to be infeasible: abort and release the locks.
        console << "rotation aborted: " << e.what() << "\n";
        moving = false;
        releaseClaims();
        refreshColor();
    }
}

// ===========================================================================
//  Parallel actuation: connecting-cycle connectivity check
// ===========================================================================

bool MillionModuleMarchCode::isMobileByConnectingCycle() const {
    // Robot neighbours = adjacent non-obstacle modules (terrain is environment,
    // not part of the robot's connectivity graph).
    vector<Cell3DPosition> robotNb;
    for (const Cell3DPosition &n : lattice->getActiveNeighborCells(module->position)) {
        BuildingBlock *bb = lattice->getBlock(n);
        if (bb == nullptr) continue;
        auto *bc = static_cast<MillionModuleMarchCode *>(bb->blockCode);
        if (bc != nullptr && bc->isObstacle) continue;
        robotNb.push_back(n);
    }

    if (robotNb.size() <= 1) return true;             // a leaf is always mobile

    // Bounded iterative-deepening (BFS) search for a connecting cycle joining
    // all of our neighbours through other modules, avoiding our own cell
    // (paper, Section 4, Figure 6). If found within the bound, we are not an
    // articulation point and may safely move.
    const Cell3DPosition self = module->position;
    set<Cell3DPosition> targets(robotNb.begin() + 1, robotNb.end());
    set<Cell3DPosition> visited;
    queue<pair<Cell3DPosition, int>> q;

    visited.insert(robotNb[0]);
    targets.erase(robotNb[0]);
    q.push({robotNb[0], 0});

    int expand = 0;
    while (!q.empty() && expand < CC_MAX_EXPAND) {
        auto [cell, depth] = q.front();
        q.pop();
        ++expand;
        if (depth >= CC_MAX_DEPTH) continue;

        for (const Cell3DPosition &nb : lattice->getActiveNeighborCells(cell)) {
            if (nb == self) continue;                 // forbidden node (us)
            if (visited.count(nb)) continue;
            BuildingBlock *bb = lattice->getBlock(nb);
            if (bb == nullptr) continue;
            auto *bc = static_cast<MillionModuleMarchCode *>(bb->blockCode);
            if (bc != nullptr && bc->isObstacle) continue;

            visited.insert(nb);
            targets.erase(nb);
            if (targets.empty()) return true;         // all neighbours reconnected
            q.push({nb, depth + 1});
        }
    }
    return targets.empty();                           // conservative: false if bound hit
}

// ===========================================================================
//  Parallel actuation: release of test-and-set locks
// ===========================================================================

void MillionModuleMarchCode::releaseClaims() {
    if (haveCellLock) {
        g_lockedCells.erase(pendingDest);
        haveCellLock = false;
    }
    if (pendingPivotId != 0) {
        g_busyPivots.erase(pendingPivotId);
        pendingPivotId = 0;
    }
}

// ===========================================================================
//  Goal advancement (the "march")
// ===========================================================================

/// The robot is "settled" if no motion has completed for this many us.
static const Time QUIESCENCE_WINDOW = 1500000;

void MillionModuleMarchCode::masterCheckGoalCompletion() {
    if (finished) return;
    if (g_dynamicMode) { if (!g_goalReady) return; }
    else if (target == nullptr) return;

    // Explicit-box mode advances only on a settled structure; wait for stillness.
    // March mode advances continuously off the centroid, so it does not wait.
    if (!g_dynamicMode) {
        for (auto &kv : Catoms3DWorld::getWorld()->getMap()) {
            auto *bc = static_cast<MillionModuleMarchCode *>(kv.second->blockCode);
            if (bc != nullptr && !bc->isObstacle && bc->moving) return;
        }
    }

    const Time now = getScheduler()->now();
    const char *why = "filled";
    bool advance;

    if (g_dynamicMode) {
        // Primary rule: advance once the robot's TRAILING edge has entered the
        // goal window -- the whole body has translated into it. Waiting for the
        // back to catch up each step keeps the robot compact (no smearing) and
        // the goal never runs away. Fallback: if the tail gets momentarily stuck
        // but the robot still overlaps the window, nudge the goal forward to
        // pull it loose (the paper's higher-level process).
        int rmin = INT_MAX, rmax = INT_MIN;
        bool overlap = false;
        for (auto &kv : Catoms3DWorld::getWorld()->getMap()) {
            auto *bc = static_cast<MillionModuleMarchCode *>(kv.second->blockCode);
            if (bc == nullptr || bc->isObstacle) continue;
            const Cell3DPosition &p = kv.second->position;
            rmin = std::min(rmin, (int)p[g_marchAxis]);
            rmax = std::max(rmax, (int)p[g_marchAxis]);
            if (inGoal(p)) overlap = true;
        }
        const int trail = (g_marchDir > 0) ? rmin : -rmax;   // grows as tail advances
        if (trail > g_trailMetric) { g_trailMetric = trail; g_lastProgressT = now; }

        const bool reached = (g_marchDir > 0) ? (rmin >= g_goalMin[g_marchAxis])
                                              : (rmax <= g_goalMax[g_marchAxis]);
        const bool unstick = overlap && (now - g_lastProgressT) > QUIESCENCE_WINDOW;
        advance = reached || unstick;
        why = reached ? "reached" : "nudged";
    } else {
        // Explicit-box mode: every goal cell occupied, or the robot has settled
        // (handles last-module contention, paper Section 5.2).
        bool boxFilled = true;
        const Cell3DPosition gs = lattice->gridSize;
        for (short x = 0; x < gs[0] && boxFilled; ++x)
            for (short y = 0; y < gs[1] && boxFilled; ++y)
                for (short z = 0; z < gs[2] && boxFilled; ++z) {
                    const Cell3DPosition c(x, y, z);
                    if (inGoal(c) && !lattice->cellHasBlock(c)) boxFilled = false;
                }
        const bool settled = (now - g_lastMotionTime) > QUIESCENCE_WINDOW;
        advance = boxFilled || settled;
        why = boxFilled ? "filled" : "settled";
    }

    if (!advance) return;

    // Give the next step a fresh window before a stall can be nudged again,
    // so nudges are paced one window apart rather than firing every tick.
    g_lastProgressT = now;

    if (!advanceGoalRegion()) {
        finished = true;
        std::cout << "[MMM] *** ALL GOAL BOXES REACHED at t=" << now
                  << " (" << why << ") ***\n";
        return;
    }
    std::cout << "[MMM] *** GOAL ADVANCED to next box at t=" << now
              << " (" << why << ") ***\n";
    g_lastMotionTime = getScheduler()->now();   // fresh quiescence window for the new box

    // Re-seed the value function against the new goal and restart every module.
    g_fieldDirty = true;
    for (auto &kv : Catoms3DWorld::getWorld()->getMap()) {
        auto *bc = static_cast<MillionModuleMarchCode *>(kv.second->blockCode);
        if (bc != nullptr && !bc->isObstacle) bc->attemptMove();
    }
}

// ===========================================================================
//  Utilities
// ===========================================================================

void MillionModuleMarchCode::scheduleMoveTick(Time extraDelay) {
    getScheduler()->schedule(new InterruptionEvent<int>(
        getScheduler()->now() + TICK_PERIOD + extraDelay, module, TICK_MOVE));
}

void MillionModuleMarchCode::refreshColor() {
    if (isObstacle)                                  { setColor(DARKGREY);  return; }
    if (moving)                                      { setColor(WHITE);     return; }
    if (g_busyPivots.count(module->blockId))         { setColor(RED);       return; }
    if (inGoal(module->position)) { setColor(GREEN); return; }
    if (distanceToGoal == INT_MAX)                   { setColor(LIGHTGREY); return; }
    setColor(Colors[distanceToGoal % NB_COLORS]);
}

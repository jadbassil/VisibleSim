/**
 * @file   millionModuleMarchBlockCode.hpp
 * @brief  Locomotion block code for 3D catoms (Million Module March).
 *
 * Adaptation of the "Million Module March" algorithm (Fitch & Butler, IJRR 2008)
 * to VisibleSim Catoms3D. Two pillars:
 *
 *   1. Parallel path planning: a dynamic program assigns every lattice cell its
 *      distance to the goal region (multi-source BFS from the goal cells, with
 *      obstacles as barriers). Under the paper's deterministic -1/step reward
 *      this distance field is exactly the optimal MDP value function. A module
 *      follows the policy by rotating to the reachable cell that descends the
 *      field (preferring the goal, then lower positions for dynamic stability).
 *
 *   2. Parallel actuation: a module moves only when it will not disconnect the
 *      structure (bounded local connecting-cycle search) and only after a
 *      test-and-set lock of its destination cell and rotation pivot. The set of
 *      legal moves is the set of single catom rotations
 *      (Catoms3DMotionEngine::getAllReachablePositions), which already encodes
 *      the 3D-catom motion constraints (pivot availability, free destination
 *      connector, collision-free swept volume).
 *
 * The goal is a sequence of overlapping bounding boxes; when the current box is
 * filled the goal advances (loadNextTarget), making the robot "march".
 *
 * @note Implementation realism: the simulator is a single-process discrete-event
 *   engine, so the value function and the locks are realized as shared state
 *   (the converged DP result, and atomic test-and-set claims). These are the
 *   exact outcomes the paper's distributed Bellman backups and locking message
 *   rounds converge to; using them directly avoids the count-to-infinity
 *   oscillation and lost-message lock leaks that the asynchronous message
 *   versions exhibit on a constantly-changing structure. See README.md.
 */

#ifndef MillionModuleMarchBlockCode_H_
#define MillionModuleMarchBlockCode_H_

#include <climits>
#include <deque>
#include <map>
#include <set>
#include <vector>

#include "robots/catoms3D/catoms3DWorld.h"
#include "robots/catoms3D/catoms3DBlockCode.h"
#include "robots/catoms3D/catoms3DMotionEngine.h"
#include "robots/catoms3D/catoms3DRotationEvents.h"
#include "events/events.h"

using namespace Catoms3D;

/// Self-interruption ("tick") identifiers.
static const int TICK_MOVE   = 1; //!< periodic attempt-to-move tick (every module)
static const int TICK_MASTER = 2; //!< periodic goal-completion check tick (master only)

class MillionModuleMarchCode : public Catoms3DBlockCode {
private:
    Catoms3DBlock *module = nullptr;

    // --- Planning state (dynamic programming value function) ---
    int distanceToGoal = INT_MAX;        //!< value function: hops to the goal region
    std::deque<Cell3DPosition> recentCells; //!< recently occupied cells (anti-cycling)

    // --- Role / configuration state ---
    bool isObstacle = false;             //!< fixed terrain: never moves, never joins the field
    bool isMaster = false;               //!< single module that advances the goal box

    // --- Actuation state ---
    bool moving = false;                 //!< true while this module is rotating
    bool haveCellLock = false;           //!< true while we hold the destination cell lock
    Cell3DPosition pendingDest;          //!< destination cell we have claimed
    bID pendingPivotId = 0;              //!< id of the pivot we have reserved (0 == none)

    bool finished = false;               //!< master: no further targets to load

public:
    MillionModuleMarchCode(Catoms3DBlock *host);
    ~MillionModuleMarchCode() {};

    // --- VisibleSim block-code lifecycle ---
    void startup() override;
    void onMotionEnd() override;
    void onInterruptionEvent(std::shared_ptr<Event> event) override;
    void onNeighborChanged(uint64_t face, int action) override;
    void parseUserBlockElements(TiXmlElement *config) override;
    void parseUserElements(TiXmlDocument *config) override;
    void onBlockSelected() override;
    void onEndOfSimulation() override;

    // --- Algorithm helpers ---
    /// True if cell c is inside the current goal region (a self-generated
    /// marching box, or the XML target when one is supplied).
    static bool inGoal(const Cell3DPosition &c);
    /// Current bounding box of the (non-obstacle) robot modules.
    /// @return false if there are no robot modules.
    static bool robotBBox(Cell3DPosition &mn, Cell3DPosition &mx);
    /// Initialise the marching goal: the robot bounding box, one body length
    /// ahead along the march axis.
    static void initMarchGoal();
    /// Advance the goal region one step; returns false when the march is done.
    static bool advanceGoalRegion();
    /// Rebuild the shared value function (BFS distance-to-goal of every relevant
    /// lattice cell) from the current world state. The converged DP result.
    static void computeField();
    /// Refresh this module's own distance-to-goal from the shared field.
    void recomputeDistance();
    /// Estimate the distance-to-goal value of a (free) candidate cell.
    int  estimatedDistanceOfCell(const Cell3DPosition &c) const;
    /// Pick the best beneficial reachable cell; returns true and fills out params.
    bool selectBestMove(Cell3DPosition &bestDest, Catoms3DBlock *&bestPivot) const;
    /// Bounded local connecting-cycle search: false if moving would disconnect.
    bool isMobileByConnectingCycle() const;
    /// Try to perform one locomotion step (called from ticks / events).
    void attemptMove();
    /// (re)schedule the periodic move tick on this module.
    void scheduleMoveTick(Time extraDelay = 0);
    /// Release our claims on the destination cell and the reserved pivot.
    void releaseClaims();
    /// Update this module's display colour from its current state.
    void refreshColor();
    /// Master only: advance the goal box once every robot module is placed.
    void masterCheckGoalCompletion();

    static BlockCode *buildNewBlockCode(BuildingBlock *host) {
        return (new MillionModuleMarchCode((Catoms3DBlock *)host));
    }
};

#endif /* MillionModuleMarchBlockCode_H_ */

#include "gnnLocomotionBlockCode.hpp"
#include "robots/slidingCubes/slidingCubesBlock.h"
#include "grid/lattice.h"
#include "base/world.h"
#include "base/simulator.h"
#include "deps/TinyXML/tinyxml.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <queue>
#include <set>
#include <string>
#include <unordered_set>

using namespace BaseSimulator;

namespace GNNLocomotion {

// Shared across all modules: destinations that have already been claimed this
// step by another module's moveTo() call.  Prevents DoubleInsertionException
// when two modules pick the same destination in the same step.
static std::set<Cell3DPosition> claimedDests;

// Per-step action ballot: every module registers its intended action; when all
// have voted the last voter picks the single winner and schedules the next step
// for everyone at the same simulation time, enforcing global step sync.
struct DeployBallotEntry {
    int    action;
    GNNLocomotionCode* code;
    std::vector<std::pair<Cell3DPosition, uint8_t>> motions;
};
struct DeployBallot {
    uint32_t                         step    = 0;
    std::map<bID, DeployBallotEntry> votes;
    bool                             settled = false;
    void reset(uint32_t s) { step = s; votes.clear(); settled = false; }
};
static DeployBallot g_ballot;

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

GNNLocomotionCode::GNNLocomotionCode(SlidingCubes::SlidingCubesBlock* host)
    : SlidingCubes::SlidingCubesBlockCode(host), module(host) {}

// ---------------------------------------------------------------------------
// startup  — called once at t=0 for every block
// ---------------------------------------------------------------------------

void GNNLocomotionCode::startup() {
    parseDeployConfig();

    if (deployMode) {
        onDeployStartup();
        return;
    }

    // ---- Training (gym) path ----
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
    if (deployMode) {
        motionPending = false;
        claimedDests.erase(pendingDest);
        return;
    }
    GymServer* gs = GymServer::getInstance();
    if (gs) gs->notifyMoveDone();
}

// ---------------------------------------------------------------------------
// onInterruptionEvent
// ---------------------------------------------------------------------------

void GNNLocomotionCode::onInterruptionEvent(std::shared_ptr<Event> event) {
    auto* ie = dynamic_cast<InterruptionEvent<int>*>(event.get());
    if (!ie) return;

    if (deployMode) {
        switch (ie->data) {
            case DEPLOY_STEP_START:     onDeployStepStart(); break;
            case DEPLOY_TIMEOUT_TOPO1:
                if (!topo2Sent) emitTopo2();
                break;
            case DEPLOY_TIMEOUT_TOPO2:
                if (!topo2Advanced) {
                    topo2Advanced = true;
                    // Inline the body of maybeAdvanceFromTopo2 minus the count check
                    bool isAP = localIsArticulationPoint();
                    bool inT  = (target != nullptr && target->isInTarget(module->position));
                    int nMoves = static_cast<int>(module->getAllMotions().size());
                    h_self = buildNodeFeature(
                        BaseSimulator::getWorld()->lattice->gridSize,
                        module->position, inT, nb1mask, nMoves, isAP);
                    emitLayerMessages(0);
                }
                break;
            case DEPLOY_TIMEOUT_LAYER1:
                // Guard: topo2 must have completed (h_self is NODE_DIM).
                // Without this, a stale watchdog from step N can fire in step N+1
                // after clearDeployStepBuffers reset layerAdvanced[0] but before
                // topo2 built h_self, causing an Eigen dimension mismatch.
                if (!layerAdvanced[0] && topo2Advanced) {
                    layerAdvanced[0] = true;
                    h_self = gcnLayerForward(GCNWeights::instance().layers[0],
                                             h_self, incoming[0]);
                    emitLayerMessages(1);
                }
                break;
            case DEPLOY_TIMEOUT_LAYER2:
                // Guard: layer 0 must have completed (h_self is HIDDEN).
                if (!layerAdvanced[1] && layerAdvanced[0]) {
                    layerAdvanced[1] = true;
                    h_self = gcnLayerForward(GCNWeights::instance().layers[1],
                                             h_self, incoming[1]);
                    emitLayerMessages(2);
                }
                break;
            case DEPLOY_TIMEOUT_LAYER3:
                // Guard: layer 1 must have completed (h_self is HIDDEN).
                if (!layerAdvanced[2] && layerAdvanced[1]) {
                    layerAdvanced[2] = true;
                    h_self = gcnLayerForward(GCNWeights::instance().layers[2],
                                             h_self, incoming[2]);
                    runActorAndMove();
                }
                break;
            default: break;
        }
        return;
    }

    if (!isLeader) return;
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

// ===========================================================================
// Deploy mode — distributed inference path
// ===========================================================================

void GNNLocomotionCode::parseDeployConfig() {
    // Env var first (overrides anything in XML)
    if (const char* env = std::getenv("GNN_DEPLOY_WEIGHTS")) {
        deployMode  = true;
        weightsPath = env;
    }

    // Optional <deploy> element under <world>
    TiXmlDocument* xmlDoc =
        BaseSimulator::Simulator::getSimulator()->getConfigDocument();
    if (xmlDoc) {
        TiXmlElement* worldElt = xmlDoc->FirstChildElement("world");
        if (worldElt) {
            TiXmlElement* d = worldElt->FirstChildElement("deploy");
            if (d) {
                const char* en = d->Attribute("enabled");
                if (en && std::string(en) == "true") deployMode = true;
                if (const char* p = d->Attribute("weights")) weightsPath = p;
                unsigned int s = 0;
                if (d->QueryUnsignedAttribute("seed", &s) == TIXML_SUCCESS)
                    globalSeed = s;
            }
        }
    }
}

void GNNLocomotionCode::onDeployStartup() {
    if (weightsPath.empty()) {
        std::cerr << "[GCN deploy] missing weights path; aborting\n";
        deployMode = false;
        return;
    }

    auto& W = GCNWeights::instance();
    if (!W.loaded) {
        try {
            W.loadFromFile(weightsPath);
            std::cout << "[GCN deploy] loaded weights from " << weightsPath << "\n";
        } catch (const std::exception& e) {
            std::cerr << "[GCN deploy] failed to load weights: " << e.what() << "\n";
            deployMode = false;
            return;
        }
    }

    rngState = (static_cast<uint64_t>(globalSeed) << 32)
             ^ static_cast<uint64_t>(module->blockId);
    if (rngState == 0) rngState = 1;

    addMessageEventFunc2(MSG_TOPO_1HOP,
        std::bind(&GNNLocomotionCode::onTopo1Msg, this,
                  std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(MSG_TOPO_2HOP,
        std::bind(&GNNLocomotionCode::onTopo2Msg, this,
                  std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(MSG_GNN_LAYER1,
        std::bind(&GNNLocomotionCode::onGnnLayerMsg, this, 0,
                  std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(MSG_GNN_LAYER2,
        std::bind(&GNNLocomotionCode::onGnnLayerMsg, this, 1,
                  std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(MSG_GNN_LAYER3,
        std::bind(&GNNLocomotionCode::onGnnLayerMsg, this, 2,
                  std::placeholders::_1, std::placeholders::_2));

    // Stagger initial step start by a tiny per-block offset to avoid all blocks
    // emitting on exactly the same scheduler tick.
    Time t0 = scheduler->now() + 1'000 + (module->blockId % 8);
    scheduler->schedule(
        new InterruptionEvent<int>(t0, module, DEPLOY_STEP_START));
}

int GNNLocomotionCode::expectedNeighborCount() const {
    int n = 0;
    for (int d = 0; d < 6; d++) {
        auto* iface = module->getInterface(SCLattice2::Direction(d));
        if (iface && iface->isConnected()) n++;
    }
    return n;
}

void GNNLocomotionCode::clearDeployStepBuffers() {
    nb1mask = 0;
    nb1ids.fill(0);
    topo1Got.fill(false);
    topo2Got.fill(false);
    topo1Received = 0;
    topo2Received = 0;
    topo2Sent     = false;
    topo2Advanced = false;
    for (int L = 0; L < N_LAYERS; L++) {
        incoming[L].clear();
        layerMsgReceived[L] = 0;
        layerAdvanced[L]    = false;
    }
}

void GNNLocomotionCode::onDeployStepStart() {
    deployStep++;
    clearDeployStepBuffers();

    // Compute local 1-hop neighbour mask and IDs.
    for (int d = 0; d < 6; d++) {
        auto* iface = module->getInterface(SCLattice2::Direction(d));
        if (iface && iface->isConnected()) {
            nb1mask |= (1u << d);
            nb1ids[d] = iface->getConnectedBlockBId();
        }
    }

    // Phase A round 1: broadcast our 1-hop mask + id.
    TopoPayload pl{deployStep, module->blockId, nb1mask};
    sendMessageToAllNeighbors(
        new MessageOf<TopoPayload>(MSG_TOPO_1HOP, pl),
        ROUND_DT_US, 0, 0);

    // Watchdog: force topo2 emission after 2 rounds even if some neighbours
    // are silent (e.g. mid-motion).
    scheduler->schedule(new InterruptionEvent<int>(
        scheduler->now() + 3 * ROUND_DT_US, module, DEPLOY_TIMEOUT_TOPO1));

    // Schedule a phase-1 advance after one round if we're isolated.
    if (expectedNeighborCount() == 0) {
        // No neighbours: skip directly to GNN forward (degenerate case).
        h_self = buildNodeFeature(
            BaseSimulator::getWorld()->lattice->gridSize,
            module->position,
            (target != nullptr && target->isInTarget(module->position)),
            0, /* nMoves */ static_cast<int>(module->getAllMotions().size()),
            /* isAP */ false);
        // Three layers with no incoming messages
        for (int L = 0; L < N_LAYERS; L++)
            h_self = gcnLayerForward(GCNWeights::instance().layers[L], h_self, {});
        scheduler->schedule(new InterruptionEvent<int>(
            scheduler->now() + 4 * ROUND_DT_US, module, DEPLOY_STEP_START));
        runActorAndMove();
    }
}

void GNNLocomotionCode::onTopo1Msg(std::shared_ptr<Message> m,
                                   P2PNetworkInterface* sender) {
    if (!deployMode) return;
    auto* msg = static_cast<MessageOf<TopoPayload>*>(m.get());
    const TopoPayload& pl = *msg->getData();
    if (pl.step != deployStep) return;  // stale

    int d = module->getDirection(sender);
    if (d < 0 || d >= 6) return;
    topo1Buf[d] = pl;
    if (!topo1Got[d]) {
        topo1Got[d] = true;
        topo1Received++;
    }
    maybeAdvanceFromTopo1();
}

void GNNLocomotionCode::maybeAdvanceFromTopo1() {
    if (topo2Sent) return;
    if (topo1Received < expectedNeighborCount()) return;
    emitTopo2();
}

void GNNLocomotionCode::emitTopo2() {
    if (topo2Sent) return;
    topo2Sent = true;

    Topo2Payload pl{};
    pl.step  = deployStep;
    pl.id    = module->blockId;
    pl.mask6 = nb1mask;
    for (int d = 0; d < 6; d++) pl.nbIds[d] = nb1ids[d];

    sendMessageToAllNeighbors(
        new MessageOf<Topo2Payload>(MSG_TOPO_2HOP, pl),
        ROUND_DT_US, 0, 0);

    // Watchdog: force topo2 advance after 2 rounds.
    scheduler->schedule(new InterruptionEvent<int>(
        scheduler->now() + 3 * ROUND_DT_US, module, DEPLOY_TIMEOUT_TOPO2));
}

void GNNLocomotionCode::onTopo2Msg(std::shared_ptr<Message> m,
                                   P2PNetworkInterface* sender) {
    if (!deployMode) return;
    auto* msg = static_cast<MessageOf<Topo2Payload>*>(m.get());
    const Topo2Payload& pl = *msg->getData();
    if (pl.step != deployStep) return;

    int d = module->getDirection(sender);
    if (d < 0 || d >= 6) return;
    topo2Buf[d] = pl;
    if (!topo2Got[d]) {
        topo2Got[d] = true;
        topo2Received++;
    }
    maybeAdvanceFromTopo2();
}

void GNNLocomotionCode::maybeAdvanceFromTopo2() {
    if (topo2Advanced) return;
    // From the message handler we wait for all expected messages; the watchdog
    // fires this method through DEPLOY_TIMEOUT_TOPO2 with the full-count check
    // shortcircuited because by then `topo2Sent` was true and the watchdog
    // forces progress regardless.
    if (topo2Received < expectedNeighborCount()) return;
    topo2Advanced = true;

    // Compute local AP from 2-hop subgraph
    bool isAP = localIsArticulationPoint();

    // Build initial node feature h^0.
    bool inT = (target != nullptr && target->isInTarget(module->position));
    int nMoves = static_cast<int>(module->getAllMotions().size());
    h_self = buildNodeFeature(
        BaseSimulator::getWorld()->lattice->gridSize,
        module->position,
        inT, nb1mask, nMoves, isAP);

    // Emit layer-1 messages: each neighbour d receives msg_lin([h^0 ‖ e_{j→i}]).
    // Convention: "fromDir" stored in payload is the direction that the receiver
    //             sees this sender on, i.e. opposite(d).
    emitLayerMessages(0);
}

bool GNNLocomotionCode::localIsArticulationPoint() const {
    // Build the induced subgraph on { self ∪ direct neighbours ∪ 2-hop nodes }.
    // If self has ≤1 neighbour, it cannot be a cut vertex.
    int nNb = expectedNeighborCount();
    if (nNb <= 1) return false;

    // `nodes` contains every block id we know about within 2 hops.
    std::unordered_set<bID> nodes;
    nodes.insert(module->blockId);
    for (int d = 0; d < 6; d++)
        if (nb1mask & (1u << d)) nodes.insert(nb1ids[d]);
    for (int d = 0; d < 6; d++) {
        if (!(nb1mask & (1u << d))) continue;
        const Topo2Payload& pl = topo2Buf[d];
        for (int dd = 0; dd < 6; dd++)
            if (pl.mask6 & (1u << dd)) nodes.insert(pl.nbIds[dd]);
    }

    // Adjacency map (undirected) over nodes.
    std::map<bID, std::unordered_set<bID>> adj;
    // Self's edges to its direct neighbours.
    for (int d = 0; d < 6; d++) {
        if (!(nb1mask & (1u << d))) continue;
        adj[module->blockId].insert(nb1ids[d]);
        adj[nb1ids[d]].insert(module->blockId);
    }
    // Each direct neighbour's edges to anything we know about.
    for (int d = 0; d < 6; d++) {
        if (!(nb1mask & (1u << d))) continue;
        const Topo2Payload& pl = topo2Buf[d];
        for (int dd = 0; dd < 6; dd++) {
            if (!(pl.mask6 & (1u << dd))) continue;
            bID other = pl.nbIds[dd];
            if (nodes.count(other)) {
                adj[pl.id].insert(other);
                adj[other].insert(pl.id);
            }
        }
    }

    // BFS over (nodes \ {self}) starting from a direct neighbour. If we can't
    // reach every OTHER direct neighbour through this restricted graph, self is
    // a cut vertex.
    bID start = 0;
    bool found = false;
    for (int d = 0; d < 6; d++) {
        if (nb1mask & (1u << d)) { start = nb1ids[d]; found = true; break; }
    }
    if (!found) return false;

    std::unordered_set<bID> visited;
    std::queue<bID> q;
    visited.insert(start);
    q.push(start);
    while (!q.empty()) {
        bID cur = q.front(); q.pop();
        for (bID nb : adj[cur]) {
            if (nb == module->blockId) continue;     // remove self
            if (visited.insert(nb).second) q.push(nb);
        }
    }

    // Self is AP iff at least one direct neighbour is unreachable from `start`.
    for (int d = 0; d < 6; d++) {
        if (!(nb1mask & (1u << d))) continue;
        if (!visited.count(nb1ids[d])) return true;
    }
    return false;
}

void GNNLocomotionCode::emitLayerMessages(int layer) {
    const auto& W = GCNWeights::instance();
    for (int d = 0; d < 6; d++) {
        if (!(nb1mask & (1u << d))) continue;
        auto* iface = module->getInterface(SCLattice2::Direction(d));
        if (!iface || !iface->isConnected()) continue;

        // Edge feature direction: from sender to receiver perspective.
        // Receiver sees this message arriving from the opposite direction `d^1`
        // (PlusX↔MinusX, PlusY↔MinusY, PlusZ↔MinusZ each XOR with 1 in the
        // SCLattice2::Direction enum).
        Eigen::VectorXf edgeFeat = edgeFeature(d);
        Eigen::VectorXf m = gcnEmitMsg(W.layers[layer], h_self, edgeFeat);

        GNNMsgPayload pl;
        pl.step    = deployStep;
        pl.fromDir = static_cast<uint8_t>(d ^ 1);   // receiver-side direction
        for (int k = 0; k < HIDDEN; k++) pl.h[k] = m(k);

        int msgType = (layer == 0) ? MSG_GNN_LAYER1
                    : (layer == 1) ? MSG_GNN_LAYER2 : MSG_GNN_LAYER3;
        sendMessage(new MessageOf<GNNMsgPayload>(msgType, pl),
                    iface, ROUND_DT_US, 0);
    }

    // Watchdog for this layer — advance even if messages don't arrive.
    int timeoutId = (layer == 0) ? DEPLOY_TIMEOUT_LAYER1
                  : (layer == 1) ? DEPLOY_TIMEOUT_LAYER2
                                 : DEPLOY_TIMEOUT_LAYER3;
    scheduler->schedule(new InterruptionEvent<int>(
        scheduler->now() + 3 * ROUND_DT_US, module, timeoutId));

    // Isolated node case: no messages will ever arrive, advance immediately.
    if (expectedNeighborCount() == 0) maybeAdvanceLayer(layer);
}

void GNNLocomotionCode::onGnnLayerMsg(int layer,
                                      std::shared_ptr<Message> m,
                                      P2PNetworkInterface* sender) {
    if (!deployMode) return;
    auto* msg = static_cast<MessageOf<GNNMsgPayload>*>(m.get());
    const GNNMsgPayload& pl = *msg->getData();
    if (pl.step != deployStep) return;
    (void)sender;  // direction comes from payload

    Eigen::VectorXf v(HIDDEN);
    for (int k = 0; k < HIDDEN; k++) v(k) = pl.h[k];
    incoming[layer].push_back(std::move(v));
    layerMsgReceived[layer]++;
    maybeAdvanceLayer(layer);
}

void GNNLocomotionCode::maybeAdvanceLayer(int layer) {
    if (layerAdvanced[layer]) return;
    if (layerMsgReceived[layer] < expectedNeighborCount()) return;
    layerAdvanced[layer] = true;

    // Finalise this layer
    const auto& W = GCNWeights::instance();
    h_self = gcnLayerForward(W.layers[layer], h_self, incoming[layer]);

    if (layer + 1 < N_LAYERS) {
        emitLayerMessages(layer + 1);
    } else {
        runActorAndMove();
    }
}

void GNNLocomotionCode::runActorAndMove() {
    const auto& W = GCNWeights::instance();
    Eigen::VectorXf logits = actorLogits(W, h_self);

    auto motions = module->getAllMotions();
    int  nMoves  = std::min<int>(static_cast<int>(motions.size()), N_ACTIONS - 1);
    bool inT     = (target != nullptr && target->isInTarget(module->position));
    bool isAP    = localIsArticulationPoint();

    std::array<bool, N_ACTIONS> mask{};
    mask[0] = true;
    if (!inT && !isAP)
        for (int i = 1; i <= nMoves; i++) mask[i] = true;

    int action = argmaxMasked(logits, mask);

    // Anti-oscillation: suppress any action that would revisit a recently
    // occupied position.  Without this, a block caught in an N-step policy
    // cycle keeps winning the ballot and prevents others from moving.
    if (posHistoryLen > 0 && action != 0 && action - 1 < static_cast<int>(motions.size())) {
        const Cell3DPosition dest = motions[action - 1].first;
        for (int h = 0; h < posHistoryLen; h++) {
            if (dest == posHistory[h]) { action = 0; break; }
        }
    }

    std::cout << "[GCN id=" << module->blockId
              << " step=" << deployStep
              << " action=" << action
              << " inTarget=" << (int)inT
              << " isAP=" << (int)isAP << "]\n";

    // Register this block's vote in the shared ballot.
    size_t totalBlocks = BaseSimulator::getWorld()->getMap().size();
    if (g_ballot.step != deployStep) g_ballot.reset(deployStep);
    g_ballot.votes[module->blockId] = {action, this, std::move(motions)};

    // Wait until every block has voted before acting (mirrors training: one
    // block moves per step, chosen as the lowest ID with a non-stay action).
    if (g_ballot.votes.size() < totalBlocks || g_ballot.settled) return;
    g_ballot.settled = true;

    // Pick winner: lowest block ID with a non-stay action.
    bID               winner  = 0;
    GNNLocomotionCode* winCode = nullptr;
    for (auto& [id, e] : g_ballot.votes) {  // map iterates in ascending ID order
        if (e.action != 0) { winner = id; winCode = e.code; break; }
    }

    // Deadlock escape: if every block voted stay (all histories exhausted),
    // clear all histories so the policy can explore fresh from the current state.
    if (winner == 0) {
        for (auto& [id, e] : g_ballot.votes)
            e.code->posHistoryLen = 0;
    }

    // Execute the winner's move.  Only one moveTo is issued per step.
    Time motionDuration = 0;
    if (winCode && !winCode->motionPending) {
        auto& e   = g_ballot.votes[winner];
        int   act = e.action;
        if (act - 1 < static_cast<int>(e.motions.size())) {
            const Cell3DPosition dest = e.motions[act - 1].first;
            if (winCode->module->moveTo(dest)) {
                // Push current position into the winner's history (FIFO, newest at [0]).
                for (int h = std::min(winCode->posHistoryLen, HIST_LEN - 1); h > 0; h--)
                    winCode->posHistory[h] = winCode->posHistory[h - 1];
                winCode->posHistory[0] = winCode->module->position;
                winCode->posHistoryLen = std::min(winCode->posHistoryLen + 1, HIST_LEN);
                winCode->motionPending = true;
                winCode->pendingDest     = dest;
                claimedDests.insert(dest);
                // TeleportationStartEvent fires at now+1000000 µs; wait until
                // motion completes before beginning the next step.
                motionDuration = 1'100'000;
            } else {
                std::cout << "  [GCN winner id=" << winner
                          << "] moveTo failed (occupied)\n";
            }
        }
    }

    // Schedule the next step for ALL blocks at the same simulation time so
    // every module begins step N+1 in sync.
    Time nextStep = scheduler->now()
                  + std::max(static_cast<Time>(motionDuration), 8 * ROUND_DT_US);
    for (auto& [id, e] : g_ballot.votes) {
        scheduler->schedule(new InterruptionEvent<int>(
            nextStep, e.code->module, DEPLOY_STEP_START));
    }
}

} // namespace GNNLocomotion

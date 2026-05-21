#include "gnnShapeReconfigurationBlockCode.hpp"
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

namespace GNNShapeReconfiguration {

static std::set<Cell3DPosition> claimedDests;

struct DeployBallotEntry {
    int    action;
    GNNShapeReconfigurationCode* code;
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

GNNShapeReconfigurationCode::GNNShapeReconfigurationCode(SlidingCubes::SlidingCubesBlock* host)
    : SlidingCubes::SlidingCubesBlockCode(host), module(host) {}

// ---------------------------------------------------------------------------
// startup
// ---------------------------------------------------------------------------

void GNNShapeReconfigurationCode::startup() {
    parseDeployConfig();

    if (deployMode) {
        onDeployStartup();
        return;
    }

    bID minId = module->blockId;
    for (auto& [id, _] : BaseSimulator::getWorld()->getMap())
        if (id < minId) minId = id;
    isLeader = (module->blockId == minId);

    if (!isLeader) return;

    std::cout << "[GNN] Leader block " << module->blockId
              << " — starting gym server on port " << GYM_PORT << "\n";

    GymServer* gs = GymServer::getInstance();

    gs->setMoveCompleteCallback([this] {
        scheduler->schedule(
            new InterruptionEvent<int>(scheduler->now(), module, MOVE_DONE));
    });

    gs->start(GYM_PORT);
    gs->setObs(buildObs());

    scheduleGymTick(TICK_IDLE_US);
}

// ---------------------------------------------------------------------------
// onMotionEnd
// ---------------------------------------------------------------------------

void GNNShapeReconfigurationCode::onMotionEnd() {
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

void GNNShapeReconfigurationCode::onInterruptionEvent(std::shared_ptr<Event> event) {
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
                if (!layerAdvanced[0] && topo2Advanced) {
                    layerAdvanced[0] = true;
                    h_self = gcnLayerForward(GCNWeights::instance().layers[0],
                                             h_self, incoming[0]);
                    emitLayerMessages(1);
                }
                break;
            case DEPLOY_TIMEOUT_LAYER2:
                if (!layerAdvanced[1] && layerAdvanced[0]) {
                    layerAdvanced[1] = true;
                    h_self = gcnLayerForward(GCNWeights::instance().layers[1],
                                             h_self, incoming[1]);
                    emitLayerMessages(2);
                }
                break;
            case DEPLOY_TIMEOUT_LAYER3:
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

void GNNShapeReconfigurationCode::scheduleGymTick(Time delayUs) {
    scheduler->schedule(
        new InterruptionEvent<int>(scheduler->now() + delayUs, module, GYM_TICK));
}

// ---------------------------------------------------------------------------
// Gym step logic
// ---------------------------------------------------------------------------

void GNNShapeReconfigurationCode::onGymTick() {
    if (episodeDone) return;

    if (!GymServer::getInstance()->hasStep()) {
        scheduleGymTick(TICK_IDLE_US);
        return;
    }

    auto actions = GymServer::getInstance()->getAndClearStep();
    prevInTarget = countBlocksInTarget();
    executeActions(std::move(actions));
}

void GNNShapeReconfigurationCode::executeActions(std::map<bID, int> actions) {
    while (!moveQueue.empty()) moveQueue.pop();

    auto& worldMap = BaseSimulator::getWorld()->getMap();

    std::vector<bID> ids;
    ids.reserve(actions.size());
    for (auto& [id, _] : actions) ids.push_back(id);
    std::sort(ids.begin(), ids.end());

    for (bID blockId : ids) {
        int moveIdx = actions[blockId];
        if (moveIdx == 0) continue;

        auto it = worldMap.find(blockId);
        if (it == worldMap.end()) continue;

        auto* sc = dynamic_cast<SlidingCubes::SlidingCubesBlock*>(it->second);
        if (!sc) continue;

        auto moves = sc->getAllMotions();
        int idx = moveIdx - 1;
        if (idx >= 0 && idx < static_cast<int>(moves.size()))
            moveQueue.push({sc, moves[idx].first});
    }

    executeNextMove();
}

bool GNNShapeReconfigurationCode::isArticulationPoint(const SlidingCubes::SlidingCubesBlock* sc) const {
    auto& worldMap = BaseSimulator::getWorld()->getMap();
    if (worldMap.size() <= 2) return false;

    bID removedId = sc->blockId;

    std::unordered_set<bID> remaining;
    for (auto& [id, _] : worldMap)
        if (id != removedId) remaining.insert(id);

    if (remaining.empty()) return false;

    auto* lattice = BaseSimulator::getWorld()->lattice;
    (void)lattice;
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

void GNNShapeReconfigurationCode::executeNextMove() {
    if (episodeDone) return;

    while (!moveQueue.empty()) {
        auto [sc, dest] = moveQueue.front();
        moveQueue.pop();

        auto& worldMap = BaseSimulator::getWorld()->getMap();
        if (!worldMap.count(sc->blockId)) continue;

        if (isArticulationPoint(sc)) {
            while (!moveQueue.empty()) moveQueue.pop();
            pendingPenalty = -10.0;
            episodeDone    = true;
            break;
        }

        if (sc->moveTo(dest)) {
            while (!moveQueue.empty()) moveQueue.pop();
            return;
        }
    }

    onAllMovesComplete();
}

void GNNShapeReconfigurationCode::onAllMovesComplete() {
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
}

// ---------------------------------------------------------------------------
// Observation builder
// ---------------------------------------------------------------------------

GymObs GNNShapeReconfigurationCode::buildObs() {
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
// Target cell cache
// ---------------------------------------------------------------------------

std::vector<Cell3DPosition>& GNNShapeReconfigurationCode::getTargetCells() {
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

int GNNShapeReconfigurationCode::countBlocksInTarget() const {
    if (!target) return 0;
    int count = 0;
    for (auto& [id, bb] : BaseSimulator::getWorld()->getMap())
        if (target->isInTarget(bb->position)) count++;
    return count;
}

double GNNShapeReconfigurationCode::computeReward(int prevIn, int newIn, bool done) const {
    double r = static_cast<double>(newIn - prevIn);
    r -= 0.01;
    if (done && newIn >= static_cast<int>(targetCells.size()))
        r += 10.0;
    return r;
}

// ===========================================================================
// Deploy mode
// ===========================================================================

void GNNShapeReconfigurationCode::parseDeployConfig() {
    if (const char* env = std::getenv("GNN_DEPLOY_WEIGHTS")) {
        deployMode  = true;
        weightsPath = env;
    }

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

void GNNShapeReconfigurationCode::onDeployStartup() {
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
        std::bind(&GNNShapeReconfigurationCode::onTopo1Msg, this,
                  std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(MSG_TOPO_2HOP,
        std::bind(&GNNShapeReconfigurationCode::onTopo2Msg, this,
                  std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(MSG_GNN_LAYER1,
        std::bind(&GNNShapeReconfigurationCode::onGnnLayerMsg, this, 0,
                  std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(MSG_GNN_LAYER2,
        std::bind(&GNNShapeReconfigurationCode::onGnnLayerMsg, this, 1,
                  std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(MSG_GNN_LAYER3,
        std::bind(&GNNShapeReconfigurationCode::onGnnLayerMsg, this, 2,
                  std::placeholders::_1, std::placeholders::_2));

    Time t0 = scheduler->now() + 1'000 + (module->blockId % 8);
    scheduler->schedule(
        new InterruptionEvent<int>(t0, module, DEPLOY_STEP_START));
}

int GNNShapeReconfigurationCode::expectedNeighborCount() const {
    int n = 0;
    for (int d = 0; d < 6; d++) {
        auto* iface = module->getInterface(SCLattice2::Direction(d));
        if (iface && iface->isConnected()) n++;
    }
    return n;
}

void GNNShapeReconfigurationCode::clearDeployStepBuffers() {
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

void GNNShapeReconfigurationCode::onDeployStepStart() {
    deployStep++;
    clearDeployStepBuffers();

    for (int d = 0; d < 6; d++) {
        auto* iface = module->getInterface(SCLattice2::Direction(d));
        if (iface && iface->isConnected()) {
            nb1mask |= (1u << d);
            nb1ids[d] = iface->getConnectedBlockBId();
        }
    }

    TopoPayload pl{deployStep, module->blockId, nb1mask};
    sendMessageToAllNeighbors(
        new MessageOf<TopoPayload>(MSG_TOPO_1HOP, pl),
        ROUND_DT_US, 0, 0);

    scheduler->schedule(new InterruptionEvent<int>(
        scheduler->now() + 3 * ROUND_DT_US, module, DEPLOY_TIMEOUT_TOPO1));

    if (expectedNeighborCount() == 0) {
        bool inT = (target != nullptr && target->isInTarget(module->position));
        h_self = buildNodeFeature(
            BaseSimulator::getWorld()->lattice->gridSize,
            module->position, inT, 0,
            static_cast<int>(module->getAllMotions().size()), false);
        for (int L = 0; L < N_LAYERS; L++)
            h_self = gcnLayerForward(GCNWeights::instance().layers[L], h_self, {});
        scheduler->schedule(new InterruptionEvent<int>(
            scheduler->now() + 4 * ROUND_DT_US, module, DEPLOY_STEP_START));
        runActorAndMove();
    }
}

void GNNShapeReconfigurationCode::onTopo1Msg(std::shared_ptr<Message> m,
                                              P2PNetworkInterface* sender) {
    if (!deployMode) return;
    auto* msg = static_cast<MessageOf<TopoPayload>*>(m.get());
    const TopoPayload& pl = *msg->getData();
    if (pl.step != deployStep) return;

    int d = module->getDirection(sender);
    if (d < 0 || d >= 6) return;
    topo1Buf[d] = pl;
    if (!topo1Got[d]) { topo1Got[d] = true; topo1Received++; }
    maybeAdvanceFromTopo1();
}

void GNNShapeReconfigurationCode::maybeAdvanceFromTopo1() {
    if (topo2Sent) return;
    if (topo1Received < expectedNeighborCount()) return;
    emitTopo2();
}

void GNNShapeReconfigurationCode::emitTopo2() {
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

    scheduler->schedule(new InterruptionEvent<int>(
        scheduler->now() + 3 * ROUND_DT_US, module, DEPLOY_TIMEOUT_TOPO2));
}

void GNNShapeReconfigurationCode::onTopo2Msg(std::shared_ptr<Message> m,
                                              P2PNetworkInterface* sender) {
    if (!deployMode) return;
    auto* msg = static_cast<MessageOf<Topo2Payload>*>(m.get());
    const Topo2Payload& pl = *msg->getData();
    if (pl.step != deployStep) return;

    int d = module->getDirection(sender);
    if (d < 0 || d >= 6) return;
    topo2Buf[d] = pl;
    if (!topo2Got[d]) { topo2Got[d] = true; topo2Received++; }
    maybeAdvanceFromTopo2();
}

void GNNShapeReconfigurationCode::maybeAdvanceFromTopo2() {
    if (topo2Advanced) return;
    if (topo2Received < expectedNeighborCount()) return;
    topo2Advanced = true;

    bool isAP = localIsArticulationPoint();
    bool inT  = (target != nullptr && target->isInTarget(module->position));
    int nMoves = static_cast<int>(module->getAllMotions().size());
    h_self = buildNodeFeature(
        BaseSimulator::getWorld()->lattice->gridSize,
        module->position, inT, nb1mask, nMoves, isAP);

    emitLayerMessages(0);
}

bool GNNShapeReconfigurationCode::localIsArticulationPoint() const {
    int nNb = expectedNeighborCount();
    if (nNb <= 1) return false;

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

    std::map<bID, std::unordered_set<bID>> adj;
    for (int d = 0; d < 6; d++) {
        if (!(nb1mask & (1u << d))) continue;
        adj[module->blockId].insert(nb1ids[d]);
        adj[nb1ids[d]].insert(module->blockId);
    }
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
            if (nb == module->blockId) continue;
            if (visited.insert(nb).second) q.push(nb);
        }
    }

    for (int d = 0; d < 6; d++) {
        if (!(nb1mask & (1u << d))) continue;
        if (!visited.count(nb1ids[d])) return true;
    }
    return false;
}

void GNNShapeReconfigurationCode::emitLayerMessages(int layer) {
    const auto& W = GCNWeights::instance();
    for (int d = 0; d < 6; d++) {
        if (!(nb1mask & (1u << d))) continue;
        auto* iface = module->getInterface(SCLattice2::Direction(d));
        if (!iface || !iface->isConnected()) continue;

        Eigen::VectorXf edgeFeat = edgeFeature(d);
        Eigen::VectorXf m = gcnEmitMsg(W.layers[layer], h_self, edgeFeat);

        GNNMsgPayload pl;
        pl.step    = deployStep;
        pl.fromDir = static_cast<uint8_t>(d ^ 1);
        for (int k = 0; k < HIDDEN; k++) pl.h[k] = m(k);

        int msgType = (layer == 0) ? MSG_GNN_LAYER1
                    : (layer == 1) ? MSG_GNN_LAYER2 : MSG_GNN_LAYER3;
        sendMessage(new MessageOf<GNNMsgPayload>(msgType, pl),
                    iface, ROUND_DT_US, 0);
    }

    int timeoutId = (layer == 0) ? DEPLOY_TIMEOUT_LAYER1
                  : (layer == 1) ? DEPLOY_TIMEOUT_LAYER2
                                 : DEPLOY_TIMEOUT_LAYER3;
    scheduler->schedule(new InterruptionEvent<int>(
        scheduler->now() + 3 * ROUND_DT_US, module, timeoutId));

    if (expectedNeighborCount() == 0) maybeAdvanceLayer(layer);
}

void GNNShapeReconfigurationCode::onGnnLayerMsg(int layer,
                                                 std::shared_ptr<Message> m,
                                                 P2PNetworkInterface* sender) {
    if (!deployMode) return;
    auto* msg = static_cast<MessageOf<GNNMsgPayload>*>(m.get());
    const GNNMsgPayload& pl = *msg->getData();
    if (pl.step != deployStep) return;
    (void)sender;

    Eigen::VectorXf v(HIDDEN);
    for (int k = 0; k < HIDDEN; k++) v(k) = pl.h[k];
    incoming[layer].push_back(std::move(v));
    layerMsgReceived[layer]++;
    maybeAdvanceLayer(layer);
}

void GNNShapeReconfigurationCode::maybeAdvanceLayer(int layer) {
    if (layerAdvanced[layer]) return;
    if (layerMsgReceived[layer] < expectedNeighborCount()) return;
    layerAdvanced[layer] = true;

    const auto& W = GCNWeights::instance();
    h_self = gcnLayerForward(W.layers[layer], h_self, incoming[layer]);

    if (layer + 1 < N_LAYERS) {
        emitLayerMessages(layer + 1);
    } else {
        runActorAndMove();
    }
}

void GNNShapeReconfigurationCode::runActorAndMove() {
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

    size_t totalBlocks = BaseSimulator::getWorld()->getMap().size();
    if (g_ballot.step != deployStep) g_ballot.reset(deployStep);
    g_ballot.votes[module->blockId] = {action, this, std::move(motions)};

    if (g_ballot.votes.size() < totalBlocks || g_ballot.settled) return;
    g_ballot.settled = true;

    bID               winner  = 0;
    GNNShapeReconfigurationCode* winCode = nullptr;
    for (auto& [id, e] : g_ballot.votes) {
        if (e.action != 0) { winner = id; winCode = e.code; break; }
    }

    if (winner == 0) {
        for (auto& [id, e] : g_ballot.votes)
            e.code->posHistoryLen = 0;
    }

    Time motionDuration = 0;
    if (winCode && !winCode->motionPending) {
        auto& e   = g_ballot.votes[winner];
        int   act = e.action;
        if (act - 1 < static_cast<int>(e.motions.size())) {
            const Cell3DPosition dest = e.motions[act - 1].first;
            if (winCode->module->moveTo(dest)) {
                for (int h = std::min(winCode->posHistoryLen, HIST_LEN - 1); h > 0; h--)
                    winCode->posHistory[h] = winCode->posHistory[h - 1];
                winCode->posHistory[0] = winCode->module->position;
                winCode->posHistoryLen = std::min(winCode->posHistoryLen + 1, HIST_LEN);
                winCode->motionPending = true;
                winCode->pendingDest   = dest;
                claimedDests.insert(dest);
                motionDuration = 1'100'000;
            } else {
                std::cout << "  [GCN winner id=" << winner
                          << "] moveTo failed (occupied)\n";
            }
        }
    }

    int inTarget    = countBlocksInTarget();
    int totalTarget = static_cast<int>(getTargetCells().size());
    bool shapeDone  = (totalTarget > 0 && inTarget >= totalTarget);
    bool timeDone   = (deployStep >= MAX_STEPS);

    if (shapeDone || timeDone) {
        std::cout << "[GCN deploy] "
                  << (shapeDone ? "target shape reached" : "step limit reached")
                  << " at step " << deployStep
                  << " (" << inTarget << "/" << totalTarget << " in target)\n";
        return;
    }

    Time nextStep = scheduler->now()
                  + std::max(static_cast<Time>(motionDuration), 8 * ROUND_DT_US);
    for (auto& [id, e] : g_ballot.votes) {
        scheduler->schedule(new InterruptionEvent<int>(
            nextStep, e.code->module, DEPLOY_STEP_START));
    }
}

} // namespace GNNShapeReconfiguration

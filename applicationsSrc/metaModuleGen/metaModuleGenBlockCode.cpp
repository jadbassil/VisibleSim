#include "metaModuleGenBlockCode.h"

#include <algorithm>
#include <cstdio>
#include <iostream>

#include "base/world.h"
#include "events/scheduler.h"
#include "grid/lattice.h"
#include "motion/teleportationEvents.h"
#include "robots/catoms3D/catoms3DRotationEvents.h"
#include "robots/catoms3D/catoms3DMotionEngine.h"

using namespace Catoms3D;
using metaModuleGen::CellSet;
using metaModuleGen::pathLength;

// --- static storage ------------------------------------------------------
bool MetaModuleGenBlockCode::infoParsed = false;
Cell3DPosition MetaModuleGenBlockCode::bX, MetaModuleGenBlockCode::bY, MetaModuleGenBlockCode::bZ;
Cell3DPosition MetaModuleGenBlockCode::center0;
std::vector<Cell3DPosition> MetaModuleGenBlockCode::cells0;
int MetaModuleGenBlockCode::nA = 1, MetaModuleGenBlockCode::nB = 1, MetaModuleGenBlockCode::nC = 1;
int MetaModuleGenBlockCode::opIdx = 0;
int MetaModuleGenBlockCode::dirIdx = 0;
Cell3DPosition MetaModuleGenBlockCode::selMeta;
bool MetaModuleGenBlockCode::hasSelection = false;
bool MetaModuleGenBlockCode::useRotations = true;
bool MetaModuleGenBlockCode::animating = false;
std::deque<std::pair<bID, Cell3DPosition>> MetaModuleGenBlockCode::moveQueue;
bID MetaModuleGenBlockCode::journeyId = 0;
CellSet MetaModuleGenBlockCode::journeyVisited;
int MetaModuleGenBlockCode::journeyHops = 0;

namespace {
Cell3DPosition add(const Cell3DPosition& a, const Cell3DPosition& b) {
    return Cell3DPosition(a[0]+b[0], a[1]+b[1], a[2]+b[2]);
}
bool parseVec(const char* s, Cell3DPosition& out) {
    if (!s) return false;
    int x, y, z;
    if (sscanf(s, "%d,%d,%d", &x, &y, &z) != 3) return false;
    out = Cell3DPosition((short)x, (short)y, (short)z);
    return true;
}
} // namespace

const char* MetaModuleGenBlockCode::opName(int i) {
    static const char* n[3] = {"Dismantle", "Transfer", "Assemble"};
    return (i >= 0 && i < 3) ? n[i] : "?";
}
const char* MetaModuleGenBlockCode::dirName(int i) {
    static const char* n[6] = {"+X", "-X", "+Y", "-Y", "+Z", "-Z"};
    return (i >= 0 && i < 6) ? n[i] : "?";
}

Cell3DPosition MetaModuleGenBlockCode::latticeShift(int a, int b, int c) {
    return Cell3DPosition(a*bX[0]+b*bY[0]+c*bZ[0],
                          a*bX[1]+b*bY[1]+c*bZ[1],
                          a*bX[2]+b*bY[2]+c*bZ[2]);
}
std::vector<Cell3DPosition> MetaModuleGenBlockCode::cellsOf(const Cell3DPosition& meta) {
    Cell3DPosition s = latticeShift(meta[0], meta[1], meta[2]);
    std::vector<Cell3DPosition> out;
    out.reserve(cells0.size());
    for (const auto& c : cells0) out.push_back(add(c, s));
    return out;
}
Cell3DPosition MetaModuleGenBlockCode::centerOf(const Cell3DPosition& meta) {
    return add(center0, latticeShift(meta[0], meta[1], meta[2]));
}
Cell3DPosition MetaModuleGenBlockCode::unitStep(int dir) {
    switch (dir) {
        case 0: return Cell3DPosition( 1, 0, 0);
        case 1: return Cell3DPosition(-1, 0, 0);
        case 2: return Cell3DPosition( 0, 1, 0);
        case 3: return Cell3DPosition( 0,-1, 0);
        case 4: return Cell3DPosition( 0, 0, 1);
        default:return Cell3DPosition( 0, 0,-1);
    }
}
bool MetaModuleGenBlockCode::metaOfCell(const Cell3DPosition& p, Cell3DPosition& metaOut) {
    for (int a = -2; a <= nA + 1; ++a)
        for (int b = -2; b <= nB + 1; ++b)
            for (int c = -2; c <= nC + 1; ++c) {
                Cell3DPosition s = latticeShift(a, b, c);
                for (const auto& cell : cells0)
                    if (add(cell, s) == p) {
                        metaOut = Cell3DPosition((short)a, (short)b, (short)c);
                        return true;
                    }
            }
    return false;
}
CellSet MetaModuleGenBlockCode::worldOccupancy() {
    CellSet occ;
    for (const auto& kv : BaseSimulator::getWorld()->getMap())
        occ.insert(kv.second->position);
    return occ;
}

void MetaModuleGenBlockCode::parseUserElements(TiXmlDocument* config) {
    if (infoParsed || !config) return;
    TiXmlElement* root = config->RootElement();
    if (!root) return;
    TiXmlElement* info = root->FirstChildElement("metaModuleInfo");
    if (!info) {
        std::cout << "[MetaModuleViz] no <metaModuleInfo> in config — "
                     "interaction disabled.\n";
        return;
    }
    parseVec(info->Attribute("X"), bX);
    parseVec(info->Attribute("Y"), bY);
    parseVec(info->Attribute("Z"), bZ);
    parseVec(info->Attribute("center"), center0);
    info->Attribute("nA", &nA);
    info->Attribute("nB", &nB);
    info->Attribute("nC", &nC);
    const char* cs = info->Attribute("cells");
    cells0.clear();
    if (cs) {
        std::string str(cs);
        size_t pos = 0;
        while (pos < str.size()) {
            size_t sep = str.find(';', pos);
            std::string tok = str.substr(pos, sep == std::string::npos ? sep : sep - pos);
            Cell3DPosition c;
            if (parseVec(tok.c_str(), c)) cells0.push_back(c);
            if (sep == std::string::npos) break;
            pos = sep + 1;
        }
    }
    infoParsed = !cells0.empty();
}

void MetaModuleGenBlockCode::startup() {
    if (hostBlock->blockId == BaseSimulator::getWorld()->getMap().begin()->second->blockId) {
        std::cout << "\n=== Meta-module interactive viewer ===\n"
                  << "  click a module to select its meta-module\n"
                  << "  o : cycle operation (Dismantle / Transfer / Assemble)\n"
                  << "  k : cycle direction (+X -X +Y -Y +Z -Z)\n"
                  << "  e : execute selected operation\n"
                  << "  v : toggle motion (rotations <-> teleport)\n"
                  << "  c : print these controls\n"
                  << "current: op=" << opName(opIdx) << " dir=" << dirName(dirIdx)
                  << (infoParsed ? "" : "  [metaModuleInfo missing!]") << "\n\n";
    }
}

// Update the selected meta-module from a clicked module's cell. Returns true
// if the cell belongs to a known meta-module.
bool MetaModuleGenBlockCode::selectFromCell(const Cell3DPosition& cell) {
    Cell3DPosition meta;
    if (!metaOfCell(cell, meta)) {
        std::cout << "[MetaModuleViz] cell " << cell
                  << " is not part of a known meta-module.\n";
        return false;
    }
    if (hasSelection && meta == selMeta) return true;   // unchanged
    selMeta = meta;
    hasSelection = true;
    // Highlight the selected meta-module red, everything else grey.
    World* w = BaseSimulator::getWorld();
    CellSet sel;
    for (const auto& c : cellsOf(selMeta)) sel.insert(c);
    for (const auto& kv : w->getMap())
        kv.second->setColor(sel.count(kv.second->position) ? RED : Color(150, 150, 150));
    std::cout << "[MetaModuleViz] selected meta-module " << selMeta
              << "  (op=" << opName(opIdx) << " dir=" << dirName(dirIdx) << ")\n";
    return true;
}

void MetaModuleGenBlockCode::onBlockSelected() {
    // Catoms3D calls this after every rotation, which would otherwise re-select
    // (and recolor for) whatever meta-cell a rolling module passes through.
    // Ignore it while an operation is animating; click-selection is handled in
    // onUserKeyPressed anyway.
    if (infoParsed && !animating) selectFromCell(hostBlock->position);
}

void MetaModuleGenBlockCode::onUserKeyPressed(unsigned char c, int, int) {
    if (!infoParsed) return;
    // Catoms3D doesn't notify the block code on click, but the viewer routes
    // key presses to the currently-selected block — sync our selection from it
    // (but not mid-animation, when blocks are moving).
    if (!animating) {
        BuildingBlock* sel = BaseSimulator::getWorld()->getSelectedBuildingBlock();
        if (sel) selectFromCell(sel->position);
    }

    switch (c) {
        case 'o': opIdx = (opIdx + 1) % 3; break;
        case 'k': dirIdx = (dirIdx + 1) % 6; break;
        case 'v':
            useRotations = !useRotations;
            console << "[MetaModuleViz] motion mode: "
                      << (useRotations ? "rotations (with teleport fallback)"
                                       : "teleport") << "\n";
            return;
        case 'c':
            console << "  click a module, then:  o: operation  k: direction  "
                         "e: execute  v: rotation/teleport\n";
            break;
        case 'e': executeOperation(); return;
        default: return;
    }
    console << "[MetaModuleViz] op=" << opName(opIdx)
            << " dir=" << dirName(dirIdx)
            << (hasSelection ? "" : "  (click a module to select one)") << "\n";
}

void MetaModuleGenBlockCode::onGlDraw() {}

void MetaModuleGenBlockCode::executeOperation() {
    if (animating || !moveQueue.empty()) {
        std::cout << "[MetaModuleViz] an operation is still playing; wait for it.\n";
        return;
    }
    if (!hasSelection) {
        std::cout << "[MetaModuleViz] select a meta-module first (click one).\n";
        return;
    }
    Cell3DPosition unit = unitStep(dirIdx);
    Cell3DPosition meta = selMeta;
    Cell3DPosition nbr  = add(meta, unit);
    Cell3DPosition donor(meta[0]-unit[0], meta[1]-unit[1], meta[2]-unit[2]);

    std::cout << "[MetaModuleViz] executing " << opName(opIdx)
              << " on " << meta << " towards " << dirName(dirIdx) << "\n";

    if (opIdx == 0) {            // Dismantle: meta -> empty +V neighbour
        relocate(meta, nbr, /*buildOrder=*/false);
    } else if (opIdx == 2) {     // Assemble: build the +V neighbour from meta
        relocate(meta, nbr, /*buildOrder=*/true);
    } else {                     // Transfer: flow donor(-V) -> +V through meta
        CellSet occ = worldOccupancy();
        bool donorFull = true;
        for (const auto& cc : cellsOf(donor)) if (!occ.count(cc)) { donorFull = false; break; }
        relocate(donorFull ? donor : meta, nbr, /*buildOrder=*/false);
    }
}

void MetaModuleGenBlockCode::relocate(const Cell3DPosition& srcMeta,
                                      const Cell3DPosition& dstMeta,
                                      bool buildOrder) {
    World* w = BaseSimulator::getWorld();
    Lattice* lat = w->lattice;
    std::vector<Cell3DPosition> src = cellsOf(srcMeta);
    std::vector<Cell3DPosition> dst = cellsOf(dstMeta);

    // Validate: every source occupied, every target free & in grid.
    for (size_t i = 0; i < src.size(); ++i) {
        if (!lat->cellHasBlock(src[i])) {
            std::cout << "  aborted: source cell " << src[i]
                      << " is empty (is a full meta-module selected?).\n";
            return;
        }
        if (!lat->isInGrid(dst[i]) || lat->cellHasBlock(dst[i])) {
            std::cout << "  aborted: target cell " << dst[i]
                      << " is occupied or out of grid (need an empty slot).\n";
            return;
        }
    }

    CellSet occ = worldOccupancy();
    Cell3DPosition srcCtr = centerOf(srcMeta);
    Cell3DPosition dstCtr = centerOf(dstMeta);

    // Order the modules: disassembly = nearest-to-source-center first;
    // assembly = nearest-to-target-center first.
    std::vector<size_t> order(src.size());
    for (size_t i = 0; i < order.size(); ++i) order[i] = i;
    std::vector<int> key(src.size());
    for (size_t i = 0; i < src.size(); ++i) {
        int d = buildOrder ? pathLength(dstCtr, dst[i], occ)
                           : pathLength(src[i], srcCtr, occ);
        key[i] = (d < 0) ? 100000 : d;
    }
    std::sort(order.begin(), order.end(),
              [&](size_t a, size_t b) { return key[a] < key[b]; });

    // Queue one entry per module: (module id, final destination). Each module
    // is then navigated to its destination one rotation at a time (see
    // scheduleNextStep). Order = the disassembly/assembly order computed above.
    for (size_t oi = 0; oi < order.size(); ++oi) {
        size_t i = order[oi];
        BuildingBlock* bb = lat->getBlock(src[i]);
        if (!bb) continue;
        moveQueue.emplace_back(bb->blockId, dst[i]);
    }
    std::cout << "  queued " << moveQueue.size() << " modules to relocate ("
              << (useRotations ? "rotations" : "teleport") << ").\n";
    scheduleNextStep();   // kick off the animation
}

static long dist2(const Cell3DPosition& a, const Cell3DPosition& b) {
    long dx = a[0]-b[0], dy = a[1]-b[1], dz = a[2]-b[2];
    return dx*dx + dy*dy + dz*dz;
}

// Advance the current module one rotation toward its destination, using the
// motion engine's own reachable set so the hop is always a valid rotation.
// Falls back to a single teleport only if greedy navigation stalls.
void MetaModuleGenBlockCode::scheduleNextStep() {
    if (moveQueue.empty()) {
        if (animating) std::cout << "[MetaModuleViz] operation complete.\n";
        animating = false;
        return;
    }
    animating = true;
    World* w = BaseSimulator::getWorld();
    Lattice* lat = w->lattice;
    const auto& front = moveQueue.front();
    bID id = front.first;
    Cell3DPosition dest = front.second;
    auto* m = static_cast<Catoms3DBlock*>(w->getBlockById(id));

    // New module reaching the front: reset its journey state.
    if (id != journeyId) { journeyId = id; journeyVisited.clear(); journeyHops = 0; }

    if (!m || m->position == dest) {            // gone or arrived
        moveQueue.pop_front();
        journeyId = 0;
        scheduleNextStep();
        return;
    }

    Time t = BaseSimulator::getScheduler()->now();
    const int kMaxHops = 50;
    Cell3DPosition cur = m->position;
    long curD = dist2(cur, dest);

    if (useRotations && journeyHops < kMaxHops) {
        // Enumerate the module's real rotations, ask each for its true final
        // position, and pick the free, unvisited one that gets closest to dest.
        // Using the actual rotation (not a guessed target cell) guarantees the
        // module lands exactly where we validated as free — no double-insertion.
        const auto rots = Catoms3DMotionEngine::getAllRotationsForModule(m);
        bool found = false;
        long bestD = curD;
        Catoms3DRotation bestRot;
        for (const auto& pr : rots) {
            Catoms3DRotation rot = pr.second;
            Cell3DPosition fpos;
            short fori;
            rot.getFinalPositionAndOrientation(fpos, fori);
            if (!lat->isInGrid(fpos) || !lat->isFree(fpos)) continue;
            if (journeyVisited.count(fpos)) continue;
            long d = dist2(fpos, dest);
            if (!found || d < bestD) { found = true; bestD = d; bestRot = rot; }
        }
        if (found && bestD < curD) {
            journeyVisited.insert(cur);
            ++journeyHops;
            BaseSimulator::getScheduler()->schedule(
                new Catoms3DRotationStartEvent(t, m, bestRot));
            return;     // same module continues from onMotionEnd
        }
    }

    // Stalled (no improving rotation), hop cap hit, or teleport mode: jump to
    // the destination if it is free, then finish this module.
    moveQueue.pop_front();
    journeyId = 0;
    if (lat->isFree(dest)) {
        if (useRotations)
            std::cout << "  module " << id << " rolled as far as it could; "
                      << "teleporting it to " << dest << ".\n";
        BaseSimulator::getScheduler()->schedule(
            new BaseSimulator::TeleportationStartEvent(t, m, dest));
        return;     // teleport end will advance the queue
    }
    std::cout << "  module " << id << " could not reach " << dest
              << " (occupied); leaving it in place.\n";
    scheduleNextStep();   // nothing scheduled; advance to the next module now
}

void MetaModuleGenBlockCode::onMotionEnd() {
    // Advance the queued operation, if any.
    if (animating) scheduleNextStep();
}

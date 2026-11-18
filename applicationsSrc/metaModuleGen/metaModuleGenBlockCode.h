#ifndef METAMODULE_GEN_BLOCKCODE_H_
#define METAMODULE_GEN_BLOCKCODE_H_

#include <deque>
#include <utility>
#include <vector>

#include "robots/catoms3D/catoms3DBlockCode.h"
#include "metaModuleCore.h"

// Interactive viewer for the generated meta-module structure.
//
// The structure XML (see writeStructureXml in metaModuleGen.cpp) tiles the
// discovered meta-module into a block of meta-cells and embeds a
// <metaModuleInfo> element. This block code parses that element so it can:
//   - click-select a meta-module (click any of its modules),
//   - cycle the operation  (o : Dismantle -> Transfer -> Assemble),
//   - cycle the direction   (k : +X -X +Y -Y +Z -Z),
//   - execute               (g),
// and animate the operation by teleporting modules cell-by-cell.
class MetaModuleGenBlockCode : public Catoms3D::Catoms3DBlockCode {
public:
    explicit MetaModuleGenBlockCode(Catoms3D::Catoms3DBlock* host)
        : Catoms3D::Catoms3DBlockCode(host) {}
    ~MetaModuleGenBlockCode() override = default;

    void startup() override;
    void parseUserElements(TiXmlDocument* config) override;
    void onBlockSelected() override;
    void onUserKeyPressed(unsigned char c, int x, int y) override;
    void onMotionEnd() override;
    void onGlDraw() override;

    static BlockCode* buildNewBlockCode(BuildingBlock* host) {
        return new MetaModuleGenBlockCode(static_cast<Catoms3D::Catoms3DBlock*>(host));
    }

private:
    // ---- shared (static) state, parsed once from <metaModuleInfo> ----
    static bool infoParsed;
    static Cell3DPosition bX, bY, bZ;          // lattice basis (module units)
    static Cell3DPosition center0;             // center cell of meta (0,0,0)
    static std::vector<Cell3DPosition> cells0; // cells of meta (0,0,0)
    static int nA, nB, nC;                     // tiled block extents

    // ---- interaction state ----
    static int opIdx;       // 0=Dismantle 1=Transfer 2=Assemble
    static int dirIdx;      // 0..5 -> +X -X +Y -Y +Z -Z
    static Cell3DPosition selMeta;  // meta-coordinate of selected meta-module
    static bool hasSelection;
    static bool useRotations;  // true: real Catoms3D rotations, false: teleport
    static bool animating;     // an operation is currently playing

    // Reactive animation: each entry is (module id, final destination cell) for
    // one module. The module is navigated to its destination one rotation at a
    // time, greedily, through the motion engine's own reachable set (so every
    // hop is a valid rotation). Advanced from onMotionEnd().
    static std::deque<std::pair<bID, Cell3DPosition>> moveQueue;
    // Per-module journey state (reset when a new module reaches the front).
    static bID journeyId;
    static metaModuleGen::CellSet journeyVisited;
    static int journeyHops;

    // ---- helpers (implemented in the .cpp) ----
    static Cell3DPosition latticeShift(int a, int b, int c);
    static std::vector<Cell3DPosition> cellsOf(const Cell3DPosition& meta);
    static Cell3DPosition centerOf(const Cell3DPosition& meta);
    static Cell3DPosition unitStep(int dir);     // meta-coord step for a dir
    static bool metaOfCell(const Cell3DPosition& p, Cell3DPosition& metaOut);
    static bool selectFromCell(const Cell3DPosition& cell);
    static metaModuleGen::CellSet worldOccupancy();
    void executeOperation();
    void relocate(const Cell3DPosition& srcMeta, const Cell3DPosition& dstMeta,
                  bool buildOrder);
    static void scheduleNextStep();   // pop & play one move from moveQueue
    static const char* opName(int i);
    static const char* dirName(int i);
};

#endif // METAMODULE_GEN_BLOCKCODE_H_

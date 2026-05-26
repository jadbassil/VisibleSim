#ifndef META_MODULE_CORE_H_
#define META_MODULE_CORE_H_

#include <array>
#include <cstdint>
#include <iosfwd>
#include <optional>
#include <set>
#include <vector>

#include "math/cell3DPosition.h"

namespace metaModuleGen {

// ---------------------------------------------------------------------------
// FCC lattice helpers (independent of the simulator runtime)
// ---------------------------------------------------------------------------

// 12 neighbors of an FCC cell. Indexing matches lattice.h.
const std::vector<Cell3DPosition>& fccNeighbors(const Cell3DPosition& p);

bool fccAdjacent(const Cell3DPosition& a, const Cell3DPosition& b);

using CellSet = std::set<Cell3DPosition>;

// Motion predicate following the paper's move(s, t, X):
// - t is free (t not in occupied),
// - some pivot p in occupied is adjacent to both s and t,
// - t is not locked between two opposing occupied cells (Catoms3D rule).
bool canMove(const Cell3DPosition& s,
             const Cell3DPosition& t,
             const CellSet& occupied);

// Reachable cells from s assuming the moving module is at s and "occupied"
// is the set of pivot cells (typically including s). Cells that are part of
// "occupied" are excluded from the result.
CellSet reachableFrom(const Cell3DPosition& s, const CellSet& occupied);

// Shortest path length in moves from s to t (or -1 if not reachable).
int pathLength(const Cell3DPosition& s,
               const Cell3DPosition& t,
               const CellSet& occupied);

// ---------------------------------------------------------------------------
// Meta-module definition
// ---------------------------------------------------------------------------

struct Basis {
    Cell3DPosition X, Y, Z;
    int determinantAbs() const;        // |det| of the lattice
    bool linearlyIndependent() const;  // Property 2, Eq. 7
};

struct MetaModule {
    CellSet cells;                     // M
    Basis basis;                       // X, Y, Z
    Cell3DPosition center{0, 0, 0};    // chosen center cell

    int size() const { return static_cast<int>(cells.size()); }
};

// Union of M and its lattice neighbors, optionally with a hole carved out
// (used by Construction planner to exclude the target meta-module).
CellSet tiledNeighborhood(const MetaModule& mm,
                          const Cell3DPosition* skip = nullptr);

// ---------------------------------------------------------------------------
// Geometric validators
// ---------------------------------------------------------------------------

bool isConnected(const CellSet& cells);

// Property 2 (Eq. 8) — every cell in M belongs to exactly one meta-module
bool isExclusive(const MetaModule& mm);

// Property 2 (Eq. 9) — contact along every axis
bool hasAxisContacts(const MetaModule& mm);

// Definition 6 — find any valid center cell (Eq. 10)
std::optional<Cell3DPosition> findCenter(const MetaModule& mm);

// Composite geometric check (Connectivity + Independence + Exclusivity +
// Contacts + Center existence).
bool isGeometricallyValid(const MetaModule& mm);

// ---------------------------------------------------------------------------
// Operation planners (paper §5)
// ---------------------------------------------------------------------------

struct TransferPlan {
    bool feasible{false};
    int nBridges{0};
    int nSetupBridgeMoves{0};
    int nTransferMovesPerModule{0};
    int nClearBridgeMoves{0};

    int totalMoves(int nModulesTransferred = 1) const {
        return nSetupBridgeMoves
             + nTransferMovesPerModule * nModulesTransferred
             + nClearBridgeMoves;
    }
};

TransferPlan planTransfer(const MetaModule& mm, const Cell3DPosition& V);

// Returns aggregate transfer cost over all 6 ±-directions; sets feasible=false
// (and total=INT_MAX) if any direction is infeasible.
struct AllDirectionsCost {
    bool feasible{true};
    int totalTransferMoves{0};
    std::array<TransferPlan, 6> perDirection{};
};

AllDirectionsCost evaluateMetaModule(const MetaModule& mm);

// ---------------------------------------------------------------------------
// Search/GA driver
// ---------------------------------------------------------------------------

struct GAConfig {
    int populationSize{120};
    int generations{200};
    double mutationRate{0.25};
    double crossoverRate{0.7};
    int tournamentK{3};
    int minSize{4};       // smallest |M| to try
    int maxSize{30};      // upper bound on |M|
    int boundingRadius{3};// shape lives in [-r..r]^3
    uint32_t seed{42};
    bool verbose{true};
};

struct CandidateBasis {
    Basis basis;
    std::string label;
};

struct GenerationResult {
    bool found{false};
    MetaModule best;
    AllDirectionsCost cost{};
    std::string basisLabel;
};

// Run the GA: tries successively larger module counts, starting with the
// smallest, and for each size searches across all candidate bases. Returns
// the first meta-module that satisfies every operational constraint.
GenerationResult runGenerator(const std::vector<CandidateBasis>& bases,
                              const GAConfig& cfg);

// Default candidate bases: a few orthogonal options + the hexagonal/scaffolding
// inspired bases reported in the paper.
std::vector<CandidateBasis> defaultCandidateBases();

// Pretty printer
std::ostream& operator<<(std::ostream& os, const MetaModule& mm);

} // namespace metaModuleGen

#endif // META_MODULE_CORE_H_

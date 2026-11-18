#ifndef META_MODULE_CORE_H_
#define META_MODULE_CORE_H_

#include <array>
#include <cstdint>
#include <iosfwd>
#include <optional>
#include <set>
#include <string>
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

// Like pathLength but returns the actual sequence of intermediate cells the
// moving module passes through, from the first hop up to and including t
// (excluding the start s). Empty if t is unreachable. Used to animate motions.
std::vector<Cell3DPosition> findPath(const Cell3DPosition& s,
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

// Disassembly / Assembly (paper §5.3) are modeled as sequences of single-module
// motions. nMoves is the total number of motions in the sequence.
struct OperationPlan {
    bool feasible{false};
    int nMoves{0};
};

// Disassembly in direction V: M takes itself apart, routing every module, one
// at a time, to the staging cell `center` (from where it is transferred toward
// the neighbor along V). The whole neighborhood is treated as filled scaffolding
// except the recipient slot along V. Removing a module must keep the remaining
// structure connected.
OperationPlan planDisassembly(const MetaModule& mm, const Cell3DPosition& V);

// Assembly in direction V: a fresh copy of M is built one module at a time from
// `center` into an otherwise empty slot, with the whole neighborhood filled as
// scaffolding. Each newly placed module must attach to the partial structure.
OperationPlan planAssembly(const MetaModule& mm, const Cell3DPosition& V);

// Returns aggregate relocation cost over all 6 ±-directions. A meta-module is
// feasible iff disassembly, transfer and assembly all succeed in every
// direction; otherwise feasible=false and the totals are INT_MAX.
struct AllDirectionsCost {
    bool feasible{true};
    int feasiblePairs{0};         // # feasible (operation, direction) pairs, 0..18
    int totalTransferMoves{0};
    int totalDisassemblyMoves{0};
    int totalAssemblyMoves{0};
    int totalMoves{0};            // transfer + disassembly + assembly
    std::array<TransferPlan, 6> perDirection{};
    std::array<OperationPlan, 6> disassembly{};
    std::array<OperationPlan, 6> assembly{};
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
    int boundingRadius{4};// shape lives in [-r..r]^3
    uint32_t seed{42};
    bool verbose{true};
};

struct CandidateBasis {
    Basis basis;
    std::string label;
};

// One feasible meta-module kept by a search.
struct FoundDesign {
    MetaModule mm;
    AllDirectionsCost cost{};
    std::string basisLabel;
};

struct GenerationResult {
    bool found{false};
    MetaModule best;
    AllDirectionsCost cost{};
    std::string basisLabel;
    // Every feasible design the search kept, sorted by size then total moves.
    // enum: all feasible shapes it evaluated. mapelites: the feasible elites.
    // The GA leaves it empty (it only keeps `best`).
    std::vector<FoundDesign> designs;
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

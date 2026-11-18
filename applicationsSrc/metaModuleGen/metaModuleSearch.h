#ifndef META_MODULE_SEARCH_H_
#define META_MODULE_SEARCH_H_

// Alternative searches to the GA in metaModuleCore:
//  - exhaustive enumeration of every connected shape of a given size
//    (Redelmeier's algorithm with exclusivity pruning), which proves the
//    smallest feasible size within the planner model;
//  - MAP-Elites over a (size x basis) archive, which maps the whole design
//    space in a single run.

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "metaModuleCore.h"

namespace metaModuleGen {

/// Score of one (shape, basis) candidate, shared by every search.
struct ShapeEval {
    bool exclusive{false};      ///< Property 2, Eq. 8
    int axisContacts{0};        ///< number of basis axes with contact, 0..3
    bool hasCenter{false};      ///< Definition 6
    Cell3DPosition center{0, 0, 0};
    AllDirectionsCost cost{};   ///< filled only when the geometry is valid
    double score{-1e18};        ///< higher is better

    bool geometricallyValid() const {
        return exclusive && axisContacts == 3 && hasCenter;
    }
    bool feasible() const { return geometricallyValid() && cost.feasible; }
};

/// Score a connected shape against a basis. Graded so that searches get a
/// signal before full feasibility: exclusivity < axis contacts < center <
/// feasible (operation, direction) pairs < feasible with fewer moves.
ShapeEval evaluateShape(const CellSet& cells, const Basis& basis);

/// Translate a shape so its smallest cell (x, y, z order) lands on (0,0,0)
/// or (0,0,1). The z shift is always even because FCC neighborhoods depend
/// on z parity, so the result is evaluated exactly like the input.
CellSet canonicalForm(const CellSet& cells);

/// Largest bounding-box side of a shape, in cells.
int shapeExtent(const CellSet& cells);

/// Call `visit` once for every connected shape of exactly `size` cells, up
/// to translation (dx, dy, even dz), whose bounding box fits in `maxExtent`
/// cells per axis. If `pruneBasis` is given, shapes that break exclusivity
/// for it are cut early: adding cells can never repair a broken exclusivity.
void enumerateShapes(
    int size, int maxExtent, const Basis* pruneBasis,
    const std::function<void(const std::vector<Cell3DPosition>&)>& visit);

/// Number of worker threads to use when the caller asks for 0 (= auto).
int defaultThreadCount();

// ---------------------------------------------------------------------------
// Exhaustive enumeration
// ---------------------------------------------------------------------------

struct EnumConfig {
    int minSize{4};
    int maxSize{30};
    int maxExtent{9};          ///< 2 * radius + 1
    int threads{0};            ///< 0 = hardware concurrency
    bool stopAtFirstSize{true};///< stop once a size admits a feasible shape
    std::string csvPath;       ///< every feasible shape found (empty = none)
    bool verbose{true};
};

/// For each size, enumerates every shape for every basis and evaluates all of
/// them. Returns the cheapest feasible shape of the smallest feasible size.
GenerationResult runEnumeration(const std::vector<CandidateBasis>& bases,
                                const EnumConfig& cfg);

// ---------------------------------------------------------------------------
// MAP-Elites
// ---------------------------------------------------------------------------

struct MapElitesConfig {
    int minSize{4};
    int maxSize{30};
    int maxExtent{9};          ///< 2 * radius + 1
    int initSamples{500};      ///< random shapes used to seed the archive
    long iterations{20000};    ///< children generated after seeding
    int batchSize{64};         ///< children evaluated in parallel per step
    double basisSwitchRate{0.1};
    /// Share of parents picked by a 2-way score tournament instead of
    /// uniformly. Focuses effort on promising niches without starving others.
    double tournamentRate{0.5};
    int threads{0};            ///< 0 = hardware concurrency
    uint32_t seed{42};
    int logEvery{2000};
    std::string archiveCsv;    ///< final archive dump (empty = none)
    bool verbose{true};
};

/// Archive niches are (|M|, basis). Each keeps its best shape by
/// ShapeEval::score. Returns the cheapest feasible elite of the smallest
/// feasible size.
GenerationResult runMapElites(const std::vector<CandidateBasis>& bases,
                              const MapElitesConfig& cfg);

} // namespace metaModuleGen

#endif // META_MODULE_SEARCH_H_

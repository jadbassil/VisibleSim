#include "metaModuleSearch.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <climits>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <unordered_set>

namespace metaModuleGen {

// ===========================================================================
// Shared helpers
// ===========================================================================

namespace {

using Clock = std::chrono::steady_clock;

double secondsSince(Clock::time_point t0) {
    return std::chrono::duration<double>(Clock::now() - t0).count();
}

inline Cell3DPosition add(const Cell3DPosition& a, const Cell3DPosition& b) {
    return Cell3DPosition(a[0] + b[0], a[1] + b[1], a[2] + b[2]);
}

inline uint64_t cellKey(const Cell3DPosition& c) {
    return (static_cast<uint64_t>(static_cast<uint16_t>(c[0])) << 32)
         | (static_cast<uint64_t>(static_cast<uint16_t>(c[1])) << 16)
         |  static_cast<uint64_t>(static_cast<uint16_t>(c[2]));
}

// The 26 nonzero shifts aX + bY + cZ with a, b, c in {-1, 0, 1}. These are
// exactly the shifts isExclusive() tests.
std::vector<Cell3DPosition> latticeShifts(const Basis& B) {
    std::vector<Cell3DPosition> out;
    for (int a = -1; a <= 1; ++a)
        for (int b = -1; b <= 1; ++b)
            for (int c = -1; c <= 1; ++c) {
                if (a == 0 && b == 0 && c == 0) continue;
                out.emplace_back(a*B.X[0] + b*B.Y[0] + c*B.Z[0],
                                 a*B.X[1] + b*B.Y[1] + c*B.Z[1],
                                 a*B.X[2] + b*B.Y[2] + c*B.Z[2]);
            }
    return out;
}

bool axisContact(const MetaModule& mm, const Cell3DPosition& V) {
    for (const auto& a : mm.cells)
        for (const auto& b : mm.cells)
            if (fccAdjacent(a, add(b, V))) return true;
    return false;
}

int countAxisContacts(const MetaModule& mm) {
    return int(axisContact(mm, mm.basis.X))
         + int(axisContact(mm, mm.basis.Y))
         + int(axisContact(mm, mm.basis.Z));
}

// Run f(i) for i in [0, n) on up to `threads` workers.
template <class F>
void parallelFor(size_t n, int threads, F&& f) {
    if (threads <= 1 || n < 2) {
        for (size_t i = 0; i < n; ++i) f(i);
        return;
    }
    std::atomic<size_t> next{0};
    std::vector<std::thread> pool;
    const size_t nt = std::min<size_t>(threads, n);
    pool.reserve(nt);
    for (size_t t = 0; t < nt; ++t) {
        pool.emplace_back([&]() {
            for (size_t i; (i = next.fetch_add(1)) < n;) f(i);
        });
    }
    for (auto& th : pool) th.join();
}

std::string cellsToString(const CellSet& cells) {
    std::ostringstream os;
    bool first = true;
    for (const auto& c : cells) {
        if (!first) os << ";";
        os << c[0] << " " << c[1] << " " << c[2];
        first = false;
    }
    return os.str();
}

int movesOrMinus1(int v) { return v == INT_MAX ? -1 : v; }

void sortDesigns(std::vector<FoundDesign>& v) {
    std::stable_sort(v.begin(), v.end(),
                     [](const FoundDesign& a, const FoundDesign& b) {
                         if (a.mm.size() != b.mm.size()) return a.mm.size() < b.mm.size();
                         return a.cost.totalMoves < b.cost.totalMoves;
                     });
}

// True if `a` should replace `b` as the overall answer: smaller size first,
// then fewer moves.
bool betterAnswer(int sizeA, int movesA, const GenerationResult& b) {
    if (!b.found) return true;
    if (sizeA != b.best.size()) return sizeA < b.best.size();
    return movesA < b.cost.totalMoves;
}

} // namespace

int defaultThreadCount() {
    unsigned n = std::thread::hardware_concurrency();
    return n == 0 ? 1 : static_cast<int>(n);
}

ShapeEval evaluateShape(const CellSet& cells, const Basis& basis) {
    ShapeEval e;
    if (cells.empty() || !basis.linearlyIndependent()) return e;
    MetaModule mm{cells, basis, Cell3DPosition(0, 0, 0)};

    e.exclusive = isExclusive(mm);
    if (!e.exclusive) { e.score = -4e9; return e; }

    e.axisContacts = countAxisContacts(mm);
    if (e.axisContacts < 3) { e.score = -3e9 + 1e8 * e.axisContacts; return e; }

    auto ctr = findCenter(mm);
    if (!ctr.has_value()) { e.score = -2.5e9; return e; }
    e.hasCenter = true;
    e.center = *ctr;
    mm.center = *ctr;

    e.cost = evaluateMetaModule(mm);
    if (!e.cost.feasible)
        e.score = -2e9 + 1e7 * static_cast<double>(e.cost.feasiblePairs);
    else
        e.score = -static_cast<double>(e.cost.totalMoves);
    return e;
}

CellSet canonicalForm(const CellSet& cells) {
    if (cells.empty()) return cells;
    const Cell3DPosition& m = *cells.begin();  // smallest in (x, y, z) order
    const int parity = ((m[2] % 2) + 2) % 2;
    const int dx = -m[0], dy = -m[1], dz = -(m[2] - parity);
    CellSet out;
    for (const auto& c : cells)
        out.insert(Cell3DPosition(c[0] + dx, c[1] + dy, c[2] + dz));
    return out;
}

int shapeExtent(const CellSet& cells) {
    if (cells.empty()) return 0;
    int mn[3] = {INT_MAX, INT_MAX, INT_MAX}, mx[3] = {INT_MIN, INT_MIN, INT_MIN};
    for (const auto& c : cells)
        for (int k = 0; k < 3; ++k) {
            mn[k] = std::min<int>(mn[k], c[k]);
            mx[k] = std::max<int>(mx[k], c[k]);
        }
    int e = 0;
    for (int k = 0; k < 3; ++k) e = std::max(e, mx[k] - mn[k] + 1);
    return e;
}

// ===========================================================================
// Redelmeier enumeration
// ===========================================================================
//
// Every shape is generated exactly once, from its smallest cell (the origin).
// A cell enters the "untried" list at most once per branch (it is tagged when
// first seen), and once popped it is excluded from all later siblings. This
// makes the search a tree with one leaf per shape, so no duplicate check is
// needed.

namespace {

class Enumerator {
public:
    Enumerator(int size, int maxExtent, const Basis* basis,
               const std::function<void(const std::vector<Cell3DPosition>&)>& visit)
        : target_(size), maxExtent_(maxExtent), visit_(visit) {
        if (basis) {
            for (const auto& s : latticeShifts(*basis)) shifts_.insert(cellKey(s));
        }
    }

    void run(const Cell3DPosition& origin) {
        origin_ = origin;
        shape_.clear();
        tagged_.clear();
        tagged_.insert(cellKey(origin));
        recurse({origin});
    }

private:
    bool allowed(const Cell3DPosition& c) const { return origin_ < c; }

    // Adding c to shape_ keeps the bounding box small enough and does not
    // place two cells one lattice shift apart. Both conditions only get
    // worse as cells are added, so a failure prunes the whole subtree.
    bool compatible(const Cell3DPosition& c) const {
        for (int k = 0; k < 3; ++k) {
            int mn = c[k], mx = c[k];
            for (const auto& p : shape_) {
                mn = std::min<int>(mn, p[k]);
                mx = std::max<int>(mx, p[k]);
            }
            if (mx - mn + 1 > maxExtent_) return false;
        }
        if (!shifts_.empty()) {
            for (const auto& p : shape_) {
                Cell3DPosition d(c[0] - p[0], c[1] - p[1], c[2] - p[2]);
                if (shifts_.count(cellKey(d))) return false;
            }
        }
        return true;
    }

    void recurse(std::vector<Cell3DPosition> untried) {
        while (!untried.empty()) {
            Cell3DPosition c = untried.back();
            untried.pop_back();
            if (!compatible(c)) continue;

            shape_.push_back(c);
            if (static_cast<int>(shape_.size()) == target_) {
                visit_(shape_);
            } else {
                std::vector<Cell3DPosition> added;
                for (const auto& d : fccNeighbors(c)) {
                    Cell3DPosition n = add(c, d);
                    if (!allowed(n)) continue;
                    if (tagged_.insert(cellKey(n)).second) added.push_back(n);
                }
                std::vector<Cell3DPosition> next = untried;
                next.insert(next.end(), added.begin(), added.end());
                recurse(std::move(next));
                for (const auto& n : added) tagged_.erase(cellKey(n));
            }
            shape_.pop_back();
        }
    }

    int target_;
    int maxExtent_;
    const std::function<void(const std::vector<Cell3DPosition>&)>& visit_;
    std::unordered_set<uint64_t> shifts_;
    Cell3DPosition origin_{0, 0, 0};
    std::vector<Cell3DPosition> shape_;
    std::unordered_set<uint64_t> tagged_;
};

} // namespace

void enumerateShapes(
    int size, int maxExtent, const Basis* pruneBasis,
    const std::function<void(const std::vector<Cell3DPosition>&)>& visit) {
    if (size <= 0) return;
    Enumerator en(size, maxExtent, pruneBasis, visit);
    // Translations keep z parity, so shapes whose smallest cell sits on an
    // even layer and on an odd layer form separate classes.
    en.run(Cell3DPosition(0, 0, 0));
    en.run(Cell3DPosition(0, 0, 1));
}

// ===========================================================================
// Exhaustive search driver
// ===========================================================================

GenerationResult runEnumeration(const std::vector<CandidateBasis>& bases,
                                const EnumConfig& cfg) {
    const int threads = cfg.threads > 0 ? cfg.threads : defaultThreadCount();
    const size_t kBatch = 4096;
    GenerationResult best;

    std::ofstream csv;
    if (!cfg.csvPath.empty()) {
        csv.open(cfg.csvPath);
        csv << "size,basis,total_moves,transfer,disassembly,assembly,center,cells\n";
    }

    if (cfg.verbose)
        std::cout << "Exhaustive enumeration: sizes " << cfg.minSize << ".."
                  << cfg.maxSize << ", max extent " << cfg.maxExtent
                  << ", " << threads << " threads\n";

    for (int size = cfg.minSize; size <= cfg.maxSize; ++size) {
        bool anyFeasible = false;
        if (cfg.verbose) std::cout << "=== size |M| = " << size << " ===\n";

        for (const auto& cb : bases) {
            const auto t0 = Clock::now();
            long long nShapes = 0, nGeom = 0, nTransfer = 0, nFeasible = 0;
            int bestMoves = INT_MAX;
            long long nextProgress = 1000000;

            std::vector<CellSet> batch;
            batch.reserve(kBatch);
            auto flush = [&]() {
                std::vector<ShapeEval> res(batch.size());
                parallelFor(batch.size(), threads, [&](size_t i) {
                    res[i] = evaluateShape(batch[i], cb.basis);
                });
                for (size_t i = 0; i < batch.size(); ++i) {
                    const ShapeEval& e = res[i];
                    if (!e.geometricallyValid()) continue;
                    ++nGeom;
                    if (e.cost.feasiblePairs >= 6) ++nTransfer;
                    if (!e.cost.feasible) continue;
                    ++nFeasible;
                    anyFeasible = true;
                    bestMoves = std::min(bestMoves, e.cost.totalMoves);
                    if (csv) {
                        csv << size << ",\"" << cb.label << "\","
                            << e.cost.totalMoves << ","
                            << e.cost.totalTransferMoves << ","
                            << e.cost.totalDisassemblyMoves << ","
                            << e.cost.totalAssemblyMoves << ",\""
                            << e.center[0] << " " << e.center[1] << " "
                            << e.center[2] << "\",\""
                            << cellsToString(batch[i]) << "\"\n";
                    }
                    best.designs.push_back(
                        {MetaModule{batch[i], cb.basis, e.center}, e.cost, cb.label});
                    if (betterAnswer(size, e.cost.totalMoves, best)) {
                        best.found = true;
                        best.best.cells = batch[i];
                        best.best.basis = cb.basis;
                        best.best.center = e.center;
                        best.cost = e.cost;
                        best.basisLabel = cb.label;
                    }
                }
                batch.clear();
                if (cfg.verbose && nShapes >= nextProgress) {
                    std::cout << "    ... " << nShapes << " shapes, "
                              << nFeasible << " feasible ("
                              << std::fixed << std::setprecision(1)
                              << secondsSince(t0) << "s)" << std::endl;
                    while (nextProgress <= nShapes) nextProgress *= 2;
                }
            };

            enumerateShapes(size, cfg.maxExtent, &cb.basis,
                [&](const std::vector<Cell3DPosition>& s) {
                    batch.emplace_back(s.begin(), s.end());
                    ++nShapes;
                    if (batch.size() >= kBatch) flush();
                });
            flush();

            if (cfg.verbose) {
                std::cout << "  [" << cb.label << " |M|=" << size << "] "
                          << nShapes << " shapes, "
                          << nGeom << " geometric, "
                          << nTransfer << " pass transfer, "
                          << nFeasible << " feasible";
                if (nFeasible > 0) std::cout << ", best moves=" << bestMoves;
                std::cout << "  (" << std::fixed << std::setprecision(1)
                          << secondsSince(t0) << "s)" << std::endl;
            }
        }

        if (anyFeasible && cfg.stopAtFirstSize) {
            if (cfg.verbose && size > cfg.minSize)
                std::cout << "Every shape of size " << cfg.minSize << ".."
                          << (size - 1) << " was checked: none is feasible.\n";
            break;
        }
    }
    sortDesigns(best.designs);
    return best;
}

// ===========================================================================
// MAP-Elites
// ===========================================================================

namespace {

struct Candidate {
    CellSet cells;
    int basis{0};
    ShapeEval eval;
};

std::vector<Cell3DPosition> frontierOf(const CellSet& s) {
    CellSet f;
    for (const auto& p : s)
        for (const auto& d : fccNeighbors(p)) {
            Cell3DPosition n = add(p, d);
            if (!s.count(n)) f.insert(n);
        }
    return {f.begin(), f.end()};
}

bool addRandomCell(CellSet& s, int maxExtent, std::mt19937& rng) {
    auto f = frontierOf(s);
    std::shuffle(f.begin(), f.end(), rng);
    for (const auto& c : f) {
        s.insert(c);
        if (shapeExtent(s) <= maxExtent) return true;
        s.erase(c);
    }
    return false;
}

bool removeRandomCell(CellSet& s, std::mt19937& rng) {
    if (s.size() <= 1) return false;
    std::vector<Cell3DPosition> v(s.begin(), s.end());
    std::shuffle(v.begin(), v.end(), rng);
    for (const auto& c : v) {
        s.erase(c);
        if (isConnected(s)) return true;
        s.insert(c);
    }
    return false;
}

std::string candidateKey(const CellSet& cells, int basis) {
    std::string k;
    k.reserve(2 + cells.size() * 6);
    k.push_back(static_cast<char>(basis));
    for (const auto& c : cells)
        for (int i = 0; i < 3; ++i) {
            uint16_t v = static_cast<uint16_t>(c[i]);
            k.push_back(static_cast<char>(v & 0xff));
            k.push_back(static_cast<char>(v >> 8));
        }
    return k;
}

} // namespace

GenerationResult runMapElites(const std::vector<CandidateBasis>& bases,
                              const MapElitesConfig& cfg) {
    const int threads = cfg.threads > 0 ? cfg.threads : defaultThreadCount();
    const int nB = static_cast<int>(bases.size());
    const int nS = cfg.maxSize - cfg.minSize + 1;
    std::mt19937 rng(cfg.seed);
    std::uniform_real_distribution<double> unif(0.0, 1.0);
    const auto t0 = Clock::now();

    std::vector<Candidate> archive(static_cast<size_t>(nS) * nB);
    std::vector<bool> filled(archive.size(), false);
    std::vector<int> filledIdx;
    std::unordered_map<std::string, ShapeEval> cache;
    long long realEvals = 0, generated = 0;

    auto niche = [&](const Candidate& c) {
        return (static_cast<int>(c.cells.size()) - cfg.minSize) * nB + c.basis;
    };

    // Evaluate a batch, reusing the cache and evaluating each new key once.
    auto evaluateBatch = [&](std::vector<Candidate>& batch) {
        std::vector<std::string> keys(batch.size());
        std::vector<size_t> todo;
        std::unordered_map<std::string, size_t> firstSeen;
        std::vector<std::pair<size_t, size_t>> dups;
        for (size_t i = 0; i < batch.size(); ++i) {
            keys[i] = candidateKey(batch[i].cells, batch[i].basis);
            auto hit = cache.find(keys[i]);
            if (hit != cache.end()) { batch[i].eval = hit->second; continue; }
            auto seen = firstSeen.find(keys[i]);
            if (seen != firstSeen.end()) { dups.push_back({i, seen->second}); continue; }
            firstSeen[keys[i]] = i;
            todo.push_back(i);
        }
        parallelFor(todo.size(), threads, [&](size_t j) {
            Candidate& c = batch[todo[j]];
            c.eval = evaluateShape(c.cells, bases[c.basis].basis);
        });
        for (size_t i : todo) cache[keys[i]] = batch[i].eval;
        for (const auto& d : dups) batch[d.first].eval = batch[d.second].eval;
        realEvals += static_cast<long long>(todo.size());
    };

    // Ties replace the incumbent so the archive can drift across plateaus.
    auto insert = [&](Candidate& c) {
        const int idx = niche(c);
        if (!filled[idx]) {
            filled[idx] = true;
            filledIdx.push_back(idx);
            archive[idx] = std::move(c);
        } else if (c.eval.score >= archive[idx].eval.score) {
            archive[idx] = std::move(c);
        }
    };

    auto mutateChild = [&](const Candidate& parent) {
        Candidate child;
        child.cells = parent.cells;
        child.basis = parent.basis;
        std::geometric_distribution<int> extra(0.5);
        const int nOps = 1 + std::min(3, extra(rng));
        for (int k = 0; k < nOps; ++k) {
            if (nB > 1 && unif(rng) < cfg.basisSwitchRate) {
                std::uniform_int_distribution<int> other(1, nB - 1);
                child.basis = (child.basis + other(rng)) % nB;
                continue;
            }
            const int size = static_cast<int>(child.cells.size());
            const int op = std::uniform_int_distribution<int>(0, 2)(rng);
            if (op == 0 && size < cfg.maxSize) {
                addRandomCell(child.cells, cfg.maxExtent, rng);
            } else if (op == 1 && size > cfg.minSize) {
                removeRandomCell(child.cells, rng);
            } else {
                // Move one cell: same size, different shape.
                CellSet backup = child.cells;
                if (!(removeRandomCell(child.cells, rng)
                      && addRandomCell(child.cells, cfg.maxExtent, rng)))
                    child.cells = std::move(backup);
            }
        }
        child.cells = canonicalForm(child.cells);
        return child;
    };

    auto bestFeasible = [&]() -> const Candidate* {
        const Candidate* b = nullptr;
        for (int idx : filledIdx) {
            const Candidate& c = archive[idx];
            if (!c.eval.feasible()) continue;
            if (!b || c.cells.size() < b->cells.size()
                || (c.cells.size() == b->cells.size()
                    && c.eval.cost.totalMoves < b->eval.cost.totalMoves))
                b = &c;
        }
        return b;
    };

    auto report = [&]() {
        int nFeasible = 0, nGeom = 0;
        for (int idx : filledIdx) {
            if (archive[idx].eval.feasible()) ++nFeasible;
            if (archive[idx].eval.geometricallyValid()) ++nGeom;
        }
        std::cout << "  [map-elites] children " << generated
                  << "  evals " << realEvals
                  << "  niches " << filledIdx.size() << "/" << archive.size()
                  << "  geometric " << nGeom
                  << "  feasible " << nFeasible;
        if (const Candidate* b = bestFeasible())
            std::cout << "  best |M|=" << b->cells.size()
                      << " (" << bases[b->basis].label
                      << ", moves=" << b->eval.cost.totalMoves << ")";
        std::cout << "  (" << std::fixed << std::setprecision(1)
                  << secondsSince(t0) << "s)" << std::endl;
    };

    if (cfg.verbose)
        std::cout << "MAP-Elites: " << nS << " sizes x " << nB << " bases = "
                  << archive.size() << " niches, " << cfg.initSamples
                  << " seeds + " << cfg.iterations << " children, "
                  << threads << " threads\n";

    // Seed the archive with random connected shapes of random size and basis.
    {
        std::uniform_int_distribution<int> pickSize(cfg.minSize, cfg.maxSize);
        std::uniform_int_distribution<int> pickBasis(0, nB - 1);
        std::vector<Candidate> seeds(cfg.initSamples);
        for (auto& c : seeds) {
            const int target = pickSize(rng);
            c.basis = pickBasis(rng);
            c.cells.insert(Cell3DPosition(0, 0, 0));
            while (static_cast<int>(c.cells.size()) < target
                   && addRandomCell(c.cells, cfg.maxExtent, rng)) {}
            c.cells = canonicalForm(c.cells);
        }
        evaluateBatch(seeds);
        for (auto& c : seeds)
            if (static_cast<int>(c.cells.size()) >= cfg.minSize) insert(c);
    }
    if (filledIdx.empty()) return {};

    long long nextLog = cfg.logEvery;
    while (generated < cfg.iterations) {
        const long long n = std::min<long long>(cfg.batchSize,
                                                cfg.iterations - generated);
        std::vector<Candidate> batch;
        batch.reserve(n);
        std::uniform_int_distribution<size_t> pick(0, filledIdx.size() - 1);
        for (long long i = 0; i < n; ++i) {
            int idx = filledIdx[pick(rng)];
            if (unif(rng) < cfg.tournamentRate) {
                const int other = filledIdx[pick(rng)];
                if (archive[other].eval.score > archive[idx].eval.score)
                    idx = other;
            }
            batch.push_back(mutateChild(archive[idx]));
        }
        evaluateBatch(batch);
        for (auto& c : batch) insert(c);
        generated += n;
        if (cfg.verbose && cfg.logEvery > 0 && generated >= nextLog) {
            report();
            nextLog += cfg.logEvery;
        }
    }
    if (cfg.verbose) report();

    // Terminal map: rows = sizes, columns = bases.
    if (cfg.verbose) {
        std::cout << "\nArchive map (number = total moves of a feasible elite,"
                     " pN = geometric but only N/18 pairs feasible,"
                     " x = geometry invalid, . = empty)\n";
        for (int b = 0; b < nB; ++b)
            std::cout << "  b" << b << " = " << bases[b].label << "\n";
        std::cout << "  |M| ";
        for (int b = 0; b < nB; ++b) std::cout << std::setw(7) << ("b" + std::to_string(b));
        std::cout << "\n";
        for (int s = 0; s < nS; ++s) {
            std::cout << "  " << std::setw(3) << (cfg.minSize + s) << " ";
            for (int b = 0; b < nB; ++b) {
                const int idx = s * nB + b;
                std::string cell = ".";
                if (filled[idx]) {
                    const ShapeEval& e = archive[idx].eval;
                    if (e.feasible()) cell = std::to_string(e.cost.totalMoves);
                    else if (e.geometricallyValid())
                        cell = "p" + std::to_string(e.cost.feasiblePairs);
                    else cell = "x";
                }
                std::cout << std::setw(7) << cell;
            }
            std::cout << "\n";
        }
    }

    if (!cfg.archiveCsv.empty()) {
        std::ofstream csv(cfg.archiveCsv);
        csv << "size,basis,score,exclusive,axis_contacts,has_center,feasible,"
               "feasible_pairs,transfer,disassembly,assembly,total_moves,center,cells\n";
        for (int idx = 0; idx < static_cast<int>(archive.size()); ++idx) {
            if (!filled[idx]) continue;
            const Candidate& c = archive[idx];
            const ShapeEval& e = c.eval;
            const bool geom = e.geometricallyValid();
            csv << c.cells.size() << ",\"" << bases[c.basis].label << "\","
                << std::setprecision(12) << e.score << ","
                << e.exclusive << "," << e.axisContacts << ","
                << e.hasCenter << "," << e.feasible() << ","
                << (geom ? e.cost.feasiblePairs : 0) << ","
                << (geom ? movesOrMinus1(e.cost.totalTransferMoves) : -1) << ","
                << (geom ? movesOrMinus1(e.cost.totalDisassemblyMoves) : -1) << ","
                << (geom ? movesOrMinus1(e.cost.totalAssemblyMoves) : -1) << ","
                << (geom ? movesOrMinus1(e.cost.totalMoves) : -1) << ",\"";
            if (e.hasCenter)
                csv << e.center[0] << " " << e.center[1] << " " << e.center[2];
            csv << "\",\"" << cellsToString(c.cells) << "\"\n";
        }
        if (cfg.verbose) std::cout << "archive CSV written to: " << cfg.archiveCsv << "\n";
    }

    GenerationResult out;
    if (const Candidate* b = bestFeasible()) {
        out.found = true;
        out.best.cells = b->cells;
        out.best.basis = bases[b->basis].basis;
        out.best.center = b->eval.center;
        out.cost = b->eval.cost;
        out.basisLabel = bases[b->basis].label;
    }
    for (int idx : filledIdx) {
        const Candidate& c = archive[idx];
        if (!c.eval.feasible()) continue;
        out.designs.push_back({MetaModule{c.cells, bases[c.basis].basis, c.eval.center},
                               c.eval.cost, bases[c.basis].label});
    }
    sortDesigns(out.designs);
    return out;
}

} // namespace metaModuleGen

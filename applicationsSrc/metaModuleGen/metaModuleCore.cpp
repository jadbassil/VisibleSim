#include "metaModuleCore.h"

#include <algorithm>
#include <climits>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <queue>
#include <random>

namespace metaModuleGen {

// ===========================================================================
// FCC neighborhood
// ===========================================================================

static inline bool fccEvenZ(int z) { return (std::abs(z) % 2) == 0; }

// Mirrors lattice.h FCCLattice::nCellsEven / nCellsOdd
static const std::vector<Cell3DPosition> kEven = {
    Cell3DPosition( 1, 0, 0),  Cell3DPosition( 0, 1, 0), Cell3DPosition( 0, 0, 1),
    Cell3DPosition(-1, 0, 1),  Cell3DPosition(-1,-1, 1), Cell3DPosition( 0,-1, 1),
    Cell3DPosition(-1, 0, 0),  Cell3DPosition( 0,-1, 0), Cell3DPosition(-1,-1,-1),
    Cell3DPosition( 0,-1,-1),  Cell3DPosition( 0, 0,-1), Cell3DPosition(-1, 0,-1)
};
static const std::vector<Cell3DPosition> kOdd = {
    Cell3DPosition( 1, 0, 0),  Cell3DPosition( 0, 1, 0), Cell3DPosition( 1, 1, 1),
    Cell3DPosition( 0, 1, 1),  Cell3DPosition( 0, 0, 1), Cell3DPosition( 1, 0, 1),
    Cell3DPosition(-1, 0, 0),  Cell3DPosition( 0,-1, 0), Cell3DPosition( 0, 0,-1),
    Cell3DPosition( 1, 0,-1),  Cell3DPosition( 1, 1,-1), Cell3DPosition( 0, 1,-1)
};

const std::vector<Cell3DPosition>& fccNeighbors(const Cell3DPosition& p) {
    return fccEvenZ(p[2]) ? kEven : kOdd;
}

bool fccAdjacent(const Cell3DPosition& a, const Cell3DPosition& b) {
    const auto& nbrs = fccNeighbors(a);
    for (const auto& d : nbrs) {
        if (Cell3DPosition(a[0] + d[0], a[1] + d[1], a[2] + d[2]) == b) return true;
    }
    return false;
}

// Returns true if t is locked (two opposing modules around t excluding 's').
static bool isTargetLocked(const Cell3DPosition& s,
                           const Cell3DPosition& t,
                           const CellSet& occ) {
    const auto& nb = fccNeighbors(t);
    for (int i = 0; i < 6; ++i) {
        Cell3DPosition p0(t[0] + nb[i][0],   t[1] + nb[i][1],   t[2] + nb[i][2]);
        Cell3DPosition p1(t[0] + nb[i+6][0], t[1] + nb[i+6][1], t[2] + nb[i+6][2]);
        if (p0 == s || p1 == s) continue;
        if (occ.count(p0) && occ.count(p1)) return true;
    }
    return false;
}

bool canMove(const Cell3DPosition& s,
             const Cell3DPosition& t,
             const CellSet& occupied) {
    if (s == t) return false;
    if (occupied.count(t)) return false;          // target not free
    if (!fccAdjacent(s, t)) {
        // Pivot must be adjacent to both, but s and t themselves are not
        // required to be neighbors; the rotation arc may skip a face.
        // We still require a pivot adjacent to both.
    }
    // find some pivot adjacent to both s and t, present in occupied, != s
    const auto& nbS = fccNeighbors(s);
    for (const auto& dp : nbS) {
        Cell3DPosition p(s[0] + dp[0], s[1] + dp[1], s[2] + dp[2]);
        if (p == t) continue;
        if (!occupied.count(p)) continue;
        if (!fccAdjacent(p, t)) continue;
        if (isTargetLocked(s, t, occupied)) return false;
        return true;
    }
    return false;
}

CellSet reachableFrom(const Cell3DPosition& s, const CellSet& occupied) {
    CellSet visited;
    std::queue<Cell3DPosition> q;
    q.push(s);
    visited.insert(s);
    const size_t kMaxNodes = 400;
    while (!q.empty() && visited.size() < kMaxNodes) {
        Cell3DPosition cur = q.front(); q.pop();
        // Try a bounded set of candidate targets: any cell within 2 hops via
        // neighbors of a pivot. For an FCC catom, reachable cells in 1 motion
        // are among the 18 positions in relReachablePosition; we approximate
        // them by enumerating cells two FCC-steps away.
        std::set<Cell3DPosition> candidates;
        for (const auto& dp : fccNeighbors(cur)) {
            Cell3DPosition pivot(cur[0]+dp[0], cur[1]+dp[1], cur[2]+dp[2]);
            if (!occupied.count(pivot)) continue;
            for (const auto& dq : fccNeighbors(pivot)) {
                Cell3DPosition t(pivot[0]+dq[0], pivot[1]+dq[1], pivot[2]+dq[2]);
                if (t == cur || occupied.count(t)) continue;
                candidates.insert(t);
            }
        }
        for (const auto& t : candidates) {
            if (visited.count(t)) continue;
            if (canMove(cur, t, occupied)) {
                visited.insert(t);
                q.push(t);
            }
        }
    }
    visited.erase(s);
    return visited;
}

int pathLength(const Cell3DPosition& s,
               const Cell3DPosition& t,
               const CellSet& occupied) {
    if (s == t) return 0;
    std::map<Cell3DPosition, int> dist;
    std::queue<Cell3DPosition> q;
    q.push(s);
    dist[s] = 0;
    const size_t kMaxNodes = 400;
    while (!q.empty() && dist.size() < kMaxNodes) {
        Cell3DPosition cur = q.front(); q.pop();
        if (cur == t) return dist[cur];
        std::set<Cell3DPosition> candidates;
        for (const auto& dp : fccNeighbors(cur)) {
            Cell3DPosition pivot(cur[0]+dp[0], cur[1]+dp[1], cur[2]+dp[2]);
            if (!occupied.count(pivot)) continue;
            for (const auto& dq : fccNeighbors(pivot)) {
                Cell3DPosition nx(pivot[0]+dq[0], pivot[1]+dq[1], pivot[2]+dq[2]);
                if (nx == cur || occupied.count(nx)) continue;
                candidates.insert(nx);
            }
        }
        for (const auto& nx : candidates) {
            if (dist.count(nx)) continue;
            if (canMove(cur, nx, occupied)) {
                dist[nx] = dist[cur] + 1;
                q.push(nx);
                if (nx == t) return dist[nx];
            }
        }
    }
    return -1;
}

// ===========================================================================
// Basis / MetaModule
// ===========================================================================

static long det3(const Cell3DPosition& a,
                 const Cell3DPosition& b,
                 const Cell3DPosition& c) {
    long ax=a[0], ay=a[1], az=a[2];
    long bx=b[0], by=b[1], bz=b[2];
    long cx=c[0], cy=c[1], cz=c[2];
    return ax*(by*cz - bz*cy) - ay*(bx*cz - bz*cx) + az*(bx*cy - by*cx);
}

int Basis::determinantAbs() const {
    return static_cast<int>(std::abs(det3(X, Y, Z)));
}

bool Basis::linearlyIndependent() const {
    return det3(X, Y, Z) != 0;
}

CellSet tiledNeighborhood(const MetaModule& mm, const Cell3DPosition* skip) {
    CellSet out;
    static const int R = 1;
    for (int a = -R; a <= R; ++a)
        for (int b = -R; b <= R; ++b)
            for (int c = -R; c <= R; ++c) {
                Cell3DPosition shift(
                    a*mm.basis.X[0] + b*mm.basis.Y[0] + c*mm.basis.Z[0],
                    a*mm.basis.X[1] + b*mm.basis.Y[1] + c*mm.basis.Z[1],
                    a*mm.basis.X[2] + b*mm.basis.Y[2] + c*mm.basis.Z[2]);
                if (skip && shift == *skip) continue;
                for (const auto& p : mm.cells) {
                    out.insert(Cell3DPosition(p[0]+shift[0], p[1]+shift[1], p[2]+shift[2]));
                }
            }
    return out;
}

// ===========================================================================
// Geometric validators
// ===========================================================================

bool isConnected(const CellSet& cells) {
    if (cells.empty()) return false;
    CellSet visited;
    std::queue<Cell3DPosition> q;
    q.push(*cells.begin());
    visited.insert(*cells.begin());
    while (!q.empty()) {
        Cell3DPosition cur = q.front(); q.pop();
        for (const auto& dp : fccNeighbors(cur)) {
            Cell3DPosition n(cur[0]+dp[0], cur[1]+dp[1], cur[2]+dp[2]);
            if (cells.count(n) && !visited.count(n)) {
                visited.insert(n);
                q.push(n);
            }
        }
    }
    return visited.size() == cells.size();
}

bool isExclusive(const MetaModule& mm) {
    // Property 2 Eq. 8: no nonzero lattice shift sends a cell of M back into M.
    static const int R = 1;
    for (int a = -R; a <= R; ++a)
        for (int b = -R; b <= R; ++b)
            for (int c = -R; c <= R; ++c) {
                if (a == 0 && b == 0 && c == 0) continue;
                Cell3DPosition shift(
                    a*mm.basis.X[0] + b*mm.basis.Y[0] + c*mm.basis.Z[0],
                    a*mm.basis.X[1] + b*mm.basis.Y[1] + c*mm.basis.Z[1],
                    a*mm.basis.X[2] + b*mm.basis.Y[2] + c*mm.basis.Z[2]);
                for (const auto& p : mm.cells) {
                    Cell3DPosition q(p[0]+shift[0], p[1]+shift[1], p[2]+shift[2]);
                    if (mm.cells.count(q)) return false;
                }
            }
    return true;
}

static bool axisContact(const MetaModule& mm, const Cell3DPosition& V) {
    for (const auto& a : mm.cells) {
        for (const auto& b : mm.cells) {
            Cell3DPosition bShift(b[0]+V[0], b[1]+V[1], b[2]+V[2]);
            if (fccAdjacent(a, bShift)) return true;
        }
    }
    return false;
}

bool hasAxisContacts(const MetaModule& mm) {
    return axisContact(mm, mm.basis.X)
        && axisContact(mm, mm.basis.Y)
        && axisContact(mm, mm.basis.Z);
}

std::optional<Cell3DPosition> findCenter(const MetaModule& mm) {
    // Candidate cells: empty cells adjacent to M and not occupied by any
    // lattice image of M (Eq. 10).
    CellSet candidates;
    for (const auto& p : mm.cells) {
        for (const auto& dp : fccNeighbors(p)) {
            Cell3DPosition n(p[0]+dp[0], p[1]+dp[1], p[2]+dp[2]);
            if (mm.cells.count(n)) continue;
            candidates.insert(n);
        }
    }
    static const int R = 1;
    for (const auto& c : candidates) {
        bool ok = true;
        for (int a = -R; a <= R && ok; ++a)
            for (int b = -R; b <= R && ok; ++b)
                for (int cc = -R; cc <= R && ok; ++cc) {
                    if (a == 0 && b == 0 && cc == 0) continue;
                    Cell3DPosition shifted(
                        c[0] + a*mm.basis.X[0] + b*mm.basis.Y[0] + cc*mm.basis.Z[0],
                        c[1] + a*mm.basis.X[1] + b*mm.basis.Y[1] + cc*mm.basis.Z[1],
                        c[2] + a*mm.basis.X[2] + b*mm.basis.Y[2] + cc*mm.basis.Z[2]);
                    if (mm.cells.count(shifted)) ok = false;
                }
        if (ok) return c;
    }
    return std::nullopt;
}

bool isGeometricallyValid(const MetaModule& mm) {
    if (mm.cells.empty()) return false;
    if (!mm.basis.linearlyIndependent()) return false;
    if (!isConnected(mm.cells)) return false;
    if (!isExclusive(mm)) return false;
    if (!hasAxisContacts(mm)) return false;
    if (!findCenter(mm).has_value()) return false;
    return true;
}

// ===========================================================================
// Bridge finder (Algorithm 1)
// ===========================================================================

static CellSet possibleBridges(const CellSet& Cpiv, const CellSet& Cblo) {
    // Heuristic (cheap) version of paper Algorithm 1:
    // a candidate bridge is a free cell that touches at least TWO cells of
    // Cpiv (so it can act as a pivot extension) and is not inside Cblo.
    // The Transfer-DFS afterwards filters candidates that actually help.
    CellSet result;
    for (const auto& p : Cpiv) {
        for (const auto& dp : fccNeighbors(p)) {
            Cell3DPosition n(p[0]+dp[0], p[1]+dp[1], p[2]+dp[2]);
            if (Cblo.count(n) || Cpiv.count(n)) continue;
            // count Cpiv neighbors of n
            int touch = 0;
            for (const auto& dq : fccNeighbors(n)) {
                Cell3DPosition m(n[0]+dq[0], n[1]+dq[1], n[2]+dq[2]);
                if (Cpiv.count(m)) {
                    if (++touch >= 2) break;
                }
            }
            if (touch >= 2) result.insert(n);
        }
    }
    return result;
}

// ===========================================================================
// Transfer planner (paper §5.2)
// ===========================================================================

TransferPlan planTransfer(const MetaModule& mm, const Cell3DPosition& V) {
    TransferPlan plan;

    Cell3DPosition center = mm.center;
    Cell3DPosition target(center[0]+V[0], center[1]+V[1], center[2]+V[2]);

    // Cpiv = M ∪ (M + V)
    CellSet Cpiv = mm.cells;
    for (const auto& p : mm.cells) {
        Cpiv.insert(Cell3DPosition(p[0]+V[0], p[1]+V[1], p[2]+V[2]));
    }
    // Cblo = union of all neighboring meta-modules (worst case: all filled)
    MetaModule m2 = mm;
    Cell3DPosition zero(0,0,0);
    CellSet Cblo = tiledNeighborhood(m2, &zero);
    // remove anything that overlaps Cpiv (Cpiv has the source + neighbor along V)
    for (const auto& p : Cpiv) Cblo.erase(p);

    // First: can a single module already transfer from center to target with just Cpiv?
    {
        CellSet env = Cpiv;
        env.insert(center);
        // Worst case: surrounding meta-modules also full
        for (const auto& q : Cblo) env.insert(q);
        int d = pathLength(center, target, env);
        if (d > 0) {
            plan.feasible = true;
            plan.nBridges = 0;
            plan.nSetupBridgeMoves = 0;
            plan.nTransferMovesPerModule = d;
            plan.nClearBridgeMoves = 0;
            return plan;
        }
    }

    // Otherwise try bridges
    CellSet bridgeCandidates = possibleBridges(Cpiv, Cblo);

    // Greedy DFS: try subsets of bridge candidates up to size kMax.
    // kMax is intentionally small — most useful meta-modules need 0-2 bridges.
    const int kMax = std::min<int>(2, static_cast<int>(bridgeCandidates.size()));
    // Limit raw bridge candidates to the closest 12 to center.
    std::vector<Cell3DPosition> cands(bridgeCandidates.begin(), bridgeCandidates.end());
    if (cands.size() > 12) {
        std::sort(cands.begin(), cands.end(),
                  [&](const Cell3DPosition& a, const Cell3DPosition& b) {
                      return a.dist_taxi(center) < b.dist_taxi(center);
                  });
        cands.resize(12);
    }

    std::vector<int> idx;
    std::function<bool(int)> dfs = [&](int start) -> bool {
        // build environment with chosen bridges
        CellSet env = Cpiv;
        for (const auto& q : Cblo) env.insert(q);
        for (int i : idx) env.insert(cands[i]);

        CellSet envWithCenter = env;
        envWithCenter.insert(center);
        int d = pathLength(center, target, envWithCenter);
        if (d > 0) {
            // Setup cost: sum of pathLength(center, bridge_i, partial_env)
            int setupMoves = 0;
            CellSet running = Cpiv;
            for (const auto& q : Cblo) running.insert(q);
            running.insert(center);
            for (int i : idx) {
                int s = pathLength(center, cands[i], running);
                if (s < 0) return false;
                setupMoves += s;
                running.insert(cands[i]);
            }
            // Clear cost: each bridge plus the last support module move to target.
            // Approximation: shortest path from bridge to target inside env.
            int clearMoves = 0;
            for (auto it = idx.rbegin(); it != idx.rend(); ++it) {
                CellSet clearEnv = envWithCenter;
                for (int j : idx) clearEnv.insert(cands[j]);
                int s = pathLength(cands[*it], target, clearEnv);
                if (s < 0) return false;
                clearMoves += s;
            }
            plan.feasible = true;
            plan.nBridges = static_cast<int>(idx.size());
            plan.nSetupBridgeMoves = setupMoves;
            plan.nTransferMovesPerModule = d;
            plan.nClearBridgeMoves = clearMoves;
            return true;
        }
        if (static_cast<int>(idx.size()) >= kMax) return false;
        for (int i = start; i < static_cast<int>(cands.size()); ++i) {
            idx.push_back(i);
            if (dfs(i + 1)) return true;
            idx.pop_back();
        }
        return false;
    };
    dfs(0);
    return plan;
}

AllDirectionsCost evaluateMetaModule(const MetaModule& mm) {
    AllDirectionsCost out;
    std::array<Cell3DPosition, 6> dirs = {
        mm.basis.X,
        Cell3DPosition(-mm.basis.X[0], -mm.basis.X[1], -mm.basis.X[2]),
        mm.basis.Y,
        Cell3DPosition(-mm.basis.Y[0], -mm.basis.Y[1], -mm.basis.Y[2]),
        mm.basis.Z,
        Cell3DPosition(-mm.basis.Z[0], -mm.basis.Z[1], -mm.basis.Z[2]),
    };
    for (int i = 0; i < 6; ++i) {
        out.perDirection[i] = planTransfer(mm, dirs[i]);
        if (!out.perDirection[i].feasible) {
            out.feasible = false;
            out.totalTransferMoves = INT_MAX;
            return out;
        }
        out.totalTransferMoves += out.perDirection[i].totalMoves(1);
    }
    return out;
}

// ===========================================================================
// GA
// ===========================================================================

namespace {

struct Indiv {
    CellSet cells;
    double fitness{-1e18};
    AllDirectionsCost cost{};
    Cell3DPosition center{0,0,0};
    bool evaluated{false};
};

std::vector<Cell3DPosition> boxCells(int R) {
    std::vector<Cell3DPosition> out;
    for (int x = -R; x <= R; ++x)
        for (int y = -R; y <= R; ++y)
            for (int z = -R; z <= R; ++z)
                out.emplace_back(x, y, z);
    return out;
}

CellSet randomConnectedShape(int size, int R, std::mt19937& rng) {
    CellSet out;
    Cell3DPosition seed(0, 0, 0);
    out.insert(seed);
    while ((int)out.size() < size) {
        std::vector<Cell3DPosition> frontier;
        for (const auto& p : out) {
            for (const auto& dp : fccNeighbors(p)) {
                Cell3DPosition n(p[0]+dp[0], p[1]+dp[1], p[2]+dp[2]);
                if (std::abs(n[0]) > R || std::abs(n[1]) > R || std::abs(n[2]) > R) continue;
                if (!out.count(n)) frontier.push_back(n);
            }
        }
        if (frontier.empty()) break;
        std::uniform_int_distribution<size_t> pick(0, frontier.size() - 1);
        out.insert(frontier[pick(rng)]);
    }
    return out;
}

void mutate(CellSet& shape, int R, std::mt19937& rng) {
    if (shape.empty()) return;
    std::bernoulli_distribution coin(0.5);
    if (coin(rng) && shape.size() > 4) {
        // remove a non-articulation cell
        std::vector<Cell3DPosition> v(shape.begin(), shape.end());
        std::shuffle(v.begin(), v.end(), rng);
        for (const auto& c : v) {
            CellSet tmp = shape;
            tmp.erase(c);
            if (isConnected(tmp)) { shape = std::move(tmp); return; }
        }
    } else {
        // grow at a random frontier cell
        std::vector<Cell3DPosition> frontier;
        for (const auto& p : shape) {
            for (const auto& dp : fccNeighbors(p)) {
                Cell3DPosition n(p[0]+dp[0], p[1]+dp[1], p[2]+dp[2]);
                if (std::abs(n[0]) > R || std::abs(n[1]) > R || std::abs(n[2]) > R) continue;
                if (!shape.count(n)) frontier.push_back(n);
            }
        }
        if (!frontier.empty()) {
            std::uniform_int_distribution<size_t> pick(0, frontier.size() - 1);
            shape.insert(frontier[pick(rng)]);
        }
    }
}

CellSet crossover(const CellSet& a, const CellSet& b, int targetSize,
                  int R, std::mt19937& rng) {
    // Union, then prune to targetSize keeping connectivity to a seed of a∩b
    CellSet u = a;
    for (const auto& c : b) u.insert(c);
    if ((int)u.size() <= targetSize) return u;
    // BFS from a random cell in a, accept up to targetSize
    std::vector<Cell3DPosition> va(a.begin(), a.end());
    std::uniform_int_distribution<size_t> pick(0, va.size() - 1);
    Cell3DPosition seed = va[pick(rng)];
    CellSet out; out.insert(seed);
    std::queue<Cell3DPosition> q; q.push(seed);
    while (!q.empty() && (int)out.size() < targetSize) {
        Cell3DPosition cur = q.front(); q.pop();
        std::vector<Cell3DPosition> nb;
        for (const auto& dp : fccNeighbors(cur)) {
            Cell3DPosition n(cur[0]+dp[0], cur[1]+dp[1], cur[2]+dp[2]);
            if (u.count(n) && !out.count(n)
                && std::abs(n[0]) <= R && std::abs(n[1]) <= R && std::abs(n[2]) <= R) {
                nb.push_back(n);
            }
        }
        std::shuffle(nb.begin(), nb.end(), rng);
        for (const auto& n : nb) {
            if ((int)out.size() >= targetSize) break;
            out.insert(n);
            q.push(n);
        }
    }
    return out;
}

double scoreFitness(const Indiv& ind, int targetSize) {
    if (!ind.cost.feasible) {
        // Soft penalty so infeasibles still drive evolution
        return -1e9 - static_cast<double>(ind.cells.size());
    }
    // size-first: huge weight on |M|, then transfer cost
    return -(1e6 * static_cast<double>(ind.cells.size())
             + static_cast<double>(ind.cost.totalTransferMoves));
}

void evaluate(Indiv& ind, const Basis& basis, int targetSize) {
    if (ind.evaluated) return;
    ind.evaluated = true;
    if ((int)ind.cells.size() != targetSize || !isConnected(ind.cells)) {
        ind.cost.feasible = false;
        ind.fitness = -1e10;
        return;
    }
    MetaModule mm{ind.cells, basis, Cell3DPosition(0,0,0)};
    if (!mm.basis.linearlyIndependent()) {
        ind.cost.feasible = false;
        ind.fitness = -1e10;
        return;
    }
    if (!isExclusive(mm) || !hasAxisContacts(mm)) {
        ind.cost.feasible = false;
        ind.fitness = scoreFitness(ind, targetSize);
        return;
    }
    auto ctr = findCenter(mm);
    if (!ctr.has_value()) {
        ind.cost.feasible = false;
        ind.fitness = scoreFitness(ind, targetSize);
        return;
    }
    mm.center = *ctr;
    ind.center = *ctr;
    ind.cost = evaluateMetaModule(mm);
    ind.fitness = scoreFitness(ind, targetSize);
}

GenerationResult runGAForBasisSize(const Basis& basis,
                                   const std::string& label,
                                   int targetSize,
                                   int R,
                                   const GAConfig& cfg,
                                   std::mt19937& rng) {
    GenerationResult out;
    out.basisLabel = label;

    std::vector<Indiv> pop(cfg.populationSize);
    for (auto& ind : pop) {
        ind.cells = randomConnectedShape(targetSize, R, rng);
        evaluate(ind, basis, targetSize);
    }

    auto tournament = [&](void) -> const Indiv& {
        std::uniform_int_distribution<int> pick(0, cfg.populationSize - 1);
        const Indiv* best = &pop[pick(rng)];
        for (int i = 1; i < cfg.tournamentK; ++i) {
            const Indiv& c = pop[pick(rng)];
            if (c.fitness > best->fitness) best = &c;
        }
        return *best;
    };

    Indiv best = *std::max_element(pop.begin(), pop.end(),
        [](const Indiv& a, const Indiv& b){ return a.fitness < b.fitness; });

    for (int gen = 0; gen < cfg.generations; ++gen) {
        std::vector<Indiv> next;
        next.reserve(cfg.populationSize);
        // elitism
        next.push_back(best);
        std::uniform_real_distribution<double> u(0.0, 1.0);
        while ((int)next.size() < cfg.populationSize) {
            const Indiv& a = tournament();
            const Indiv& b = tournament();
            Indiv child;
            if (u(rng) < cfg.crossoverRate) {
                child.cells = crossover(a.cells, b.cells, targetSize, R, rng);
            } else {
                child.cells = a.cells;
            }
            if (u(rng) < cfg.mutationRate) {
                mutate(child.cells, R, rng);
            }
            // repair size: if too few cells, grow; if too many, trim
            while ((int)child.cells.size() < targetSize) {
                mutate(child.cells, R, rng);
                if ((int)child.cells.size() > targetSize) break;
                // safety net: ensure progress
                if (!isConnected(child.cells)) break;
            }
            while ((int)child.cells.size() > targetSize) {
                std::vector<Cell3DPosition> v(child.cells.begin(), child.cells.end());
                std::shuffle(v.begin(), v.end(), rng);
                bool removed = false;
                for (const auto& c : v) {
                    CellSet tmp = child.cells;
                    tmp.erase(c);
                    if (isConnected(tmp)) { child.cells = std::move(tmp); removed = true; break; }
                }
                if (!removed) break;
            }
            if ((int)child.cells.size() != targetSize) continue;
            evaluate(child, basis, targetSize);
            next.push_back(std::move(child));
        }
        pop = std::move(next);
        for (const auto& ind : pop) {
            if (ind.fitness > best.fitness) best = ind;
        }
        if (cfg.verbose && (gen % 25 == 0 || gen == cfg.generations - 1)) {
            std::cout << "  [" << label << " |M|=" << targetSize
                      << "] gen " << gen
                      << " best fitness " << best.fitness
                      << (best.cost.feasible
                          ? " (feasible, transfer=" + std::to_string(best.cost.totalTransferMoves) + ")"
                          : " (infeasible)")
                      << std::endl;
        }
        if (best.cost.feasible) break; // size-first: any feasible at this size wins
    }

    if (best.cost.feasible) {
        out.found = true;
        out.best.cells = best.cells;
        out.best.basis = basis;
        out.best.center = best.center;
        out.cost = best.cost;
    }
    return out;
}

} // namespace

GenerationResult runGenerator(const std::vector<CandidateBasis>& bases,
                              const GAConfig& cfg) {
    std::mt19937 rng(cfg.seed);
    GenerationResult best;
    best.found = false;

    for (int size = cfg.minSize; size <= cfg.maxSize; ++size) {
        if (cfg.verbose) std::cout << "=== size |M| = " << size << " ===\n";
        for (const auto& cb : bases) {
            auto r = runGAForBasisSize(cb.basis, cb.label, size,
                                       cfg.boundingRadius, cfg, rng);
            if (r.found) {
                if (!best.found
                    || (int)r.best.cells.size() < (int)best.best.cells.size()
                    || ((int)r.best.cells.size() == (int)best.best.cells.size()
                        && r.cost.totalTransferMoves < best.cost.totalTransferMoves)) {
                    best = r;
                }
            }
        }
        if (best.found) return best; // smallest size that admits any solution
    }
    return best;
}

std::vector<CandidateBasis> defaultCandidateBases() {
    auto V = [](short x, short y, short z) { return Cell3DPosition(x,y,z); };
    return {
        {{V(2,0,0), V(0,2,0), V(0,0,2)}, "cubic-2"},
        {{V(3,0,0), V(0,3,0), V(0,0,2)}, "ortho-3-3-2"},
        {{V(4,0,0), V(0,4,0), V(0,0,2)}, "ortho-4-4-2"},
        {{V(4,0,0), V(0,4,0), V(0,0,4)}, "cubic-4"},
        {{V(6,0,0), V(0,6,0), V(0,0,2)}, "scaffold-flat"},
        {{V(8,0,0), V(4,3,0), V(4,1,4)}, "hexagonal (paper)"},
    };
}

std::ostream& operator<<(std::ostream& os, const MetaModule& mm) {
    os << "MetaModule { size=" << mm.size()
       << " basis=( X=" << mm.basis.X
       << ", Y=" << mm.basis.Y
       << ", Z=" << mm.basis.Z
       << ") center=" << mm.center
       << " cells={";
    bool first = true;
    for (const auto& c : mm.cells) {
        if (!first) os << ", ";
        os << c;
        first = false;
    }
    os << "} }";
    return os;
}

} // namespace metaModuleGen

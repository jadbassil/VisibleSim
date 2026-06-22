#pragma once
#include <Eigen/Dense>
#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "math/cell3DPosition.h"
#include "utils/tDefs.h"

namespace GNNShapeReconfiguration {

constexpr int NODE_DIM   = 16;
constexpr int EDGE_DIM   = 6;
constexpr int HIDDEN     = 64;
constexpr int N_ACTIONS  = 13;
constexpr int N_LAYERS   = 3;

struct GCNLayer {
    Eigen::MatrixXf msgW;    // [HIDDEN, in + EDGE_DIM]
    Eigen::MatrixXf selfW;   // [HIDDEN, in]
    Eigen::VectorXf bias;    // [HIDDEN]
};

struct GCNWeights {
    GCNLayer        layers[N_LAYERS];
    Eigen::MatrixXf actor_fc1_W;  // [HIDDEN, HIDDEN]
    Eigen::VectorXf actor_fc1_b;  // [HIDDEN]
    Eigen::MatrixXf actor_fc2_W;  // [N_ACTIONS, HIDDEN]
    Eigen::VectorXf actor_fc2_b;  // [N_ACTIONS]
    bool loaded = false;

    static GCNWeights& instance();
    void loadFromFile(const std::string& path);  // throws on bad format
};

// L1 distance from `pos` to the nearest UNFILLED target cell, also writing the
// L1-normalised direction (3-vector) towards it into `dirNorm`.
//   targetCells: (cell, filled) pairs; filled==true means a block occupies it.
// Returns 0 and dirNorm=(0,0,0) when there are no unfilled targets.
// Unfilled cells are visited in sorted (x,y,z) order so the argmin tie-break
// matches Python's sorted(unfilled_targets) (env.py).
float nearestUnfilledTarget(const Cell3DPosition& pos,
    Eigen::VectorXf& dirNorm,
    const std::vector<std::pair<Cell3DPosition,bool>>& targetCells);

// 6-d one-hot direction vector. `direction` is an SCLattice2::Direction index.
Eigen::VectorXf edgeFeature(int direction);

// Build the 16-d node feature vector for this module.
//   neighborMask6: bit d set iff direction d has a connected neighbour.
//   distToTarget : raw L1 distance to nearest unfilled target (normalised here).
//   dirNorm      : L1-normalised direction (3-vector) to that target.
Eigen::VectorXf buildNodeFeature(const Cell3DPosition& gridSize,
                                 const Cell3DPosition& pos,
                                 bool inTarget,
                                 uint8_t neighborMask6,
                                 int nMoves,
                                 bool isAP,
                                 float distToTarget,
                                 const Eigen::VectorXf& dirNorm);

// Compute the message a node sends to a neighbour given its hidden state and
// the edge feature for that neighbour.
Eigen::VectorXf gcnEmitMsg(const GCNLayer& L,
                           const Eigen::VectorXf& selfH,
                           const Eigen::VectorXf& edgeFeat);

// Single-layer forward at a node. `incomingMsgs[k]` is msg_lin([h_j ‖ e_ji])
// already computed by the sender. Returns ELU(mean(msgs) + selfW*selfH + bias).
// If incomingMsgs is empty (isolated node) the aggregated term is zero.
Eigen::VectorXf gcnLayerForward(const GCNLayer& L,
                                const Eigen::VectorXf& selfH,
                                const std::vector<Eigen::VectorXf>& incomingMsgs);

// Apply actor head and return logits before masking.
Eigen::VectorXf actorLogits(const GCNWeights& W, const Eigen::VectorXf& finalH);

// Sample an action given logits and a boolean mask. `mask[i]==false` => masked out.
// Uses xorshift64* on `rngState` for reproducibility.
int sampleMaskedSoftmax(const Eigen::VectorXf& logits,
                        const std::array<bool, N_ACTIONS>& mask,
                        uint64_t& rngState);

// Argmax variant (deterministic deployment).
int argmaxMasked(const Eigen::VectorXf& logits,
                 const std::array<bool, N_ACTIONS>& mask);

} // namespace GNNShapeReconfiguration

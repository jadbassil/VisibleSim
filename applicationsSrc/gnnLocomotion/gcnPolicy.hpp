#pragma once
#include <Eigen/Dense>
#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "math/cell3DPosition.h"
#include "utils/tDefs.h"

namespace GNNLocomotion {

constexpr int NODE_DIM   = 12;
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

// 6-d one-hot direction vector. `direction` is an SCLattice2::Direction index.
Eigen::VectorXf edgeFeature(int direction);

// Build the 12-d node feature vector for this module.
//   neighborMask6: bit d set iff direction d has a connected neighbour.
Eigen::VectorXf buildNodeFeature(const Cell3DPosition& gridSize,
                                 const Cell3DPosition& pos,
                                 bool inTarget,
                                 uint8_t neighborMask6,
                                 int nMoves,
                                 bool isAP);

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

} // namespace GNNLocomotion

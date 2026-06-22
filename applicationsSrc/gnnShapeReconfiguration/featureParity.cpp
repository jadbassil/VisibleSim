// Standalone feature-parity dumper.
//
// test_export_parity.py checks the forward-pass math against random features;
// it does NOT exercise feature *construction*. This program computes the C++
// node-feature vectors (via the real nearestUnfilledTarget + buildNodeFeature)
// for a handful of hand-built scenarios and prints them as JSON lines, so that
// train/test_feature_parity.py can compare them against env.py's obs_to_tensors
// on the identical scenarios.
//
// Build:  make feature-parity
// Run:    ../../applicationsBin/gnnShapeReconfiguration/featureParity
//
// Scenarios use <= 2 blocks so the articulation-point set is empty on both
// sides (matching env._articulation_points, which returns {} for <= 2 blocks).

#include <iostream>
#include <utility>
#include <vector>

#include "gcnPolicy.hpp"
#include "math/cell3DPosition.h"

using namespace GNNShapeReconfiguration;

namespace {

struct Block {
    const char*    name;
    Cell3DPosition pos;
    bool           inTarget;
    uint8_t        mask6;    // bit d set iff neighbour in direction d
    int            nMoves;
};

struct Scenario {
    int                                            id;
    Cell3DPosition                                 gridSize;
    std::vector<std::pair<Cell3DPosition, bool>>   targetOcc;  // (cell, filled)
    std::vector<Block>                             blocks;
};

void dumpBlock(int scenarioId, const Scenario& s, const Block& b) {
    Eigen::VectorXf dirNorm;
    float dist = nearestUnfilledTarget(b.pos, dirNorm, s.targetOcc);
    Eigen::VectorXf x = buildNodeFeature(s.gridSize, b.pos, b.inTarget,
                                         b.mask6, b.nMoves, /*isAP=*/false,
                                         dist, dirNorm);
    std::cout << "{\"scenario\":" << scenarioId
              << ",\"block\":\"" << b.name << "\",\"x\":[";
    for (int i = 0; i < x.size(); i++) {
        if (i) std::cout << ",";
        std::cout << x(i);
    }
    std::cout << "]}\n";
}

}  // namespace

int main() {
    std::cout.precision(9);

    std::vector<Scenario> scenarios = {
        // 1: tie-break test. Unfilled {(2,0,0),(3,0,0),(0,2,0)}; (1,0,0) filled.
        //    Block A at origin is equidistant (L1=2) to (0,2,0) and (2,0,0);
        //    sorted order must pick (0,2,0) -> direction (0,1,0).
        {1, Cell3DPosition(6, 6, 6),
         {{Cell3DPosition(1, 0, 0), true},
          {Cell3DPosition(2, 0, 0), false},
          {Cell3DPosition(3, 0, 0), false},
          {Cell3DPosition(0, 2, 0), false}},
         {{"A", Cell3DPosition(0, 0, 0), false, 0, 2},
          {"B", Cell3DPosition(1, 0, 0), true, 0, 0}}},

        // 2: multi-axis direction. Single unfilled target far on the diagonal.
        {2, Cell3DPosition(4, 4, 4),
         {{Cell3DPosition(3, 3, 3), false}},
         {{"A", Cell3DPosition(0, 0, 0), false, 0, 1}}},

        // 3: no unfilled targets -> dist 0, direction (0,0,0).
        {3, Cell3DPosition(5, 5, 5),
         {{Cell3DPosition(0, 0, 0), true}},
         {{"A", Cell3DPosition(0, 0, 0), true, 0, 0}}},
    };

    for (const auto& s : scenarios)
        for (const auto& b : s.blocks)
            dumpBlock(s.id, s, b);

    return 0;
}

#include "robots/slidingCubes/slidingCubesSimulator.h"
#include "robots/slidingCubes/slidingCubesBlockCode.h"
#include "gnnShapeReconfigurationBlockCode.hpp"
#include <iostream>
#include <exception>

int main(int argc, char** argv) {
    try {
        SlidingCubes::createSimulator(argc, argv,
            GNNShapeReconfiguration::GNNShapeReconfigurationCode::buildNewBlockCode);
        SlidingCubes::getSimulator()->printInfo();
        BaseSimulator::getWorld()->printInfo();
        BaseSimulator::deleteSimulator();
    } catch (const std::exception& e) {
        std::cerr << "[gnnShapeReconfiguration] Uncaught exception: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

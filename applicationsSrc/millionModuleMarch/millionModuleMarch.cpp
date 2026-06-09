/**
 * @file   millionModuleMarch.cpp
 * @brief  Entry point for the Million Module March locomotion application.
 *
 * Implements the distributed locomotion algorithm of Fitch & Butler,
 * "Million Module March: Scalable Locomotion for Large Self-Reconfiguring
 * Robots" (IJRR 2008), adapted to VisibleSim 3D catoms.
 */

#include <iostream>

#include "robots/catoms3D/catoms3DSimulator.h"
#include "robots/catoms3D/catoms3DBlockCode.h"

#include "millionModuleMarchBlockCode.hpp"

using namespace std;
using namespace Catoms3D;

int main(int argc, char **argv) {
    try {
        createSimulator(argc, argv, MillionModuleMarchCode::buildNewBlockCode);
        getSimulator()->printInfo();
        BaseSimulator::getWorld()->printInfo();
        deleteSimulator();
    } catch (std::exception const &e) {
        cerr << "Uncaught exception: " << e.what();
    }

    return 0;
}

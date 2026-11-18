#include <iostream>
#include "AI4PMCode_Embedded3D.hpp"

using namespace std;
using namespace BlinkyBlocks;

int main(int argc, char **argv)
{
    try
    {
        createSimulator(argc, argv, AI4PMCode_Embedded3D::buildNewBlockCode);
        getSimulator()->printInfo();
        BaseSimulator::getWorld()->printInfo();
        BaseSimulator::getScheduler()->start(SCHEDULER_MODE_FASTEST);
        BaseSimulator::getScheduler()->waitForSchedulerEnd();

        deleteSimulator();
    }
    catch (std::exception const &e)
    {
        cerr << "Uncaught exception: " << e.what();
    }

    return 0;
}

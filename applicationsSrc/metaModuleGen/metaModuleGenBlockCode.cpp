#include "metaModuleGenBlockCode.h"

#include <iostream>

void MetaModuleGenBlockCode::startup() {
    // Bootstrap block is purely a render anchor; the GA runs in main().
    hostBlock->setColor(BLUE);
    std::cout << "[MetaModuleGen] bootstrap catom at " << hostBlock->position
              << " — GA results are written to stdout above this line."
              << std::endl;
}

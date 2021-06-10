#ifndef INIT_HPP__
#define INIT_HPP__

#include "metaModuleBlockCode.hpp"

class Init {
private:
    /* data */
    
public:
    Init(/* args */);
    ~Init();
    static bool initialMapBuildDone;
    static void buildMM(Catoms3DBlock *s, MMShape shape, Color col);
    static void buildMMon(Catoms3DBlock *s, Direction pos);
    static void buildInitialMap(Cell3DPosition firstSeedPos);
    static void getNeighborMMSeedPos(const Cell3DPosition &seedPos, const Cell3DPosition &MMPos, Direction pos, Cell3DPosition &newSeed);
    static void fillMM(Catoms3DBlock *s);
};

#endif

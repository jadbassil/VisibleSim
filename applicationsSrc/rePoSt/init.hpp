#ifndef INIT_HPP__
#define INIT_HPP__

#include "rePoStBlockCode.hpp"

class Init {
private:
    /* data */
    
public:
    static map<Cell3DPosition, map<Cell3DPosition, int>> distanceMatrix;

    Init(/* args */);
    ~Init();
    static bool initialMapBuildDone;
    static void buildMM(Catoms3DBlock *s, MMShape shape, Color col);
    static void buildMMon(Catoms3DBlock *s, Direction pos);
    static void buildInitialMap(Cell3DPosition firstSeedPos, vector<array<int,4>> &map);
    static void getNeighborMMSeedPos(const Cell3DPosition &seedPos, const Cell3DPosition &MMPos, Direction pos, Cell3DPosition &newSeed);
    static void fillMM(Catoms3DBlock *s);
    static Cell3DPosition getSeedPositionFromMMPosition(Cell3DPosition &MMPos);
    static void buildDistanceMatrix(bool rebuild = false);
    static int getDistance(Cell3DPosition p1, Cell3DPosition p2);
    void readDistanceMapFromFile();
};

#endif

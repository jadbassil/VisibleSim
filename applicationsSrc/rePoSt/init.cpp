#include "init.hpp"
#include <ctime>
#include <fstream>

Init::Init(/* args */) {
}

Init::~Init() {
}

bool Init::initialMapBuildDone = false;

void Init::buildInitialMap(Cell3DPosition firstSeedPos, vector<array<int, 4>> &map) {
    if (initialMapBuildDone) {
        cerr << "Initial map already built!!" << endl;
        return;
    }
    cerr << "Number of Meta-Modules: " << map.size() << endl;
    srand(2);
    for (int i = 1; i < map.size(); i++) {
        int x, y, z;
        x = firstSeedPos.pt[0] + 4 * map[i][0];
        y = firstSeedPos.pt[1] + 3*map[i][1];
        z = firstSeedPos.pt[2] + 4*map[i][2];
        if(map[i][2] % 2 != 0) {
            y -= 1;
        }
        int col = rand() % NB_COLORS;
        BaseSimulator::getWorld()->addBlock(0, RePoStBlockCode::buildNewBlockCode, Cell3DPosition(x,y,z),  Colors[col] );

        RePoStBlockCode *newSeed = static_cast<RePoStBlockCode*>(
            BaseSimulator::getWorld()->getBlockByPosition(Cell3DPosition(x,y,z))->blockCode
        );
   
      
        newSeed->MMPosition = Cell3DPosition(map[i][0],map[i][1],map[i][2]);
       // if(newSeed->MMPosition.pt[2] % 2 == 0)
            if( (newSeed->MMPosition.pt[0]+newSeed->MMPosition.pt[1]) % 2 == 0)
                newSeed->shapeState = FRONTBACK;
            else
                newSeed->shapeState = BACKFRONT;
        // else
        //     if( (newSeed->MMPosition.pt[0]+newSeed->MMPosition.pt[1]) % 2 == 0)
        //         newSeed->shapeState = BACKFRONT;
        //     else
        //         newSeed->shapeState = FRONTBACK;
        newSeed->seedPosition = newSeed->module->position;
             vector<Color> neighborsColors;
        for(auto neighSeedPos: newSeed->getAdjacentMMSeeds()) {
            RePoStBlockCode *neighbSeed = static_cast<RePoStBlockCode*>(
                BaseSimulator::getWorld()->getBlockByPosition(neighSeedPos)->blockCode
            );
            cerr << neighSeedPos << endl;
            neighborsColors.push_back(neighbSeed->module->color);
        }
        while(isIn(neighborsColors, newSeed->module->color)) {
            col = rand() % NB_COLORS;
            newSeed->module->setColor( Colors[col]);
        }
        buildMM(newSeed->module, newSeed->shapeState, Colors[col]);
        if(map[i][3] == 1)
            fillMM(newSeed->module);
    }
    // buildDistanceMatrix();
    initialMapBuildDone = true;
}

void Init::buildMM(Catoms3DBlock *s, MMShape shape, Color col) {
    Cell3DPosition seed_position = s->position;
    RePoStBlockCode* seedModule = static_cast<RePoStBlockCode*>(
        BaseSimulator::getWorld()->getBlockByPosition(seed_position)->blockCode);

    for(int i=1; i<FrontBackMM.size(); i++) {
        Cell3DPosition newBlock_position; 
        for(int j=0; j<3; j++) {
            if(shape == FRONTBACK)
                newBlock_position.pt[j] = seed_position.pt[j] + FrontBackMM[i].pt[j];
            else  
                newBlock_position.pt[j] = seed_position.pt[j] + BackFrontMM[i].pt[j];
        }
        //cout << newBlock_position << endl;
        BaseSimulator::getWorld()->addBlock(0, RePoStBlockCode::buildNewBlockCode, newBlock_position, col);
        RePoStBlockCode* newBlock = static_cast<RePoStBlockCode*>(
            BaseSimulator::getWorld()->getBlockByPosition(newBlock_position)->blockCode);

        newBlock->MMPosition = seedModule->MMPosition;
        newBlock->seedPosition = seedModule->module->position;
        newBlock->initialPosition = newBlock->module->position - newBlock->seedPosition;
        newBlock->shapeState = shape;
        newBlock->parentPosition = newBlock->MMPosition;
    }
}

void Init::buildMMon(Catoms3DBlock *s, Direction pos) {

    Cell3DPosition neighborSeedPos;
     RePoStBlockCode *block = static_cast<RePoStBlockCode*>(s->blockCode);
    getNeighborMMSeedPos(s->position, block->MMPosition, pos, neighborSeedPos);
    BaseSimulator::getWorld()->addBlock(0, seed->buildNewBlockCode, neighborSeedPos, Color(RED));
    RePoStBlockCode* neighborSeed = static_cast<RePoStBlockCode*>(BaseSimulator::getWorld()->getBlockByPosition(neighborSeedPos)->blockCode);
   
    if(block->getShapeState() == FRONTBACK) {
        neighborSeed->setShapeState(BACKFRONT);
    } else {
        neighborSeed->setShapeState(FRONTBACK);
    }
    switch (pos) {
    case Direction::UP:{
        neighborSeed->MMPosition = block->MMPosition.offsetZ(1);
        break;
    }
    case Direction::RIGHT: {
        neighborSeed->MMPosition = block->MMPosition.offsetX(1);
        break;
    }
    case Direction::DOWN: {
        neighborSeed->MMPosition = block->MMPosition.offsetZ(-1);
        break;
    }
    case Direction::LEFT: {
        neighborSeed->MMPosition = block->MMPosition.offsetX(-1);
        break;
    }
    case Direction::FRONT: {
        neighborSeed->MMPosition = block->MMPosition.offsetY(-1);
        break;
    }
    case Direction::BACK: {
        neighborSeed->MMPosition = block->MMPosition.offsetY(1);
        break;
    }
    default:
        break;
    }
    buildMM(neighborSeed->module, neighborSeed->getShapeState(), Color(RED));
}

void Init::fillMM(Catoms3DBlock *s) {
    RePoStBlockCode *S = static_cast<RePoStBlockCode*>(BaseSimulator::getWorld()->getBlockByPosition(s->position)->blockCode);
    if (S->MMPosition.pt[2] % 2 == 0) { //Z_even
        if (S->shapeState == FRONTBACK) {
            for (int i = 0; i < 10; i++) {
                Cell3DPosition newBlockPos =
                    s->position + Cell3DPosition(FillingPositions_FrontBack_Zeven[i]);
                BaseSimulator::getWorld()->addBlock(0, RePoStBlockCode::buildNewBlockCode,
                                                    newBlockPos, Color(LIGHTBLUE));
                RePoStBlockCode *newBlock = static_cast<RePoStBlockCode *>(
                    BaseSimulator::getWorld()->getBlockByPosition(newBlockPos)->blockCode);
                newBlock->seedPosition = s->position;
                newBlock->MMPosition = S->MMPosition;
            }
        } else if (S->shapeState == BACKFRONT) {
            for (int i = 0; i < 10; i++) {
                Cell3DPosition newBlockPos =
                    s->position + Cell3DPosition(FillingPositions_BackFront_Zeven[i]);
                BaseSimulator::getWorld()->addBlock(0, RePoStBlockCode::buildNewBlockCode,
                                                    newBlockPos, Color(LIGHTBLUE));
                RePoStBlockCode *newBlock = static_cast<RePoStBlockCode *>(
                    BaseSimulator::getWorld()->getBlockByPosition(newBlockPos)->blockCode);
                newBlock->seedPosition = s->position;
                newBlock->MMPosition = S->MMPosition;
            }
        }
    } else {  // Z_odd
        if (S->shapeState == FRONTBACK) {
            for (int i = 0; i < 10; i++) {
                Cell3DPosition newBlockPos =
                    s->position + Cell3DPosition(FillingPositions_FrontBack_Zodd[i]);
                BaseSimulator::getWorld()->addBlock(0, RePoStBlockCode::buildNewBlockCode,
                                                    newBlockPos, Color(LIGHTBLUE));
                RePoStBlockCode *newBlock = static_cast<RePoStBlockCode *>(
                    BaseSimulator::getWorld()->getBlockByPosition(newBlockPos)->blockCode);
                newBlock->seedPosition = s->position;
                newBlock->MMPosition = S->MMPosition;
            }
        } else if (S->shapeState == BACKFRONT) {
            for (int i = 0; i < 10; i++) {
                Cell3DPosition newBlockPos =
                    s->position + Cell3DPosition(FillingPositions_BackFront_Zodd[i]);
                BaseSimulator::getWorld()->addBlock(0, RePoStBlockCode::buildNewBlockCode,
                                                    newBlockPos, Color(LIGHTBLUE));
                RePoStBlockCode *newBlock = static_cast<RePoStBlockCode *>(
                    BaseSimulator::getWorld()->getBlockByPosition(newBlockPos)->blockCode);
                newBlock->seedPosition = s->position;
                newBlock->MMPosition = S->MMPosition;
            }
        }
    }
}

void Init::getNeighborMMSeedPos(const Cell3DPosition &seedPos, const Cell3DPosition &MMPos, Direction pos, Cell3DPosition &newSeed) {
    Cell3DPosition neighbor_position = seedPos;
   
    switch (pos) {
    case Direction::UP:{
        // RePoStBlockCode* seedBlockCode =  static_cast<RePoStBlockCode*>(
        //     BaseSimulator::getWorld()->getBlockByPosition(seedPos)->blockCode
        // );
        (MMPos.pt[2] %2 == 0) 
            ? neighbor_position = neighbor_position.offsetY(-1): neighbor_position = neighbor_position.offsetY(1);
        neighbor_position = neighbor_position.offsetZ(4);
        break;
    }
    case Direction::RIGHT: {
        neighbor_position = neighbor_position.offsetX(4);
        break;
    }
    case Direction::DOWN: {
        //  RePoStBlockCode* seedBlockCode =  static_cast<RePoStBlockCode*>(
        //     BaseSimulator::getWorld()->getBlockByPosition(seedPos)->blockCode
        // );
         (MMPos.pt[2] %2 == 0) 
                ? neighbor_position = neighbor_position.offsetY(-1): neighbor_position = neighbor_position.offsetY(1);
        neighbor_position = neighbor_position.offsetZ(-4);
        break;
    }
    case Direction::LEFT: {
        neighbor_position = neighbor_position.offsetX(-4);
        break;
    }
    case Direction::FRONT: {
        neighbor_position = neighbor_position.offsetY(-3);
        break;
    }
    case Direction::BACK: {
        neighbor_position = neighbor_position.offsetY(3);
        break;
    }
    default:
        break;
    }
    newSeed = neighbor_position;
}

map<Cell3DPosition, map<Cell3DPosition, int>> Init::distanceMatrix =
    map<Cell3DPosition, map<Cell3DPosition, int>>();

void Init::buildDistanceMatrix(bool rebuild) {
    if(not distanceMatrix.empty() and not rebuild) {
        return;
    }
    cerr << "Building DistanceMatrix" << endl;
    for(auto id_block1: BaseSimulator::getWorld()->buildingBlocksMap) {
        Cell3DPosition blockPosition1 = id_block1.second->position;
        for(auto id_block2: BaseSimulator::getWorld()->buildingBlocksMap) {
            Cell3DPosition blockPosition2 = id_block2.second->position;
            if(blockPosition1 != blockPosition2)
                distanceMatrix[blockPosition1][blockPosition2] = -1;
            else
                distanceMatrix[blockPosition1][blockPosition2] = 0;
            
        }
    }
    for(auto id_block1: BaseSimulator::getWorld()->buildingBlocksMap) {
        Cell3DPosition blockPosition1 = id_block1.second->position;
        for(auto id_block2: BaseSimulator::getWorld()->buildingBlocksMap) {
            Cell3DPosition blockPosition2 = id_block2.second->position;
            if (blockPosition1 != blockPosition2 and distanceMatrix[blockPosition1][blockPosition2] == -1) {
                distanceMatrix[blockPosition1][blockPosition2] = BaseSimulator::getWorld()
                        ->lattice->getCellDistance(blockPosition1, blockPosition2);
                 distanceMatrix[blockPosition2][blockPosition1] = distanceMatrix[blockPosition1][blockPosition2];
            }
        }
    }
    cerr << "distanceMatrix built" << endl;
}

int Init::getDistance(Cell3DPosition p1, Cell3DPosition p2) {
    if(distanceMatrix.empty()) {
        buildDistanceMatrix();
    } 
    return distanceMatrix[p1][p2];
}

#include "init.hpp"


Init::Init(/* args */) {
}

Init::~Init() {
}
 
bool Init::initialMapBuildDone = false;

void Init::buildInitialMap(Cell3DPosition firstSeedPos) {
    if(initialMapBuildDone) {
        cerr << "Initial map already built!!" << endl;
        return;
    }
    for(int i=1; i<(int)sizeof(initialMap)/(int)sizeof(initialMap[0]); i++) {
        int x,y,z;
        x = firstSeedPos.pt[0] + 4*initialMap[i][0];
        y = firstSeedPos.pt[1] + 3*initialMap[i][1];
        z = firstSeedPos.pt[2] + 4*initialMap[i][2];
        if(initialMap[i][2] % 2 != 0) {
            y -= 1;
        }
        BaseSimulator::getWorld()->addBlock(0, MetaModuleBlockCode::buildNewBlockCode, Cell3DPosition(x,y,z),  Colors[i % NB_COLORS] );
        MetaModuleBlockCode *newSeed = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockByPosition(Cell3DPosition(x,y,z))->blockCode
        );
      
        newSeed->MMPosition = Cell3DPosition(initialMap[i][0],initialMap[i][1],initialMap[i][2]);
        if( (newSeed->MMPosition.pt[0]+newSeed->MMPosition.pt[1]) % 2 == 0)
            newSeed->shapeState = FRONTBACK;
        else
            newSeed->shapeState = BACKFRONT;
        newSeed->seedPosition = newSeed->module->position;

        buildMM(newSeed->module, newSeed->shapeState, Colors[i % NB_COLORS]);
        if(initialMap[i][3] == 1)
            fillMM(newSeed->module);
    }
    initialMapBuildDone = true;
}

void Init::buildMM(Catoms3DBlock *s, MMShape shape, Color col) {
    Cell3DPosition seed_position = s->position;
    MetaModuleBlockCode* seedModule = static_cast<MetaModuleBlockCode*>(
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
        BaseSimulator::getWorld()->addBlock(0, MetaModuleBlockCode::buildNewBlockCode, newBlock_position, col);
        MetaModuleBlockCode* newBlock = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockByPosition(newBlock_position)->blockCode);

        newBlock->MMPosition = seedModule->MMPosition;
        newBlock->seedPosition = seedModule->module->position;
        newBlock->initialPosition = newBlock->module->position - newBlock->seedPosition;
        newBlock->shapeState = shape;
    }
}

void Init::buildMMon(Catoms3DBlock *s, Direction pos) {

    Cell3DPosition neighborSeedPos;
     MetaModuleBlockCode *block = static_cast<MetaModuleBlockCode*>(s->blockCode);
    getNeighborMMSeedPos(s->position, block->MMPosition, pos, neighborSeedPos);
    BaseSimulator::getWorld()->addBlock(0, seed->buildNewBlockCode, neighborSeedPos, Color(RED));
    MetaModuleBlockCode* neighborSeed = static_cast<MetaModuleBlockCode*>(BaseSimulator::getWorld()->getBlockByPosition(neighborSeedPos)->blockCode);
   
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
    MetaModuleBlockCode *S = static_cast<MetaModuleBlockCode*>(BaseSimulator::getWorld()->getBlockByPosition(s->position)->blockCode);
     if(S->shapeState == FRONTBACK) {
        for(int i=0; i<10; i++) {
            Cell3DPosition newBlockPos = s->position + Cell3DPosition(FillingPositions_FrontBack[i]);
            BaseSimulator::getWorld()->addBlock(0, MetaModuleBlockCode::buildNewBlockCode, newBlockPos, Color(LIGHTBLUE));
        }
    } else if(S->shapeState == BACKFRONT) {
        for(int i=0; i<10; i++) {
            Cell3DPosition newBlockPos = s->position + Cell3DPosition(FillingPositions_BackFront[i]);
            BaseSimulator::getWorld()->addBlock(0,  MetaModuleBlockCode::buildNewBlockCode, newBlockPos, Color(LIGHTBLUE));
        }
    }
    // else if(S->shapeState == BACKFRONT && S->MMPosition.pt[2] % 2 != 0) {
    //     for(int i=0; i<10; i++) {
    //         Cell3DPosition newBlockPos(s->position.pt[0]+ Fill_BackFrontZOdd[i][0],
    //          s->position.pt[1] + Fill_BackFrontZOdd[i][1],
    //          s->position.pt[2] + Fill_BackFrontZOdd[i][2]);
    //         BaseSimulator::getWorld()->addBlock(0,  MetaModuleBlockCode::buildNewBlockCode, newBlockPos, Color(LIGHTBLUE));
    //     }
    // }else if(S->shapeState == FRONTBACK && S->MMPosition.pt[2] % 2 != 0) {
    //     for(int i=0; i<10; i++) {
    //         Cell3DPosition newBlockPos(s->position.pt[0]+ Fill_FrontBackZOdd[i][0],
    //          s->position.pt[1] + Fill_FrontBackZOdd[i][1],
    //          s->position.pt[2] + Fill_FrontBackZOdd[i][2]);
    //         BaseSimulator::getWorld()->addBlock(0,  MetaModuleBlockCode::buildNewBlockCode, newBlockPos, Color(LIGHTBLUE));
    //     }
    // }

    // if(S->shapeState == FRONTBACK && S->MMPosition.pt[2] % 2 == 0) {
    //     for(int i=0; i<10; i++) {
    //         Cell3DPosition newBlockPos(s->position.pt[0]+ Fill_FrontBackZEven[i][0],
    //          s->position.pt[1] + Fill_FrontBackZEven[i][1],
    //          s->position.pt[2] + Fill_FrontBackZEven[i][2]);
    //         BaseSimulator::getWorld()->addBlock(0, MetaModuleBlockCode::buildNewBlockCode, newBlockPos, Color(LIGHTBLUE));
    //     }
    // } else if(S->shapeState == BACKFRONT && S->MMPosition.pt[2] % 2 == 0) {
    //     for(int i=0; i<10; i++) {
    //         Cell3DPosition newBlockPos(s->position.pt[0]+ Fill_BackFrontZEven[i][0],
    //          s->position.pt[1] + Fill_BackFrontZEven[i][1],
    //          s->position.pt[2] + Fill_BackFrontZEven[i][2]);
    //         BaseSimulator::getWorld()->addBlock(0,  MetaModuleBlockCode::buildNewBlockCode, newBlockPos, Color(LIGHTBLUE));
    //     }
    // }else if(S->shapeState == BACKFRONT && S->MMPosition.pt[2] % 2 != 0) {
    //     for(int i=0; i<10; i++) {
    //         Cell3DPosition newBlockPos(s->position.pt[0]+ Fill_BackFrontZOdd[i][0],
    //          s->position.pt[1] + Fill_BackFrontZOdd[i][1],
    //          s->position.pt[2] + Fill_BackFrontZOdd[i][2]);
    //         BaseSimulator::getWorld()->addBlock(0,  MetaModuleBlockCode::buildNewBlockCode, newBlockPos, Color(LIGHTBLUE));
    //     }
    // }else if(S->shapeState == FRONTBACK && S->MMPosition.pt[2] % 2 != 0) {
    //     for(int i=0; i<10; i++) {
    //         Cell3DPosition newBlockPos(s->position.pt[0]+ Fill_FrontBackZOdd[i][0],
    //          s->position.pt[1] + Fill_FrontBackZOdd[i][1],
    //          s->position.pt[2] + Fill_FrontBackZOdd[i][2]);
    //         BaseSimulator::getWorld()->addBlock(0,  MetaModuleBlockCode::buildNewBlockCode, newBlockPos, Color(LIGHTBLUE));
    //     }
    // }
}

void Init::getNeighborMMSeedPos(const Cell3DPosition &seedPos, const Cell3DPosition &MMPos, Direction pos, Cell3DPosition &newSeed) {
    Cell3DPosition neighbor_position = seedPos;
   
    switch (pos) {
    case Direction::UP:{
        // MetaModuleBlockCode* seedBlockCode =  static_cast<MetaModuleBlockCode*>(
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
        //  MetaModuleBlockCode* seedBlockCode =  static_cast<MetaModuleBlockCode*>(
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
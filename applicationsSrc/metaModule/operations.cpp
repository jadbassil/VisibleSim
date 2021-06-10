#include "metaModuleBlockCode.hpp"
#include "messages.hpp"
#include "operations.hpp"
#include "init.hpp"

/************************************************************************
 *************************  OPERATION  **********************************
 ***********************************************************************/

Operation::Operation(/* args */) {
    //localRules.reset(&LocalRules_BF_Dismantle_Left);
    op = NO_OPERATION;
}
Operation:: Operation(Direction _direction, MMShape _mmshape)
    :direction(_direction), mmShape(_mmshape) {
    op = NO_OPERATION;
}
Operation::~Operation() {}

/************************************************************************
 *************************  DISMANTLE OPERATION  ************************
 ***********************************************************************/

Dismantle_Operation::Dismantle_Operation (Direction _direction, MMShape _mmShape)
    :Operation(_direction, _mmShape) {
    
    switch (direction) {
    case Direction::LEFT:
        if(mmShape == BACKFRONT) {
            localRules.reset(&LocalRules_BF_Dismantle_Left);
            op = BF_Dismantle_Left;
            //localRules = &LocalRules_BF_Dismantle_Left;
        } else if(mmShape == FRONTBACK) {
            //localRules = &LocalRules_FB_Dismantle_Left
        }
        break;
    
    default:
        break;
    }
}

Dismantle_Operation::~Dismantle_Operation () {}

void Dismantle_Operation::handleAddNeighborEvent(BaseSimulator::BlockCode* bc, const Cell3DPosition&) {
    MetaModuleBlockCode* mmbc = static_cast<MetaModuleBlockCode*>(bc);
}

void Dismantle_Operation::updateState(BaseSimulator::BlockCode* bc) {
    MetaModuleBlockCode* mmbc = static_cast<MetaModuleBlockCode*>(bc);
    switch (direction and mmShape) {
    case Direction::LEFT and BACKFRONT: {
        mmbc->shapeState = FRONTBACK;
        Init::getNeighborMMSeedPos(mmbc->seedPosition, mmbc->MMPosition, Direction::LEFT, mmbc->seedPosition);
        mmbc->MMPosition = mmbc->MMPosition.offsetX(-1);
        mmbc->initialPosition = mmbc->module->position - mmbc->seedPosition;
        break;
    }
    
    default:
        break;
    }
}
/************************************************************************
 ***************************  FILL OPERATION  ***************************
 ***********************************************************************/

Fill_Operation::Fill_Operation (Direction _direction, MMShape _mmShape) 
    :Operation(_direction, _mmShape) {
    switch (direction) {
    case Direction::LEFT:
        if(mmShape == BACKFRONT) {
            //localRules.reset(&LocalRules_BF_Fill_Left);
        } else if(mmShape == FRONTBACK) {
            localRules.reset(&LocalRules_FB_Fill_Left);
            op = FB_Fill_Left;
        }
        break;
    
    default:
        break;
    }
}

Fill_Operation::~Fill_Operation () {}

void Fill_Operation::handleAddNeighborEvent(BaseSimulator::BlockCode* bc, const Cell3DPosition& pos) {
    MetaModuleBlockCode* mmbc = static_cast<MetaModuleBlockCode*>(bc);
    if(mmbc->isCoordinator 
        and pos == mmbc->module->position.offsetY(-1) 
        and mmbc->mvt_it < localRules->size()) {
        // console << "module added\n";
        // localRules = &LocalRules_FB_Fill_Left;
        Cell3DPosition targetModule = mmbc->seedPosition + (*localRules)[mmbc->mvt_it].currentPosition;
        while(targetModule != mmbc->module->position.offsetY(-1)) {
            mmbc->sendMessage("Coordinate Msg", new MessageOf<Coordinate>(
                COORDINATE_MSG_ID, Coordinate(mmbc->operation, targetModule, mmbc->module->position, mmbc->mvt_it)),
                mmbc->module->getInterface(mmbc->nearestPositionTo(targetModule)), 100, 200
            );
            mmbc->mvt_it++;
            targetModule = mmbc->seedPosition + (*localRules)[mmbc->mvt_it].currentPosition;
        }
        mmbc->sendMessage("Coordinate Msg", new MessageOf<Coordinate>(
            COORDINATE_MSG_ID, Coordinate(mmbc->operation, targetModule,  mmbc->module->position,  mmbc->mvt_it)),
            mmbc->module->getInterface( mmbc->nearestPositionTo(targetModule)), 100, 200
        );
        int i=0;
        int j= mmbc->mvt_it;
        while((*localRules)[j].state == MOVING) {
            i++;
            j++;
        }
         mmbc->mvt_it += i+1;
        //console << "mvt_it: " << mvt_it << "\n";
    }
}

void Fill_Operation::updateState(BaseSimulator::BlockCode *bc) {
    MetaModuleBlockCode* mmbc = static_cast<MetaModuleBlockCode*>(bc);
}

/************************************************************************
 ***************************  TRANSFER OPERATION  ***********************
 ***********************************************************************/

Transfer_Operation::Transfer_Operation(Direction _direction, MMShape _mmShape, bool _isLastTransferOperation) 
    :Operation(_direction, _mmShape), isLastTransferOperation(_isLastTransferOperation){
    switch (direction){
    case Direction::LEFT: {
        if(mmShape == FRONTBACK){
            localRules.reset(&LocalRules_FB_Transfer_Left);
            op = FB_Transfer_Left;
        } 
        else if(mmShape == BACKFRONT) {
            localRules.reset(&LocalRules_BF_Transfer_Left);
            op = BF_Transfer_Left;
        }
        break;
    }
    default:
        break;
    }
}

Transfer_Operation::~Transfer_Operation() {};

void Transfer_Operation::handleAddNeighborEvent(BaseSimulator::BlockCode* bc, const Cell3DPosition& pos) {
    MetaModuleBlockCode* mmbc = static_cast<MetaModuleBlockCode*>(bc);
    if(mmbc->isCoordinator) {
        if( pos == mmbc->module->position.offsetY(-1)
            or pos == mmbc->module->position.offsetY(1)) { // FB_Left and BF_LEFT
            mmbc->transferCount++;
            Cell3DPosition targetModule = mmbc->seedPosition + (*localRules)[mmbc->mvt_it].currentPosition;
            mmbc->sendMessage("Coordinate Msg", new MessageOf<Coordinate>(
                COORDINATE_MSG_ID, Coordinate(mmbc->operation, targetModule, mmbc->module->position, mmbc->mvt_it)),
                mmbc->module->getInterface(mmbc->nearestPositionTo(targetModule)), 100, 200
            ); 
            
            //mvt_it 
            if (mmbc->transferCount <= 2){ 
                int i = 0;
                int j = mmbc->mvt_it;
                while((*localRules)[j].state == MOVING) {
                    i++;
                    j++;
                }
                mmbc->mvt_it += i+1;
            } else if(mmbc->transferCount < 10) {
                mmbc->mvt_it = 7;
            }
            if(mmbc->transferCount == 10) {
                mmbc->mvt_it = 14;
            }
        }
    } else if(mmbc->movingState == WAITING) {
        if(mmbc->module->position - mmbc->seedPosition == Cell3DPosition(-1,0,1) // FB_Bridge
            or mmbc->module->position - mmbc->seedPosition == Cell3DPosition(-1,-1,1)) { //BF_bridge
            mmbc->transferCount++;
            //mmbc->scheduler->trace("transferCount: " + to_string(mmbc->transferCount), mmbc->module->blockId, Color(CYAN));
            if(mmbc->transferCount == 27) { // when all modules passed the bridge
                mmbc->sendMessage("CoordinateBack Msg", 
                    new MessageOf<CoordinateBack>(COORDINATEBACK_MSG_ID, CoordinateBack(0, mmbc->coordinatorPosition)),
                    mmbc->module->getInterface(mmbc->nearestPositionTo(mmbc->coordinatorPosition)) ,100, 200);
            }
        }
    }
    
}

void Transfer_Operation::updateState(BaseSimulator::BlockCode *bc) {
    MetaModuleBlockCode* mmbc = static_cast<MetaModuleBlockCode*>(bc);
    switch (direction and mmShape) {
    case Direction::LEFT and FRONTBACK: {
        mmbc->shapeState = BACKFRONT;
        Init::getNeighborMMSeedPos(mmbc->seedPosition, mmbc->MMPosition, Direction::LEFT, mmbc->seedPosition);
        mmbc->MMPosition = mmbc->MMPosition.offsetX(-1);
        mmbc->initialPosition = mmbc->module->position - mmbc->seedPosition;
        break;
    }
    case Direction::LEFT and BACKFRONT: {
        mmbc->shapeState = FRONTBACK;
        Init::getNeighborMMSeedPos( mmbc->seedPosition,  mmbc->MMPosition, Direction::LEFT,  mmbc->seedPosition);
        mmbc->MMPosition =  mmbc->MMPosition.offsetX(-1);
        mmbc->initialPosition =  mmbc->module->position -  mmbc->seedPosition;
    }
    default:
        break;
    }
}

bool Transfer_Operation::handleBridgeMovements(BaseSimulator::BlockCode* bc) {
    MetaModuleBlockCode* mmbc = static_cast<MetaModuleBlockCode*>(bc);
    LocalMovement lmvt = (*localRules)[mmbc->mvt_it];
    cout << "Test\n";
    if(lmvt.currentPosition == lmvt.nextPosition) {
        mmbc->mvt_it++;
        mmbc->movingState = IN_POSITION;
        mmbc->sendMessage("CoordinateBack Msg3", 
                        new MessageOf<CoordinateBack>(COORDINATEBACK_MSG_ID, CoordinateBack(1, mmbc->coordinatorPosition)),
                        mmbc->module->getInterface(mmbc->nearestPositionTo(mmbc->coordinatorPosition)) ,100, 200);
        mmbc->updateState();
        cout << mmbc->module->blockId << ": Transfer operation Done1!!!\n";
        mmbc->module->getInterface(mmbc->module->position.offsetY(-1))->isConnected()
            ? mmbc->coordinatorPosition = mmbc->module->position.offsetY(-1)
            : mmbc->coordinatorPosition = mmbc->module->position.offsetY(1);
        
        mmbc->sendMessage("CoordinateBack Msg4", 
                        new MessageOf<CoordinateBack>(COORDINATEBACK_MSG_ID, CoordinateBack(0, mmbc->coordinatorPosition)),
                        mmbc->module->getInterface(mmbc->coordinatorPosition) ,100, 200);
        return true;
    }
    if(direction == Direction::LEFT) {
        if(mmbc->movingState == WAITING and mmbc->relativePos() == Cell3DPosition(-2,0,2)) {
            mmbc->console << "mvt_it!!: " << mmbc->mvt_it << "\n";
            return true;
        }
        if(mmbc->movingState == WAITING 
            and (mmbc->module->position - mmbc->seedPosition).pt[0] <= -2
            and isLastTransferOperation
        ) {
            mmbc->movingState = IN_POSITION;
            mmbc->updateState();
            cout << mmbc->module->blockId << ": Last operation Done2!!!\n";
            return true;
        }
    } else {
        VS_ASSERT_MSG(false, "Direction not implemented");
    }
    
    return false;
}
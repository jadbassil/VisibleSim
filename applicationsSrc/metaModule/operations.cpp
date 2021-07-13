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
Operation:: Operation(Direction _direction, MMShape _mmshape, int Z)
    :direction(_direction), mmShape(_mmshape) {
    Zeven = (Z % 2 == 0);
    op = NO_OPERATION;
}
Operation::~Operation() {}

Cell3DPosition Operation::getNextSeed(BaseSimulator::BlockCode* bc) {
    MetaModuleBlockCode* mmbc = static_cast<MetaModuleBlockCode*>(bc);
    Cell3DPosition nextSeed;
    if(localRules) {
        Init::getNeighborMMSeedPos(mmbc->seedPosition, mmbc->MMPosition, getDirection(),nextSeed);
    }
    return nextSeed;
}

bool Operation::mustHandleBridgeOnAdd(const Cell3DPosition& pos) {
    MetaModuleBlockCode* posbc = static_cast<MetaModuleBlockCode*>(
        BaseSimulator::getWorld()->getBlockByPosition(pos)->blockCode);
    if (posbc->operation->isTransfer() and (posbc->operation->getDirection() == Direction::LEFT or
        posbc->operation->getDirection() == Direction::RIGHT)) {
            return true;
    }
    return false;
}

/************************************************************************
 *************************  DISMANTLE OPERATION  ************************
 ***********************************************************************/

Dismantle_Operation::Dismantle_Operation (Direction _direction, MMShape _mmShape, int Z)
    :Operation(_direction, _mmShape, Z) {
    
    switch (direction) {
    case Direction::LEFT:
        if(mmShape == BACKFRONT) {
            localRules.reset(&LocalRules_BF_Dismantle_Left);
            op = BF_Dismantle_Left;
            //localRules = &LocalRules_BF_Dismantle_Left;
        } else if(mmShape == FRONTBACK) {
            localRules.reset(&LocalRules_FB_Dismantle_Left);
            op = FB_Dismantle_Left;
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
    case Direction::LEFT and FRONTBACK: {
        mmbc->shapeState = BACKFRONT;
        Init::getNeighborMMSeedPos(mmbc->seedPosition, mmbc->MMPosition, Direction::LEFT, mmbc->seedPosition);
        mmbc->MMPosition = mmbc->MMPosition.offsetX(-1);
        mmbc->initialPosition = mmbc->module->position - mmbc->seedPosition;
    }
    
    default:
        break;
    }
}
/************************************************************************
 ***************************  FILL OPERATION  ***************************
 ***********************************************************************/

Fill_Operation::Fill_Operation (Direction _direction, MMShape _mmShape, int Z) 
    :Operation(_direction, _mmShape, Z) {
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

Transfer_Operation::Transfer_Operation(Direction _direction, MMShape _mmShape, int Z) 
    :Operation(_direction, _mmShape, Z){
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
    case Direction::UP: {
        if(Zeven) {
            if(mmShape == FRONTBACK) {
                localRules.reset(&LocalRules_FB_Transfer_Up_Zeven);
            } else  { //mmsShape = BACKFRRONT
                localRules.reset(&LocalRules_BF_Transfer_Up_Zeven);
            }
        } else { //Zodd
            if(mmShape == FRONTBACK) {
                localRules.reset(&LocalRules_FB_Transfer_Up_Zodd);
            } else  { //mmsShape = BACKFRRONT
                localRules.reset(&LocalRules_BF_Transfer_Up_Zodd);
            }
        }
    } break;
    case Direction::BACK: {
        if(mmShape == FRONTBACK) {
            localRules.reset(&LocalRules_FB_Transfer_Back);
        } else {// mmshape = BACKFRRONT
            localRules.reset(&LocalRules_BF_Transfer_Back);
        }
    } break;
    default:
        break;
    }
}

Transfer_Operation::~Transfer_Operation() {};

void Transfer_Operation::handleAddNeighborEvent(BaseSimulator::BlockCode* bc, const Cell3DPosition& pos) {
    MetaModuleBlockCode* mmbc = static_cast<MetaModuleBlockCode*>(bc);
    MetaModuleBlockCode* posbc = static_cast<MetaModuleBlockCode*>(
        BaseSimulator::getWorld()->getBlockByPosition(pos)->blockCode);
    if (mmbc->isCoordinator) {
        switch (direction) {
        case Direction::LEFT: case Direction::RIGHT: {
            if (pos == mmbc->module->position.offsetY(-1) or
                pos == mmbc->module->position.offsetY(1)) {  // FB_Left and BF_LEFT
                mmbc->transferCount++;

                stringstream sstream;
                sstream << "TransferCount: " + to_string(mmbc->transferCount) << "\n";
                getScheduler()->trace(sstream.str(), mmbc->module->blockId, Color(MAGENTA));
                Cell3DPosition targetModule =
                    mmbc->seedPosition + (*localRules)[mmbc->mvt_it].currentPosition;
                if (mmbc->transferCount == 8 and mustHandleBridgeOnAdd(pos)) {
                    // skip to avoid unsupported motion
                    return;
                }
                mmbc->sendMessage(
                    "Coordinate Msg",
                    new MessageOf<Coordinate>(COORDINATE_MSG_ID,
                                              Coordinate(mmbc->operation, targetModule,
                                                         mmbc->module->position, mmbc->mvt_it)),
                    mmbc->module->getInterface(pos), 100, 200);

                if (mmbc->transferCount <= 2) {
                    int i = 0;
                    int j = mmbc->mvt_it;
                    while ((*localRules)[j].state == MOVING) {
                        i++;
                        j++;
                    }
                    mmbc->mvt_it += i + 1;
                } else if (mmbc->transferCount < 10) {
                    mmbc->mvt_it = 8;
                }
                if (mmbc->transferCount == 10) {
                    mmbc->mvt_it = 14;
                }
            }
            break;
        }
        case Direction::UP: case Direction::DOWN: {
            if (pos == mmbc->module->position.offsetY(-1) or
                pos == mmbc->module->position.offsetY(1)) {
                VS_ASSERT(mmbc->mvt_it == 0);
                mmbc->transferCount++;
                
                if (mmbc->transferCount == 8 and mustHandleBridgeOnAdd(pos)) {
                    // skip to avoid unsupported motion
                    return;
                }
                Cell3DPosition targetModule =
                    mmbc->seedPosition + (*localRules)[mmbc->mvt_it].currentPosition;
                mmbc->sendMessage(
                    "Coordinate Msg",
                    new MessageOf<Coordinate>(COORDINATE_MSG_ID,
                                              Coordinate(mmbc->operation, targetModule,
                                                         mmbc->module->position, mmbc->mvt_it)),
                    mmbc->module->getInterface(pos), 100, 200);
            }
            // Same sequence of movements for all modules.
            
            break;
        }
        case Direction::BACK: case Direction::FRONT: {
            if (pos == mmbc->module->position.offsetY(-1) or
                pos == mmbc->module->position.offsetY(1)) {
                mmbc->transferCount++;
                stringstream sstream;
                sstream << "TransferCount: " + to_string(mmbc->transferCount) << "\n";
                getScheduler()->trace(sstream.str(), mmbc->module->blockId, Color(MAGENTA));
                if (mmShape == FRONTBACK) {
                    if (mmbc->transferCount == 8 and mustHandleBridgeOnAdd(pos)) {
                        // skip to avoid unsupported motion
                        return;
                    }
                    Cell3DPosition targetModule =
                        mmbc->seedPosition + (*localRules)[mmbc->mvt_it].currentPosition;
                    mmbc->sendMessage(
                        "Coordinate Msg",
                        new MessageOf<Coordinate>(COORDINATE_MSG_ID,
                                                  Coordinate(mmbc->operation, targetModule,
                                                             mmbc->module->position, mmbc->mvt_it)),
                        mmbc->module->getInterface(mmbc->nearestPositionTo(targetModule)), 100,
                        200);

                    if (mmbc->transferCount <= 3) {
                        int i = 0;
                        int j = mmbc->mvt_it;
                        while ((*localRules)[j].state == MOVING) {
                            i++;
                            j++;
                        }
                        mmbc->mvt_it += i + 1;
                        if (mmbc->transferCount == 3) {
                            targetModule =
                                mmbc->seedPosition + (*localRules)[mmbc->mvt_it].currentPosition;
                            mmbc->sendMessage("Coordinate Msg",
                                              new MessageOf<Coordinate>(
                                                  COORDINATE_MSG_ID,
                                                  Coordinate(mmbc->operation, targetModule,
                                                             mmbc->module->position, mmbc->mvt_it)),
                                              mmbc->module->getInterface(pos), 100, 200);
                        }
                    } else if (mmbc->transferCount < 10) {
                        mmbc->mvt_it = 8;
                    }
                    if (mmbc->transferCount == 10) {
                        mmbc->mvt_it = 13;
                    }
                } else { //mmShape = BACKFRONT
                    if (mmbc->transferCount == 8 and mustHandleBridgeOnAdd(pos)) {
                        // skip to avoid unsupported motion
                        return;
                    }
                    Cell3DPosition targetModule =
                        mmbc->seedPosition + (*localRules)[mmbc->mvt_it].currentPosition;
                    mmbc->sendMessage(
                        "Coordinate Msg",
                        new MessageOf<Coordinate>(COORDINATE_MSG_ID,
                                                  Coordinate(mmbc->operation, targetModule,
                                                             mmbc->module->position, mmbc->mvt_it)),
                        mmbc->module->getInterface(mmbc->nearestPositionTo(targetModule)), 100,
                        200);
                    if (mmbc->transferCount <= 4) {
                        int i = 0;
                        int j = mmbc->mvt_it;
                        while ((*localRules)[j].state == MOVING) {
                            i++;
                            j++;
                        }
                        mmbc->mvt_it += i + 1;
                        if (mmbc->transferCount == 4 and
                            mmbc->module->getInterface(pos)->isConnected()) {
                            targetModule =
                                mmbc->seedPosition + (*localRules)[mmbc->mvt_it].currentPosition;
                            mmbc->sendMessage("Coordinate Msg",
                                              new MessageOf<Coordinate>(
                                                  COORDINATE_MSG_ID,
                                                  Coordinate(mmbc->operation, targetModule,
                                                             mmbc->module->position, mmbc->mvt_it)),
                                              mmbc->module->getInterface(pos), 100, 200);
                        }
                    } else if(mmbc->transferCount < 10) {
                        mmbc->mvt_it = 17;
                    }
                    if (mmbc->transferCount == 10) {
                        mmbc->mvt_it = 21;
                    }
                }
            }
        }
        default:
            break;
        }

    } else if (mmbc->movingState == WAITING) {
        // Special logic for bridge: wait for all modules to pass
        // the bridge then transfer the bridge
        mmbc->transferCount++;
        mmbc->scheduler->trace("transferCount: " + to_string(mmbc->transferCount),
            mmbc->module->blockId, Color(CYAN));
        if ((mmbc->transferCount == 33 and
             (direction == Direction::LEFT or direction == Direction::RIGHT)) or
            (mmbc->transferCount == 20 and 
             (direction == Direction::BACK or
              direction == Direction::FRONT ))) {  // when all modules passed the bridge
            mmbc->sendMessage(
                "CoordinateBack Msg",
                new MessageOf<CoordinateBack>(COORDINATEBACK_MSG_ID,
                                              CoordinateBack(0, mmbc->coordinatorPosition)),
                mmbc->module->getInterface(mmbc->nearestPositionTo(mmbc->coordinatorPosition)), 100,
                200);
        }
    }
}

void Transfer_Operation::updateState(BaseSimulator::BlockCode *bc) {
    MetaModuleBlockCode* mmbc = static_cast<MetaModuleBlockCode*>(bc);
    switch (direction ) {
    case Direction::LEFT : {
        if (mmShape == FRONTBACK) {
            mmbc->shapeState = BACKFRONT;
            Init::getNeighborMMSeedPos(mmbc->seedPosition, mmbc->MMPosition, Direction::LEFT,
                                       mmbc->seedPosition);
            mmbc->MMPosition = mmbc->MMPosition.offsetX(-1);
            mmbc->initialPosition = mmbc->module->position - mmbc->seedPosition;
            break;
        } else if (mmShape == BACKFRONT) {
            mmbc->shapeState = FRONTBACK;
            Init::getNeighborMMSeedPos(mmbc->seedPosition, mmbc->MMPosition, Direction::LEFT,
                                       mmbc->seedPosition);
            mmbc->MMPosition = mmbc->MMPosition.offsetX(-1);
            mmbc->initialPosition = mmbc->module->position - mmbc->seedPosition;
            break;
        }
    } break;
    case Direction::UP : {
        Init::getNeighborMMSeedPos(mmbc->seedPosition, mmbc->MMPosition, Direction::UP,
                                    mmbc->seedPosition);
        mmbc->MMPosition = mmbc->MMPosition.offsetZ(1);
        mmbc->initialPosition = mmbc->module->position - mmbc->seedPosition;
    } break;
    case Direction::BACK : {
        Init::getNeighborMMSeedPos(mmbc->seedPosition, mmbc->MMPosition, Direction::BACK,
                                    mmbc->seedPosition);
        mmbc->MMPosition = mmbc->MMPosition.offsetY(1);
        mmbc->initialPosition = mmbc->module->position - mmbc->seedPosition;
    } break;

    default:
        break;
    }
}

bool Transfer_Operation::handleBridgeMovements(BaseSimulator::BlockCode* bc) {
    MetaModuleBlockCode* mmbc = static_cast<MetaModuleBlockCode*>(bc);
    LocalMovement lmvt = (*localRules)[mmbc->mvt_it];
 
    if(lmvt.currentPosition == lmvt.nextPosition /**and not isFirstTransferOperation**/) {
        mmbc->mvt_it++;
        mmbc->movingState = IN_POSITION;
        mmbc->sendMessage("CoordinateBack Msg3", 
                        new MessageOf<CoordinateBack>(COORDINATEBACK_MSG_ID, CoordinateBack(1, mmbc->coordinatorPosition)),
                        mmbc->module->getInterface(mmbc->nearestPositionTo(mmbc->coordinatorPosition)) ,100, 200);
       
       //mmbc->updateState();
        //if(direction == Direction::LEFT or direction == Direction::RIGHT) {
            mmbc->module->getInterface(mmbc->module->position.offsetY(-1))->isConnected()
                ? mmbc->coordinatorPosition = mmbc->module->position.offsetY(-1)
                : mmbc->coordinatorPosition = mmbc->module->position.offsetY(1);
            mmbc->sendMessage(
                "CoordinateBack Msg4",
                new MessageOf<CoordinateBack>(COORDINATEBACK_MSG_ID,
                                              CoordinateBack(0, mmbc->coordinatorPosition)),
                mmbc->module->getInterface(mmbc->coordinatorPosition), 100, 200);
            //}
            

            return true;
    } else {
        return false;
    }
    // if(direction == Direction::LEFT) {
    //     if(mmbc->movingState == WAITING and mmbc->relativePos() == Cell3DPosition(-2,0,2)) {
    //         mmbc->console << "mvt_it!!: " << mmbc->mvt_it << "\n";
    //         return true;
    //     }
    //     if(mmbc->movingState == WAITING 
    //         and (mmbc->module->position - mmbc->seedPosition).pt[0] <= -2
    //         and isFirstTransferOperation
    //     ) {
    //         mmbc->movingState = IN_POSITION;
    //         mmbc->updateState();
    //         cout << mmbc->module->blockId << ": Last operation Done2!!!\n";
    //         return true;
    //     }
    // } else {
    //     VS_ASSERT_MSG(false, "Direction not implemented");
    // }
    
    return false;
}

/************************************************************************
 ***************************  BUILD OPERATION  **************************
 ***********************************************************************/

Build_Operation::Build_Operation (Direction _direction, MMShape _mmShape, int Z)
    :Operation(_direction, _mmShape, Z) {
   
    switch (direction) {
    case Direction::UP:
        // motions rules are set for Z even
        if(Zeven) {
            if(mmShape == BACKFRONT) {
                localRules.reset(&LocalRules_BF_Build_Up);
            } else if(mmShape == FRONTBACK) {
                localRules.reset(&LocalRules_FB_Build_Up);
            }
        } else { // Z odd
            
            cerr << "Z is odd" << endl;
            if(mmShape == BACKFRONT) {
                localRules.reset(&LocalRules_BF_Build_Up_ZOdd);
            } else if(mmShape == FRONTBACK) {
                localRules.reset(&LocalRules_FB_Build_Up_ZOdd);
            }
        }
        
        break;
    
    default:
        break;
    }
}
 
Build_Operation::~Build_Operation () {}

void Build_Operation::handleAddNeighborEvent(BaseSimulator::BlockCode* bc,
                                             const Cell3DPosition& pos) {
    MetaModuleBlockCode* mmbc = static_cast<MetaModuleBlockCode*>(bc);
     MetaModuleBlockCode* posbc = static_cast<MetaModuleBlockCode*>(
        BaseSimulator::getWorld()->getBlockByPosition(pos)->blockCode);
    if (mmbc->isCoordinator and abs(pos.pt[1] - mmbc->module->position.pt[1]) == 1 and
        abs(pos.pt[2] - mmbc->module->position.pt[2]) == 0 and
        mmbc->mvt_it < localRules->size()) {
        
        mmbc->transferCount++; 
        getScheduler()->trace("transferCount: " + to_string(mmbc->transferCount), mmbc->module->blockId, Color(MAGENTA));
        Cell3DPosition targetModule =
            mmbc->seedPosition + (*localRules)[mmbc->mvt_it].currentPosition;
        if (mustHandleBridgeOnAdd(pos)) {  // suppose that there is a bridge
            if(mmbc->transferCount == 8) return;
            else if(mmbc->transferCount == 9) {
                // msg so must jump to next module if previous operation requires bridging
                while((*localRules)[mmbc->mvt_it].state == MOVING) {
                    mmbc->mvt_it++;
                }
                mmbc->mvt_it++; 
            }
        }

        mmbc->sendMessage(
            "Coordinate Msg",
            new MessageOf<Coordinate>(
                COORDINATE_MSG_ID,
                Coordinate(mmbc->operation, targetModule, mmbc->module->position, mmbc->mvt_it)),
            mmbc->module->getInterface(pos), 100, 200);
    
        int i = 0;
        int j = mmbc->mvt_it;
        while ((*localRules)[j].state == MOVING) {
            i++;
            j++;
        }
        mmbc->mvt_it += i + 1;
        mmbc->console << "mvt_it: " << mmbc->mvt_it << "\n";
    }
}

void Build_Operation::updateState(BaseSimulator::BlockCode* bc) {
    MetaModuleBlockCode* mmbc = static_cast<MetaModuleBlockCode*>(bc);
    switch (direction) {
    case Direction::UP: {
        if(mmShape == FRONTBACK) {
            mmbc->shapeState = FRONTBACK;
            Init::getNeighborMMSeedPos(mmbc->seedPosition, mmbc->MMPosition, Direction::UP, mmbc->seedPosition);
            mmbc->MMPosition = mmbc->MMPosition.offsetZ(1);
            mmbc->initialPosition = mmbc->module->position - mmbc->seedPosition;
            break;
        } else if(mmShape == BACKFRONT){
            mmbc->shapeState = BACKFRONT;
            Init::getNeighborMMSeedPos(mmbc->seedPosition, mmbc->MMPosition, Direction::UP, mmbc->seedPosition);
            mmbc->MMPosition = mmbc->MMPosition.offsetZ(1);
            mmbc->initialPosition = mmbc->module->position - mmbc->seedPosition;
            break;
        }
        
    }

    
    default:
        break;
    }

}


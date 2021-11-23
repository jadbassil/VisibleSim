#include "rePoStBlockCode.hpp"
#include "operations.hpp"
#include "init.hpp"
#include "messages/transportMessages.hpp"

/************************************************************************
 *************************  OPERATION  **********************************
 ***********************************************************************/

Operation::Operation(/* args */) {
    //localRules.reset(&LocalRules_BF_Dismantle_Left);
}
Operation:: Operation(Direction _direction, MMShape _mmshape, Direction _prevOpDirection, int Z)
    :direction(_direction), mmShape(_mmshape), prevOpDirection(_prevOpDirection) {
    Zeven = (Z % 2 == 0);
}
Operation::~Operation() {}

Cell3DPosition Operation::getNextSeed(BaseSimulator::BlockCode* bc) {
    RePoStBlockCode* rbc = static_cast<RePoStBlockCode*>(bc);
    Cell3DPosition nextSeed;
    if(localRules) {
        Init::getNeighborMMSeedPos(rbc->seedPosition, rbc->MMPosition, getDirection(),nextSeed);
    }
    return nextSeed;
}

bool Operation::mustHandleBridgeOnAdd(const Cell3DPosition& pos) {
    RePoStBlockCode* posbc = static_cast<RePoStBlockCode*>(
        BaseSimulator::getWorld()->getBlockByPosition(pos)->blockCode);
    if (posbc->operation->isTransfer() and (posbc->operation->getDirection() == Direction::LEFT)) {
        return true;
    }
    if (posbc->operation->isDismantle()) {
        if (static_cast<Dismantle_Operation*>(posbc->operation)->filled and
            (posbc->operation->getDirection() == Direction::LEFT))
            return true;
    }
    return false;
}

void Operation::setMvtItToNextModule(BaseSimulator::BlockCode* bc) {
    RePoStBlockCode* rbc = static_cast<RePoStBlockCode*>(bc);
    assert(localRules);
    int i = 0;
    int j = rbc->mvt_it;
    while ((*localRules)[j].state == MOVING) {
        i++;
        j++;
    }
    rbc->mvt_it += i + 1;
}

/************************************************************************
 *************************  DISMANTLE OPERATION  ************************
 ***********************************************************************/

Dismantle_Operation::Dismantle_Operation (Direction _direction, MMShape _mmShape, Direction _prevOpDirection, int Z, bool _filled)
    :Operation(_direction, _mmShape, _prevOpDirection, Z), filled(_filled)  {
    
    switch (direction) {
    case Direction::LEFT: {
        if(mmShape == BACKFRONT) {
            if(filled) localRules.reset(&LocalRules_BF_DismantleFilled_Left_Zeven);
            else{
                Zeven ? localRules.reset(&LocalRules_BF_Dismantle_Left):
                    localRules.reset(&LocalRules_BF_Dismantle_Left_Zodd);
            }
        } else if(mmShape == FRONTBACK) {
            Zeven ? localRules.reset(&LocalRules_FB_Dismantle_Left):
                    localRules.reset(&LocalRules_FB_Dismantle_Left_Zodd);
        }
    } break;
    
    case Direction::BACK: {
        if(mmShape == BACKFRONT) {
            localRules.reset(&LocalRules_BF_Dismantle_Back);
        } else if(mmShape == FRONTBACK) {
            localRules.reset(&LocalRules_FB_Dismantle_Back);
        }
    } break;

    case Direction::UP: {
        if(mmShape == BACKFRONT) {
            Zeven ? VS_ASSERT_MSG(false, "Not implemented"): 
                localRules.reset(&LocalRules_BF_Dismantle_Up_Zodd);
        } else if(mmShape == FRONTBACK) {
             Zeven ? VS_ASSERT_MSG(false, "Not implemented"): 
                localRules.reset(&LocalRules_FB_Dismantle_Up_Zodd);
        }
    } break;
    
    default:
        VS_ASSERT_MSG(false, "Not implemented");
    }
}

Dismantle_Operation::~Dismantle_Operation () {}

void Dismantle_Operation::handleAddNeighborEvent(BaseSimulator::BlockCode* bc, const Cell3DPosition&) {
    RePoStBlockCode* rbc = static_cast<RePoStBlockCode*>(bc);
}

void Dismantle_Operation::updateState(BaseSimulator::BlockCode* bc) {
    RePoStBlockCode* rbc = static_cast<RePoStBlockCode*>(bc);
    switch (direction) {
        case Direction::LEFT: {
            (rbc->shapeState == FRONTBACK) ? rbc->shapeState = BACKFRONT
                                            : rbc->shapeState = FRONTBACK;
            Init::getNeighborMMSeedPos(rbc->seedPosition, rbc->MMPosition, Direction::LEFT,
                                       rbc->seedPosition);
            rbc->MMPosition = rbc->MMPosition.offsetX(-1);
            rbc->initialPosition = rbc->module->position - rbc->seedPosition;

        } break;

        case Direction::BACK: {
            (rbc->shapeState == FRONTBACK) ? rbc->shapeState = BACKFRONT
                                            : rbc->shapeState = FRONTBACK;
            Init::getNeighborMMSeedPos(rbc->seedPosition, rbc->MMPosition, Direction::BACK,
                                       rbc->seedPosition);
            rbc->MMPosition = rbc->MMPosition.offsetY(1);
            rbc->initialPosition = rbc->module->position - rbc->seedPosition;
        } break;

        case Direction::UP: {
            Init::getNeighborMMSeedPos(rbc->seedPosition, rbc->MMPosition, Direction::UP,
                                       rbc->seedPosition);
            rbc->MMPosition = rbc->MMPosition.offsetZ(1);
            rbc->initialPosition = rbc->module->position - rbc->seedPosition;
        } break;

        default:
            VS_ASSERT_MSG(false, "Not implemented");
    }
}

bool Dismantle_Operation::mustSendCoordinateBack(BaseSimulator::BlockCode* bc) {
    RePoStBlockCode* rbc = static_cast<RePoStBlockCode*>(bc);
    return rbc->isCoordinator? false:  true;
}
/************************************************************************
 ***************************  FILL OPERATION  ***************************
 ***********************************************************************/

Fill_Operation::Fill_Operation (Direction _direction, MMShape _mmShape, Direction _prevOpDirection, bool _comingFromBack, int Z) 
    :Operation(_direction, _mmShape, _prevOpDirection, Z) {
    switch (direction) {
        case Direction::LEFT: {
            if (mmShape == BACKFRONT) {
                Zeven ? localRules.reset(&LocalRules_BF_Fill_Left_Zeven)
                      : localRules.reset(&LocalRules_BF_Fill_Left_Zodd);
            } else if (mmShape == FRONTBACK) {
                Zeven ? localRules.reset(&LocalRules_FB_Fill_Left_Zeven)
                      : localRules.reset(&LocalRules_FB_Fill_Left_Zodd);
            }
        } break;
        case Direction::BACK: {
            if (mmShape == BACKFRONT) {
                comingFromBack = _comingFromBack;
                if(Zeven) {
                    if(not comingFromBack) localRules.reset(&LocalRules_BF_Fill_Back_Zeven);
                    else localRules.reset(&LocalRules_BF_Fill_Back_Zeven_ComingFromBack); //comingFromBack
                } else { //Zodd
                     if(not comingFromBack) localRules.reset(&LocalRules_BF_Fill_Back_Zodd);
                     else   localRules.reset(&LocalRules_BF_Fill_Back_Zodd_ComingFromBack);
                }
            } else if (mmShape == FRONTBACK) {
                comingFromBack = false;
                Zeven ?
                 localRules.reset(&LocalRules_FB_Fill_Back_Zeven)
                    : localRules.reset(&LocalRules_FB_Fill_Back_Zodd);
            }
        } break;
        default:
            break;
    }
}

Fill_Operation::~Fill_Operation () {}

void Fill_Operation::handleAddNeighborEvent(BaseSimulator::BlockCode* bc, const Cell3DPosition& pos) {
    RePoStBlockCode* rbc = static_cast<RePoStBlockCode*>(bc);
    if(rbc->isCoordinator) {
        switch (direction){
            case Direction::LEFT: {
                if (pos == rbc->module->position.offsetY(-1) or
                    pos == rbc->module->position.offsetY(1)) {
                    rbc->transferCount++;
                    rbc->console << "mvt_it1: " << rbc->mvt_it << "\n";

                    getScheduler()->trace("transferCount: " + to_string(rbc->transferCount),
                                          rbc->module->blockId, Color(MAGENTA));

                    Cell3DPosition targetModule =
                        rbc->seedPosition + (*localRules)[rbc->mvt_it].currentPosition;
                    // if(rbc->transferCount == 10) return;
                    if (rbc->transferCount >= 8) {
                        return;
                    }
                    rbc->sendHandleableMessage(
                        new CoordinateMessage(rbc->operation, targetModule, rbc->module->position,
                                              rbc->mvt_it),
                        rbc->module->getInterface(pos), 100, 200);
                    if (rbc->transferCount < 7) {
                        setMvtItToNextModule(bc);
                        rbc->console << "mvt_it2: " << rbc->mvt_it << "\n";
                    }
                }
            } break;

            case Direction::BACK: {
                if (pos == rbc->module->position.offsetY(-1) or
                    pos == rbc->module->position.offsetY(1) and
                        prevOpDirection != Direction::BACK) {
                    rbc->transferCount++;
                    rbc->console << "mvt_it1: " << rbc->mvt_it << "\n";
                    getScheduler()->trace("transferCount: " + to_string(rbc->transferCount), rbc->module->blockId, Color(MAGENTA));
                    // if (mmShape == BACKFRONT) {
                    //     if (rbc->transferCount == 3) {
                    //         return;
                    //     }
                    //     if(rbc->transferCount == 4) {
                    //         setMvtItToNextModule(bc);
                    //     }
                    //     //    Cell3DPosition targetModule =
                    //     //     rbc->seedPosition + (*localRules)[rbc->mvt_it].currentPosition;
                    //     // rbc->sendHandleableMessage(
                    //     //     new CoordinateMessage(rbc->operation, targetModule,
                    //     //                           rbc->module->position, rbc->mvt_it),
                    //     //     rbc->module->getInterface(pos), 100, 200);
                    //     // setMvtItToNextModule(bc);
                        
                    // }

                    if (mustHandleBridgeOnAdd(pos)) {  // suppose that there is a bridge
                        if (rbc->transferCount == 8)
                            return;
                        else if (rbc->transferCount == 9) {
                            //if (mmShape == BACKFRONT) return;
                            // msg so must jump to next module if previous operation requires
                            // bridging
                            setMvtItToNextModule(bc);
                        }
                    }
                    Cell3DPosition targetModule =
                        rbc->seedPosition + (*localRules)[rbc->mvt_it].currentPosition;
                    rbc->sendHandleableMessage(
                        new CoordinateMessage(rbc->operation, targetModule, rbc->module->position,
                                              rbc->mvt_it),
                        rbc->module->getInterface(pos), 100, 200);

                    // if(mmShape == BACKFRONT) {
                    //     if(rbc->transferCount == 2) {
                    //         return;
                    //     }
                    // }
              
                    if(rbc->transferCount < 10) {
                         setMvtItToNextModule(bc);
                        rbc->console << "mvt_it2: " << rbc->mvt_it << "\n";
                    }
                   
                } else if(pos == rbc->module->position + Cell3DPosition(0, 1, 1) and rbc->transferCount < 10) {
                    if(direction == Direction::BACK and prevOpDirection == Direction::BACK and mmShape == BACKFRONT) {
                        rbc->transferCount++;
                        getScheduler()->trace("transferCount: " + to_string(rbc->transferCount),
                                            rbc->module->blockId, Color(MAGENTA));
                        if(rbc->transferCount >= 5 and Zeven) return;
                        Cell3DPosition targetModule =
                            rbc->seedPosition + (*localRules)[rbc->mvt_it].currentPosition;
                        rbc->sendHandleableMessage(
                                new CoordinateMessage(rbc->operation, targetModule, rbc->module->position,
                                                    rbc->mvt_it),
                                rbc->module->getInterface(pos), 100, 200);
                        
                        if (rbc->transferCount < 9 and rbc->transferCount != 4 and Zeven) {
                            setMvtItToNextModule(bc);
                            
                        }  else if(not Zeven and rbc->transferCount < 10) {
                            setMvtItToNextModule(bc);
                        }
                    }
                }
            } break;
        default:
            break;
        }
      
    }
}

void Fill_Operation::updateState(BaseSimulator::BlockCode* bc) {
    RePoStBlockCode* rbc = static_cast<RePoStBlockCode*>(bc);
    switch (direction) {
        case Direction::LEFT: {
            if (mmShape == FRONTBACK) {
                rbc->shapeState = BACKFRONT;
                Init::getNeighborMMSeedPos(rbc->seedPosition, rbc->MMPosition, Direction::LEFT,
                                           rbc->seedPosition);
                rbc->MMPosition = rbc->MMPosition.offsetX(-1);
                rbc->initialPosition = rbc->module->position - rbc->seedPosition;
                break;
            } else if (mmShape == BACKFRONT) {
                rbc->shapeState = FRONTBACK;
                Init::getNeighborMMSeedPos(rbc->seedPosition, rbc->MMPosition, Direction::LEFT,
                                           rbc->seedPosition);
                rbc->MMPosition = rbc->MMPosition.offsetX(-1);
                rbc->initialPosition = rbc->module->position - rbc->seedPosition;
                break;
            }
        } break;
        case Direction::BACK: {
            rbc->shapeState == FRONTBACK ? rbc->shapeState = BACKFRONT
                                         : rbc->shapeState = FRONTBACK;
            Init::getNeighborMMSeedPos(rbc->seedPosition, rbc->MMPosition, Direction::BACK,
                                       rbc->seedPosition);
            rbc->MMPosition = rbc->MMPosition.offsetY(1);
            rbc->initialPosition = rbc->module->position - rbc->seedPosition;
        } break;

        default:
            break;
    }
}

bool Fill_Operation::mustSendCoordinateBack(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode* rbc = static_cast<RePoStBlockCode*>(bc);
    if(rbc->mvt_it >= localRules->size() - 1) return false;
    if (direction == Direction::LEFT) {
        if (mmShape == FRONTBACK and Zeven and (rbc->mvt_it >= 51))
            return true;
        if (mmShape == BACKFRONT and Zeven and rbc->mvt_it >= 40) return true;
        if (mmShape == BACKFRONT and rbc->mvt_it >= 45) return true;
        if (mmShape == FRONTBACK and (rbc->mvt_it >= 51)) return true;
    } else if(direction == Direction::BACK) {
        if(Zeven) {
            if (mmShape == FRONTBACK and rbc->mvt_it >= 53) return true;
            if(mmShape == BACKFRONT and not comingFromBack and rbc->mvt_it >= 42 ) return true;
            if (mmShape == BACKFRONT and comingFromBack and
                (rbc->mvt_it == 9 or rbc->mvt_it == 15 or rbc->mvt_it == 21 or rbc->mvt_it >= 31))
                return true;
        } else { //Zodd
            if (mmShape == FRONTBACK and rbc->mvt_it >= 52) return true;
            if (mmShape == BACKFRONT and rbc->mvt_it >= 52 and prevOpDirection != Direction::BACK) return true;
            if (mmShape == BACKFRONT and rbc->mvt_it >= 44 and prevOpDirection == Direction::BACK) return true;
        }
        
    }

    return false;
}

/************************************************************************
 ***************************  TRANSFER OPERATION  ***********************
 ***********************************************************************/

Transfer_Operation::Transfer_Operation(Direction _direction, MMShape _mmShape, Direction _prevOpDirection, bool _comingFromBack, int Z) 
    :Operation(_direction, _mmShape,_prevOpDirection, Z), comingFromBack(_comingFromBack) {
    switch (direction){
    case Direction::LEFT: {
        if(mmShape == FRONTBACK){
            localRules.reset(&LocalRules_FB_Transfer_Left);
        } 
        else if(mmShape == BACKFRONT) {
            localRules.reset(&LocalRules_BF_Transfer_Left);
        }   
    } break; 

    case Direction::RIGHT: {
        if(mmShape == FRONTBACK){
            Zeven ? localRules.reset(&LocalRules_FB_Transfer_Right_Zeven)
                : localRules.reset(&LocalRules_FB_Transfer_Right_Zodd);
        }  else if(mmShape == BACKFRONT) {
            Zeven ? localRules.reset(&LocalRules_BF_Transfer_Right_Zeven)
                : localRules.reset(&LocalRules_BF_Transfer_Right_Zodd);
            ;
        } 
    } break;

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

    case Direction::DOWN: {
        if(mmShape == FRONTBACK) {
            (Zeven) ? localRules.reset(&LocalRules_FB_Transfer_Down_Zeven):
                    localRules.reset(&LocalRules_FB_Transfer_Down_Zodd);
        } else { //mmShape = BACKFRONT
            (Zeven) ? localRules.reset(&LocalRules_BF_Transfer_Down_Zeven):
                    localRules.reset(&LocalRules_BF_Transfer_Down_Zodd);
        }
    } break;

    case Direction::BACK: {
        if (mmShape == FRONTBACK) {
            localRules.reset(&LocalRules_FB_Transfer_Back);
        } else {  // mmshape = BACKFRRONT
            if(comingFromBack) {
                localRules.reset(&LocalRules_BF_Transfer_Back_ComingFromBack);
            } else {
                localRules.reset(&LocalRules_BF_Transfer_Back);
            }
            
        }
    } break;

    case Direction::FRONT: {
        if(mmShape == FRONTBACK) {
            if(prevOpDirection == Direction::FRONT) {
                localRules.reset(&LocalRules_FB_Transfer_Front_ComingFromBack);
            } else {
                localRules.reset(&LocalRules_FB_Transfer_Front);
            }
        } else { //BACKFRONT
            localRules.reset(&LocalRules_BF_Transfer_Front);
        }
    } break;
    default:
        VS_ASSERT_MSG(false, "Not implemented");
        break;
    }
}

Transfer_Operation::~Transfer_Operation() {};

void Transfer_Operation::handleAddNeighborEvent(BaseSimulator::BlockCode* bc, const Cell3DPosition& pos) {
    RePoStBlockCode* rbc = static_cast<RePoStBlockCode*>(bc);
    RePoStBlockCode* posbc = static_cast<RePoStBlockCode*>(
        BaseSimulator::getWorld()->getBlockByPosition(pos)->blockCode);
    if (rbc->isCoordinator) {
        switch (direction) {
        case Direction::LEFT: {
            if (pos == rbc->module->position.offsetY(-1) or
                pos == rbc->module->position.offsetY(1)) {  // FB_Left and BF_LEFT
                rbc->transferCount++;
                stringstream sstream;
                sstream << "TransferCount: " + to_string(rbc->transferCount) << "\n";
                getScheduler()->trace(sstream.str(), rbc->module->blockId, Color(MAGENTA));
                Cell3DPosition targetModule =
                    rbc->seedPosition + (*localRules)[rbc->mvt_it].currentPosition;
                if (rbc->transferCount == 8 and mustHandleBridgeOnAdd(pos)) {
                    // skip to avoid unsupported motion
                    return;
                }
                rbc->sendHandleableMessage(
                        new CoordinateMessage(rbc->operation, targetModule, rbc->module->position,
                                              rbc->mvt_it),
                        rbc->module->getInterface(pos), 100, 200);

                if (rbc->transferCount <= 2) {
                    setMvtItToNextModule(bc);
                } else if (rbc->transferCount < 10) {
                    rbc->mvt_it = 8;
                }
                if (rbc->transferCount == 10) {
                    rbc->mvt_it = 14;
                }
            } else if (rbc->isCoordinator and
                       pos == (rbc->module->position + Cell3DPosition(0, 1, 1)) and
                       direction == Direction::LEFT and isComingFromBack()) {
                rbc->transferCount++;

                stringstream sstream;
                sstream << "TransferCount: " + to_string(rbc->transferCount) << "\n";
                getScheduler()->trace(sstream.str(), rbc->module->blockId, Color(MAGENTA));
                LocalMovement lmvt = (*localRules)[rbc->mvt_it];
                if(lmvt.nextPosition == Cell3DPosition(1,0,2)){
                    rbc->mvt_it++;
                }
                
                Cell3DPosition targetModule =
                    rbc->seedPosition + (*localRules)[rbc->mvt_it].currentPosition;
                rbc->sendHandleableMessage(
                        new CoordinateMessage(rbc->operation, targetModule, rbc->module->position,
                                              rbc->mvt_it),
                        rbc->module->getInterface(pos), 100, 200);
                if (rbc->transferCount <= 2) {
                    setMvtItToNextModule(bc);
                } else if (rbc->transferCount < 10) {
                    rbc->mvt_it = 8;
                }
                if (rbc->transferCount == 10) {
                    rbc->mvt_it = 14;
                }
            }
        } break;

        case Direction::RIGHT: {
            if (pos == rbc->module->position.offsetY(-1) or
                pos == rbc->module->position.offsetY(1)) {  // FB and BF
                rbc->transferCount++;
                stringstream sstream;
                sstream << "TransferCount: " + to_string(rbc->transferCount) << "\n";
                getScheduler()->trace(sstream.str(), rbc->module->blockId, Color(MAGENTA));
                if(mmShape == FRONTBACK){
                    if(not Zeven and rbc->transferCount == 3) setMvtItToNextModule(bc);
                }else {
                    if(Zeven and rbc->transferCount == 3) setMvtItToNextModule(bc);
                }
                    
                Cell3DPosition targetModule =
                    rbc->seedPosition + (*localRules)[rbc->mvt_it].currentPosition;
                rbc->console << "targetModule: " << targetModule << "\n";
                rbc->sendHandleableMessage(
                        new CoordinateMessage(rbc->operation, targetModule, rbc->module->position,
                                              rbc->mvt_it),
                        rbc->module->getInterface(pos), 100, 200);
                if (mmShape == FRONTBACK) {
                    if (Zeven and rbc->transferCount < 10) {
                        setMvtItToNextModule(bc);
                    } else if (not Zeven and rbc->transferCount != 2 and rbc->transferCount < 10) {
                        setMvtItToNextModule(bc);
                    }
                } else {
                    if (Zeven and rbc->transferCount != 2 and rbc->transferCount < 10) {
                        setMvtItToNextModule(bc);
                    } else if(not Zeven and rbc->transferCount < 10) {
                        setMvtItToNextModule(bc);
                    }
                }
            }
        } break;

        case Direction::UP: {
            // Same sequence of movements for all modules.
            if (pos == rbc->module->position.offsetY(-1) or
                pos == rbc->module->position.offsetY(1)) {
                VS_ASSERT(rbc->mvt_it == 0);
                rbc->transferCount++;
                
                if (rbc->transferCount == 8 and mustHandleBridgeOnAdd(pos)) {
                    // skip to avoid unsupported motion
                    return;
                }
                Cell3DPosition targetModule =
                    rbc->seedPosition + (*localRules)[rbc->mvt_it].currentPosition;
                rbc->console << "targetModule: " << targetModule << "\n";
                rbc->sendHandleableMessage(
                        new CoordinateMessage(rbc->operation, targetModule, rbc->module->position,
                                              rbc->mvt_it),
                        rbc->module->getInterface(pos), 100, 200);
            }
        } break;

        case Direction::DOWN: {
            if (pos == rbc->module->position.offsetY(-1) or
                pos == rbc->module->position.offsetY(1)) {
                rbc->transferCount++;
                stringstream sstream;
                sstream << "TransferCount: " + to_string(rbc->transferCount);
                getScheduler()->trace(sstream.str(), rbc->module->blockId, Color(MAGENTA));

                if (mustHandleBridgeOnAdd(pos)) {  // suppose that there is a bridge
                    if (rbc->transferCount == 8)
                        return;
                    else if (rbc->transferCount == 9) {
                        // msg so must jump to next module if previous operation requires bridging
                        setMvtItToNextModule(bc);
                    }
                }
                Cell3DPosition targetModule =
                    rbc->seedPosition + (*localRules)[rbc->mvt_it].currentPosition;
                rbc->sendHandleableMessage(
                    new CoordinateMessage(rbc->operation, targetModule, rbc->module->position,
                                          rbc->mvt_it),
                    rbc->module->getInterface(pos), 100, 200);
                if (rbc->transferCount > 1) {
                    setMvtItToNextModule(bc);
                    rbc->console << "mvt_itX: " << rbc->mvt_it << "\n";
                }
            } else if ((pos == rbc->seedPosition + Cell3DPosition(1, 0, 0)) and rbc->getPreviousOpDir() == Direction::RIGHT) {
                // When coming from right and trandfering down modules will be connected below to
                // the coordinator at position rbc->seedPosition + Cell3DPosition(1,0,0)
                rbc->transferCount++;
                stringstream sstream;
                sstream << "TransferCount: " + to_string(rbc->transferCount);
                getScheduler()->trace(sstream.str(), rbc->module->blockId, Color(MAGENTA));
                if (rbc->transferCount == 2 or rbc->transferCount > 10) return;
                rbc->mvt_it++;
                Cell3DPosition targetModule =
                    rbc->seedPosition + (*localRules)[rbc->mvt_it].currentPosition;
                rbc->sendHandleableMessage(
                    new CoordinateMessage(rbc->operation, targetModule, rbc->module->position,
                                          rbc->mvt_it),
                    rbc->module->getInterface(pos), 100, 200);
                if (rbc->transferCount > 1) {
                    setMvtItToNextModule(bc);
                    rbc->console << "mvt_itX: " << rbc->mvt_it << "\n";
                }
            }

        } break;

        case Direction::BACK: {
            if (pos == rbc->module->position.offsetY(-1) or
                pos == rbc->module->position.offsetY(1)) {
                rbc->transferCount++;
                stringstream sstream;
                sstream << "TransferCount: " + to_string(rbc->transferCount);
                getScheduler()->trace(sstream.str(), rbc->module->blockId, Color(MAGENTA));
                if (mmShape == FRONTBACK) {
                    if (rbc->transferCount == 8 and mustHandleBridgeOnAdd(pos)) {
                        // skip to avoid unsupported motion
                        return;
                    }
                    if(rbc->transferCount == 3){
                        rbc->console << "mvt_it3: " << rbc->mvt_it << "\n";
                        return;
                    } 
                    Cell3DPosition targetModule =
                        rbc->seedPosition + (*localRules)[rbc->mvt_it].currentPosition;
                    rbc->sendHandleableMessage(
                        new CoordinateMessage(rbc->operation, targetModule, rbc->module->position,
                                              rbc->mvt_it),
                        rbc->module->getInterface(targetModule), 100, 200);
                    if(rbc->transferCount == 2) 
                        // mvt_it will be updated when the coordinateBack is received
                        return;
                    if (rbc->transferCount == 1) {
                        setMvtItToNextModule(bc);                        
                    } else if (rbc->transferCount < 10) {
                        rbc->mvt_it = 7;
                    }
                    if (rbc->transferCount == 10) {
                        rbc->mvt_it = 11;
                    }
                } else if(mmShape == BACKFRONT) {
                    if (not comingFromBack) {  // mmShape = BACKFRONT
                        if (rbc->transferCount == 8 and mustHandleBridgeOnAdd(pos)) {
                            // skip to avoid unsupported motion
                            rbc->console << "skip\n";
                            return;
                        }
                        //if (rbc->transferCount > 1) return;
                        Cell3DPosition targetModule =
                            rbc->seedPosition + (*localRules)[rbc->mvt_it].currentPosition;
                        if(rbc->transferCount == 10)
                            return;
                        rbc->sendHandleableMessage(
                            new CoordinateMessage(rbc->operation, targetModule,
                                                  rbc->module->position, rbc->mvt_it),
                            rbc->module->getInterface(targetModule), 100, 200);
                        if(rbc->transferCount < 8)
                            setMvtItToNextModule(bc);
                    }
                }
            } else if (pos == rbc->module->position + Cell3DPosition(0, 1, 1)) {
                if (comingFromBack and mmShape == BACKFRONT) {
                    rbc->transferCount++;
                    stringstream sstream;
                    sstream << "TransferCount: " + to_string(rbc->transferCount) << "\n";
                    Cell3DPosition targetModule =
                        rbc->seedPosition + (*localRules)[rbc->mvt_it].currentPosition;
                    getScheduler()->trace(sstream.str(), rbc->module->blockId, Color(BLUE));
                    if (rbc->transferCount == 13) {
                        //Wait for coordinateBack to avoid blocking
                        return; 
                    }
                    rbc->sendHandleableMessage(
                        new CoordinateMessage(rbc->operation, targetModule, rbc->module->position,
                                              rbc->mvt_it),
                        rbc->module->getInterface(targetModule), 100, 200);
                    if (rbc->transferCount < 2) {
                        setMvtItToNextModule(bc);
                    } else if (rbc->transferCount < 11) {
                        rbc->mvt_it = 4;
                    }
                    if (rbc->transferCount > 11) {
                        rbc->mvt_it = 8;
                    }
                }
            }
        } break;

        case Direction::FRONT: {
            if (pos == rbc->module->position.offsetY(-1) or
                pos == rbc->module->position.offsetY(1)) {
                rbc->transferCount++;
                stringstream sstream;
                sstream << "TransferCount: " + to_string(rbc->transferCount);
                getScheduler()->trace(sstream.str(), rbc->module->blockId, Color(MAGENTA));

                if(mmShape == FRONTBACK) {
                    if (prevOpDirection != Direction::FRONT) { 
                        if (rbc->transferCount == 8 and mustHandleBridgeOnAdd(pos)) {
                            // skip to avoid unsupported motion
                            rbc->console << "skip\n";
                            return;
                        }
                        //if (rbc->transferCount > 1) return;
                        Cell3DPosition targetModule =
                            rbc->seedPosition + (*localRules)[rbc->mvt_it].currentPosition;
                        if(rbc->transferCount == 10)
                            return;
                        rbc->sendHandleableMessage(
                            new CoordinateMessage(rbc->operation, targetModule,
                                                  rbc->module->position, rbc->mvt_it),
                            rbc->module->getInterface(targetModule), 100, 200);
                        if(rbc->transferCount < 8)
                            setMvtItToNextModule(bc);
                    }
                } else {  // BACKFRONT
                    if (rbc->transferCount == 8 and mustHandleBridgeOnAdd(pos)) {
                        // skip to avoid unsupported motion
                        return;
                    }
                    if (rbc->transferCount == 3) {
                        rbc->console << "mvt_it3: " << rbc->mvt_it << "\n";
                        return;
                    }
                    Cell3DPosition targetModule =
                        rbc->seedPosition + (*localRules)[rbc->mvt_it].currentPosition;
                    rbc->sendHandleableMessage(
                        new CoordinateMessage(rbc->operation, targetModule, rbc->module->position,
                                              rbc->mvt_it),
                        rbc->module->getInterface(targetModule), 100, 200);
                    if (rbc->transferCount == 2)
                        // mvt_it will be updated when the coordinateBack is received
                        return;
                    if (rbc->transferCount == 1) {
                        setMvtItToNextModule(bc);
                    } else if (rbc->transferCount < 10) {
                        rbc->mvt_it = 7;
                    }
                    if (rbc->transferCount == 10) {
                        rbc->mvt_it = 11;
                    }
                }
            } else if(pos == rbc->module->position.offsetZ(1)){
                if(rbc->getPreviousOpDir() == Direction::FRONT and mmShape == FRONTBACK) {
                    rbc->transferCount++;
                    stringstream sstream;
                    sstream << "TransferCount: " + to_string(rbc->transferCount) << "\n";
                    Cell3DPosition targetModule =
                        rbc->seedPosition + (*localRules)[rbc->mvt_it].currentPosition;
                    getScheduler()->trace(sstream.str(), rbc->module->blockId, Color(BLUE));
                    if (rbc->transferCount == 13) {
                        //Wait for coordinateBack to avoid blocking
                        return; 
                    }
                    rbc->sendHandleableMessage(
                        new CoordinateMessage(rbc->operation, targetModule, rbc->module->position,
                                              rbc->mvt_it),
                        rbc->module->getInterface(targetModule), 100, 200);
                    if (rbc->transferCount < 2) {
                        setMvtItToNextModule(bc);
                    } else if (rbc->transferCount < 11) {
                        rbc->mvt_it = 4;
                    }
                    if (rbc->transferCount > 11) {
                        rbc->mvt_it = 8;
                    }
                }
            }
        } break;

        default:
            break;
        }

    } else if (rbc->movingState == WAITING) {
        // Special logic for bridge: wait for all modules to pass
        // the bridge then transfer the bridge
        rbc->transferCount++;
        rbc->scheduler->trace("transferCount: " + to_string(rbc->transferCount),
                              rbc->module->blockId, Color(CYAN));
        if ((rbc->transferCount == 33 and (direction == Direction::LEFT)) or
            /*(rbc->transferCount == 30 and direction == Direction::RIGHT) or*/
            (rbc->transferCount == 30 and getMMShape() == FRONTBACK and
             (direction == Direction::BACK or direction == Direction::FRONT)) or
            ((rbc->transferCount == 19 and rbc->relativePos() == Cell3DPosition(0, 1, 2) and
              getMMShape() == BACKFRONT and comingFromBack and
              (direction == Direction::BACK or direction == Direction::FRONT)) or
             (rbc->transferCount == 30 and getMMShape() == BACKFRONT and
              direction == Direction::FRONT) or
             (rbc->transferCount == 19 and rbc->relativePos() == Cell3DPosition(0, -1, 2) and
              prevOpDirection == Direction::FRONT and getMMShape() == FRONTBACK) and
                 direction == Direction::FRONT)) {  // when all modules passed the bridge
            rbc->sendHandleableMessage(
                new CoordinateBackMessage(0, rbc->coordinatorPosition),
                rbc->module->getInterface(rbc->nearestPositionTo(rbc->coordinatorPosition)), 100,
                200);
        }
    }
}

void Transfer_Operation::updateState(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode* rbc = static_cast<RePoStBlockCode*>(bc);
    switch (direction ) {
    case Direction::LEFT : {
        if (mmShape == FRONTBACK) {
            rbc->shapeState = BACKFRONT;
            Init::getNeighborMMSeedPos(rbc->seedPosition, rbc->MMPosition, Direction::LEFT,
                                       rbc->seedPosition);
            rbc->MMPosition = rbc->MMPosition.offsetX(-1);
            rbc->initialPosition = rbc->module->position - rbc->seedPosition;
            break;
        } else if (mmShape == BACKFRONT) {
            rbc->shapeState = FRONTBACK;
            Init::getNeighborMMSeedPos(rbc->seedPosition, rbc->MMPosition, Direction::LEFT,
                                       rbc->seedPosition);
            rbc->MMPosition = rbc->MMPosition.offsetX(-1);
            rbc->initialPosition = rbc->module->position - rbc->seedPosition;
            break;
        }
    } break;
    case Direction::RIGHT : {
         (rbc->shapeState == FRONTBACK) ? rbc->shapeState = BACKFRONT : rbc->shapeState = FRONTBACK;
        Init::getNeighborMMSeedPos(rbc->seedPosition, rbc->MMPosition, Direction::RIGHT,
                                   rbc->seedPosition);
        rbc->MMPosition = rbc->MMPosition.offsetX(1);
        rbc->initialPosition = rbc->module->position - rbc->seedPosition;
    } break;
    case Direction::UP : {
        Init::getNeighborMMSeedPos(rbc->seedPosition, rbc->MMPosition, Direction::UP,
                                    rbc->seedPosition);
        rbc->MMPosition = rbc->MMPosition.offsetZ(1);
        rbc->initialPosition = rbc->module->position - rbc->seedPosition;
    } break;

    case Direction::DOWN: {
        if(rbc->mvt_it == 2 and mmShape == FRONTBACK and Zeven) return;
        if(rbc->mvt_it == 2 and mmShape == BACKFRONT and not Zeven) return;
        if(rbc->mvt_it == localRules->size() -1) return;
        Init::getNeighborMMSeedPos(rbc->seedPosition, rbc->MMPosition, Direction::DOWN,
                                    rbc->seedPosition);
        rbc->MMPosition = rbc->MMPosition.offsetZ(-1);
        rbc->initialPosition = rbc->module->position - rbc->seedPosition;
    } break;

    case Direction::BACK : {
         (rbc->shapeState == FRONTBACK) ? rbc->shapeState = BACKFRONT : rbc->shapeState = FRONTBACK;
        Init::getNeighborMMSeedPos(rbc->seedPosition, rbc->MMPosition, Direction::BACK,
                                   rbc->seedPosition);
        rbc->MMPosition = rbc->MMPosition.offsetY(1);
        rbc->initialPosition = rbc->module->position - rbc->seedPosition;
    } break;

    case Direction::FRONT: {
        (rbc->shapeState == FRONTBACK) ? rbc->shapeState = BACKFRONT : rbc->shapeState = FRONTBACK;
        Init::getNeighborMMSeedPos(rbc->seedPosition, rbc->MMPosition, Direction::FRONT,
                                   rbc->seedPosition);
        rbc->MMPosition = rbc->MMPosition.offsetY(-1);
        rbc->initialPosition = rbc->module->position - rbc->seedPosition;
    } break;
    default:
        break;
    }
}

bool Transfer_Operation::mustSendCoordinateBack(BaseSimulator::BlockCode* bc) {
    RePoStBlockCode* rbc = static_cast<RePoStBlockCode*>(bc);
    if (rbc->isCoordinator) return false;
    if ((direction == Direction::LEFT) and rbc->mvt_it >= 14) {
        return true;
    } else if(direction == Direction::RIGHT) {  
        if(/*Zeven and*/ rbc->mvt_it >= 46) return true;
        else if(rbc->mvt_it == 2) return true;
    } else if (direction == Direction::BACK) {
        if (mmShape == BACKFRONT) {
            if(rbc->getPreviousOpDir() == Direction::RIGHT) {
                if (not comingFromBack and rbc->mvt_it >= 39) return true;
            } else {
                if (not comingFromBack and rbc->mvt_it >= 26) return true;
            }
            

           // if (not comingFromBack and rbc->mvt_it >= 26) return true;
            
            //if (not comingFromBack and rbc->mvt_it >= 39) return true;
            if (comingFromBack and rbc->mvt_it > 8) return true;
        } else if (mmShape == FRONTBACK) {
            if (not comingFromBack and rbc->mvt_it == 4 or rbc->mvt_it == 6) return true;
        }
    } else if (direction == Direction::DOWN and rbc->mvt_it == 2) {
        return true;
    } else if (direction == Direction::FRONT) {
        if(mmShape == FRONTBACK) {
             if (prevOpDirection != Direction::FRONT) {
                 if(rbc->mvt_it >= 26 and prevOpDirection != Direction::RIGHT) return true;
                 else if(prevOpDirection == Direction::RIGHT and rbc->mvt_it >= 42) return true;
             }  
             if (prevOpDirection == Direction::FRONT and rbc->mvt_it > 8) return true;
        } else { //BACKFRONT
            if(/*not comingFromBack and*/ rbc->mvt_it == 4 or rbc->mvt_it == 6) return true;
        }
    }
    return false;
}

bool Transfer_Operation::handleBridgeMovements(BaseSimulator::BlockCode* bc) {
    RePoStBlockCode* rbc = static_cast<RePoStBlockCode*>(bc);
    LocalMovement lmvt = (*localRules)[rbc->mvt_it];
 
    if(lmvt.currentPosition == lmvt.nextPosition /**and not isFirstTransferOperation**/) {
        rbc->mvt_it++;
        rbc->movingState = IN_POSITION;
       // rbc->updateState();
       rbc->sendHandleableMessage(new CoordinateBackMessage(1, rbc->coordinatorPosition),
        rbc->module->getInterface(rbc->nearestPositionTo(rbc->coordinatorPosition)) ,100, 200);;

        rbc->module->getInterface(rbc->module->position.offsetY(-1))->isConnected()
            ? rbc->coordinatorPosition = rbc->module->position.offsetY(-1)
            : rbc->coordinatorPosition = rbc->module->position.offsetY(1);
        RePoStBlockCode* coordinatorBlock = static_cast<RePoStBlockCode*>(
            BaseSimulator::getWorld()->getBlockByPosition(rbc->coordinatorPosition)->blockCode);
        if (coordinatorBlock->operation->isTransfer() and
            coordinatorBlock->operation->getDirection() == Direction::BACK and
            static_cast<Transfer_Operation*>(coordinatorBlock->operation)->isComingFromBack()) {
            return true;
        }
        rbc->sendHandleableMessage(new CoordinateBackMessage(0, rbc->coordinatorPosition),
            rbc->module->getInterface(rbc->coordinatorPosition), 100, 200);

        return true;
    } else {
        return false;
    }
    // if(direction == Direction::LEFT) {
    //     if(rbc->movingState == WAITING and rbc->relativePos() == Cell3DPosition(-2,0,2)) {
    //         rbc->console << "mvt_it!!: " << rbc->mvt_it << "\n";
    //         return true;
    //     }
    //     if(rbc->movingState == WAITING 
    //         and (rbc->module->position - rbc->seedPosition).pt[0] <= -2
    //         and isFirstTransferOperation
    //     ) {
    //         rbc->movingState = IN_POSITION;
    //         rbc->updateState();
    //         cout << rbc->module->blockId << ": Last operation Done2!!!\n";
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

Build_Operation::Build_Operation (Direction _direction, MMShape _mmShape, Direction _prevOpDirection, bool _comingFromBack, int Z)
    :Operation(_direction, _mmShape,_prevOpDirection, Z), comingFromBack(_comingFromBack) {
   
    switch (direction) {
    case Direction::UP: {
        // motions rules are set for Z even
        if(Zeven) {
            if(mmShape == BACKFRONT) {
                if(comingFromBack) localRules.reset(&LocalRules_BF_Build_Up_ComingFromBack);
                else localRules.reset(&LocalRules_BF_Build_Up);
            } else if(mmShape == FRONTBACK) {
                localRules.reset(&LocalRules_FB_Build_Up);
            }
        } else { // Z odd
            if(mmShape == BACKFRONT) {
                if(comingFromBack)  localRules.reset(&LocalRules_BF_Build_Up_ZOdd_ComingFromBack);
                else localRules.reset(&LocalRules_BF_Build_Up_ZOdd);
            } else if(mmShape == FRONTBACK) {
                localRules.reset(&LocalRules_FB_Build_Up_ZOdd);
            }
        }
    } break;
    case Direction::BACK: {
        if(mmShape == FRONTBACK) {
            localRules.reset(&LocalRules_FB_Build_Back);
        } else if(mmShape == BACKFRONT) {
            comingFromBack ? localRules.reset(&LocalRules_BF_Build_Back_ComingFromBack):
                localRules.reset(&LocalRules_BF_Build_Back);
        }
    } break;
    
    default:
        VS_ASSERT_MSG(false, "Not implemented");
    } 
}
 
Build_Operation::~Build_Operation () {}

void Build_Operation::handleAddNeighborEvent(BaseSimulator::BlockCode* bc,
                                             const Cell3DPosition& pos) {
    RePoStBlockCode* rbc = static_cast<RePoStBlockCode*>(bc);
     RePoStBlockCode* posbc = static_cast<RePoStBlockCode*>(
        BaseSimulator::getWorld()->getBlockByPosition(pos)->blockCode);
    if (rbc->isCoordinator and abs(pos.pt[1] - rbc->module->position.pt[1]) == 1 and
        abs(pos.pt[2] - rbc->module->position.pt[2]) == 0 and
        rbc->mvt_it < localRules->size() and not isComingFromBack()) {
        rbc->transferCount++; 
        getScheduler()->trace("transferCount: " + to_string(rbc->transferCount), rbc->module->blockId, Color(MAGENTA));
        Cell3DPosition targetModule =
            rbc->seedPosition + (*localRules)[rbc->mvt_it].currentPosition;
        if (mustHandleBridgeOnAdd(pos)) {  // suppose that there is a bridge
            if(rbc->transferCount == 8) return;
            else if(rbc->transferCount == 9) {
                // msg so must jump to next module if previous operation requires bridging
                setMvtItToNextModule(bc);
            }
        }
        rbc->sendHandleableMessage(
                        new CoordinateMessage(rbc->operation, targetModule, rbc->module->position,
                                              rbc->mvt_it),
                        rbc->module->getInterface(pos), 100, 200);
        setMvtItToNextModule(bc);

        rbc->console << "mvt_it: " << rbc->mvt_it << "\n";
    } else if (rbc->isCoordinator and pos == (rbc->module->position + Cell3DPosition(0, 1, 1)) and
               (direction == Direction::BACK or direction == Direction::UP) and mmShape == BACKFRONT and isComingFromBack() ) {
        if(rbc->transferCount >= 10) return;
        rbc->transferCount++; 
        getScheduler()->trace("transferCount: " + to_string(rbc->transferCount), rbc->module->blockId, Color(MAGENTA));
        Cell3DPosition targetModule =
            rbc->seedPosition + (*localRules)[rbc->mvt_it].currentPosition;
        rbc->sendHandleableMessage(
                        new CoordinateMessage(rbc->operation, targetModule, rbc->module->position,
                                              rbc->mvt_it),
                        rbc->module->getInterface(pos), 100, 200);
        if(rbc->transferCount < 10)
            setMvtItToNextModule(bc);
    }
    if (rbc->movingState == WAITING) {
        rbc->transferCount++;
        rbc->scheduler->trace("transferCount: " + to_string(rbc->transferCount),
                               rbc->module->blockId, Color(CYAN));
        if (direction == Direction::BACK and mmShape == FRONTBACK and rbc->transferCount == 27) {
                rbc->sendHandleableMessage(new CoordinateBackMessage(0, rbc->coordinatorPosition),
                    rbc->interfaceTo(rbc->coordinatorPosition), 100, 200);
            }
        else if (direction == Direction::BACK and mmShape == BACKFRONT and
                 rbc->transferCount == 19) {
                rbc->sendHandleableMessage(new CoordinateBackMessage(0, rbc->coordinatorPosition),
                   rbc->interfaceTo(rbc->coordinatorPosition), 100,200 );
        }
    }
}

void Build_Operation::updateState(BaseSimulator::BlockCode* bc) {
    RePoStBlockCode* rbc = static_cast<RePoStBlockCode*>(bc);
    switch (direction) {
    case Direction::UP: {
        if(mmShape == FRONTBACK) {
            rbc->shapeState = FRONTBACK;
            Init::getNeighborMMSeedPos(rbc->seedPosition, rbc->MMPosition, Direction::UP, rbc->seedPosition);
            rbc->MMPosition = rbc->MMPosition.offsetZ(1);
            rbc->initialPosition = rbc->module->position - rbc->seedPosition;
            break;
        } else if(mmShape == BACKFRONT){
            rbc->shapeState = BACKFRONT;
            Init::getNeighborMMSeedPos(rbc->seedPosition, rbc->MMPosition, Direction::UP, rbc->seedPosition);
            rbc->MMPosition = rbc->MMPosition.offsetZ(1);
            rbc->initialPosition = rbc->module->position - rbc->seedPosition;
            break;
        }
        
    } break;
     case Direction::BACK: {
        (rbc->shapeState == FRONTBACK) ? rbc->shapeState = BACKFRONT : rbc->shapeState = FRONTBACK;
        Init::getNeighborMMSeedPos(rbc->seedPosition, rbc->MMPosition, Direction::BACK, rbc->seedPosition);
        rbc->MMPosition = rbc->MMPosition.offsetY(1);
        rbc->initialPosition = rbc->module->position - rbc->seedPosition;
     } break;   

    
    default:
        break;
    }
}

bool Build_Operation::mustSendCoordinateBack(BaseSimulator::BlockCode* bc) {
    RePoStBlockCode* rbc = static_cast<RePoStBlockCode*>(bc);
    if(direction == Direction::BACK and mmShape == FRONTBACK and rbc->mvt_it == 49) return true;
    else if(isComingFromBack() and direction != Direction::UP and rbc->mvt_it >= 37 and rbc->mvt_it != localRules->size()-1) return true;
    else if(direction == Direction::BACK and mmShape == BACKFRONT and rbc->mvt_it >= 42 and rbc->mvt_it != localRules->size()-1)  return true;
    return false;
}


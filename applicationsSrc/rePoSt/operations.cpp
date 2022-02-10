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

    if (posbc->operation->isTransfer()) {
        if (posbc->operation->getDirection() == Direction::LEFT and
            posbc->operation->getMMShape() == FRONTBACK and posbc->operation->isZeven() and
            static_cast<Transfer_Operation*>(posbc->operation)->getNextOpDir() == Direction::DOWN)
            return false;
        if (posbc->operation->getDirection() == Direction::LEFT) {
            return true;
        }
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


void Operation::updateProbingPoints(BaseSimulator::BlockCode *bc, vector<Catoms3D::Catoms3DBlock *> &latchingPoints,
                                    const Cell3DPosition &targetPos) const {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);
    if (targetPos - rbc.seedPosition == Cell3DPosition(-2, 1, 2)) {
        if (rbc.lattice->cellHasBlock(rbc.seedPosition + Cell3DPosition(-3, 0, 2)) and
            isTransfer() and direction == Direction::LEFT) {
            // Avoid blocking when passing the bridge if the next operation direction is back
            RePoStBlockCode &rlp = *static_cast<RePoStBlockCode *>(
                    rbc.lattice->getBlock(rbc.seedPosition + Cell3DPosition(-3, 0, 2))->blockCode);
            if (rlp.movingState == WAITING) {
                latchingPoints.push_back(static_cast<Catoms3DBlock *>(
                                                 rbc.lattice->getBlock(rbc.seedPosition + Cell3DPosition(-3, 0, 2))));
            }
            return;
        }
        if (rbc.lattice->cellHasBlock(rbc.seedPosition + Cell3DPosition(-4, 1, 2))) {
        }
    }

    /* ------------------ Avoid blocking when coming from back ------------------ */
    if (direction == BACK and rbc.shapeState == BACKFRONT) {
        if (rbc.relativePos() == Cell3DPosition(0, 2, 2) and
                rbc.lattice->cellHasBlock(rbc.module->position + Cell3DPosition(1, 1, -1))) {
            latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                                             rbc.lattice->getBlock(rbc.module->position + Cell3DPosition(1, 1, -1))));
        }
        if (rbc.relativePos() == Cell3DPosition(0, 2, 1) and isTransfer()) {
            latchingPoints.clear();
        }
        if (rbc.relativePos() == Cell3DPosition(-1, 3, 3)) {
            latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                                             rbc.lattice->getBlock(rbc.module->position + Cell3DPosition(2, 0, -2))));
        }
    }
    /* -------------------------------------------------------------------------- */
}

/************************************************************************
 *************************  DISMANTLE OPERATION  ************************
 ***********************************************************************/

Dismantle_Operation::Dismantle_Operation (Direction _direction, MMShape _mmShape, Direction _prevOpDirection, int Z, bool _filled, bool _fill)
    :Operation(_direction, _mmShape, _prevOpDirection, Z), filled(_filled) , fill(_fill) {
    
    switch (direction) {
    case Direction::LEFT: {
        if(mmShape == BACKFRONT) {
            if(filled){
                if(fill)  {
                    VS_ASSERT_MSG(false, "Not implemented");
                } else {
                    Zeven ? localRules.reset(&LocalRules_BF_DismantleFilled_Left_Zeven)
                          : VS_ASSERT_MSG(false, "Not implemented");;
                }

            } else { // not filled
                if(fill) {
                    Zeven ? localRules.reset(&LocalRules_BF_DismantleAndFill_Left_Zeven) :
                    localRules.reset(&LocalRules_BF_DismantleAndFill_Left_Zodd);
                } else {
                    Zeven ? localRules.reset(&LocalRules_BF_Dismantle_Left) :
                        localRules.reset(&LocalRules_BF_Dismantle_Left_Zodd);
                }

            }
        } else if(mmShape == FRONTBACK) {
            if(filled) {
                VS_ASSERT_MSG(false, "Not implemented");
            }
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
            Zeven ? localRules.reset(&LocalRules_BF_Dismantle_Up_Zeven):
                localRules.reset(&LocalRules_BF_Dismantle_Up_Zodd);
        } else if(mmShape == FRONTBACK) {
             Zeven ? VS_ASSERT_MSG(false, "Not implemented"): 
                localRules.reset(&LocalRules_FB_Dismantle_Up_Zodd);
        }
    } break;

    case Direction::DOWN: {
        if (mmShape == BACKFRONT) {
            if (filled) {
                Zeven ? VS_ASSERT_MSG(false, "Not implemented") :
                localRules.reset(&LocalRules_BF_DismantleFilled_Down_Zodd);
            } else {
                Zeven ? localRules.reset(&LocalRules_BF_Dismantle_Down_Zeven) :
                VS_ASSERT_MSG(false, "Not implemented");
            }
        } else { //FRONTBACK
            if (filled) {
                Zeven ? localRules.reset(&LocalRules_FB_DismantleFilled_Down_Even):
                VS_ASSERT_MSG(false, "Not implemented") ;
            } else {
                Zeven ? localRules.reset(&LocalRules_FB_Dismantle_Down_Zeven):
                localRules.reset(&LocalRules_FB_Dismantle_Down_Zodd);
            }
        }
    } break;

        case Direction::RIGHT: {
            if (mmShape == BACKFRONT) {
                Zeven ? localRules.reset(&LocalRules_BF_Dismantle_Right_Zeven) :
                localRules.reset(&LocalRules_BF_Dismantle_Right_Zodd);
            } else { //FRONTBACK
                Zeven ? localRules.reset(&LocalRules_FB_Dismantle_Right_Zeven) :
                localRules.reset(&LocalRules_FB_Dismantle_Right_Zodd);
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
    if(filled) {
        rbc->fillingState = FULL;
    }
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

        case Direction::DOWN: {
            Init::getNeighborMMSeedPos(rbc->seedPosition, rbc->MMPosition, Direction::DOWN,
                                       rbc->seedPosition);
            rbc->MMPosition = rbc->MMPosition.offsetZ(-1);
            rbc->initialPosition = rbc->module->position - rbc->seedPosition;
        } break;

        case Direction::RIGHT: {
            (rbc->shapeState == FRONTBACK) ? rbc->shapeState = BACKFRONT
                                           : rbc->shapeState = FRONTBACK;
            Init::getNeighborMMSeedPos(rbc->seedPosition, rbc->MMPosition, Direction::RIGHT,
                                       rbc->seedPosition);
            rbc->MMPosition = rbc->MMPosition.offsetX(1);
            rbc->initialPosition = rbc->module->position - rbc->seedPosition;
        } break;

        default:
            VS_ASSERT_MSG(false, "Not implemented");
    }
}

bool Dismantle_Operation::mustSendCoordinateBack(BaseSimulator::BlockCode* bc) {
    RePoStBlockCode* rbc = static_cast<RePoStBlockCode*>(bc);
    return !rbc->isCoordinator;
}

void
Dismantle_Operation::updateProbingPoints(BaseSimulator::BlockCode *bc, vector<Catoms3D::Catoms3DBlock *> &latchingPoints,
                                         const Cell3DPosition &targetPos) const {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);
    Operation::updateProbingPoints(bc, latchingPoints, targetPos);
    switch (direction) {
        case Direction::LEFT: {
            /* ------ Avoid blocking when FB dismantling left then transferring down ----- */
            if (targetPos - rbc.seedPosition == Cell3DPosition(-1, 0, 2) and
                rbc.lattice->cellHasBlock(rbc.seedPosition + Cell3DPosition(-2, 0, 1))) {
                latchingPoints.push_back(static_cast<Catoms3DBlock *>(
                                                 rbc.lattice->getBlock(rbc.seedPosition + Cell3DPosition(-2, 0, 1))));
            }
        } break;

        case Direction::DOWN: {
            // Avoid blocing when BF dismantling down then going right via bridge; Zeven
            if(targetPos - rbc.seedPosition == Cell3DPosition(1,-1,-1) and rbc.getNextOpDir() == Direction::RIGHT
                and rbc.lattice->cellHasBlock(rbc.seedPosition + Cell3DPosition(2, -1, -3)) ) {
                latchingPoints.push_back(static_cast<Catoms3DBlock *>(
                                                 rbc.lattice->getBlock(rbc.seedPosition + Cell3DPosition(2, -1, -3))));
            }
        } break;
        default: return;
    }
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

        case Direction::UP: {
            if(mmShape == BACKFRONT) {
                Zeven ? localRules.reset(&LocalRules_BF_Fill_Up_Zeven)
                    : VS_ASSERT_MSG(false, "Not implemented");
            } else { //FRONTBACK
                Zeven ? localRules.reset(&LocalRules_FB_Fill_Up_Zeven)
                    : VS_ASSERT_MSG(false, "Not implemented");
            }
        } break;

        default:
            VS_ASSERT_MSG(false, "Not implemented");
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
                    (pos == rbc->module->position.offsetY(1) and
                        prevOpDirection != Direction::BACK)) {
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

            case Direction::UP: {
                if (pos == rbc->module->position.offsetY(-1) or
                    pos == rbc->module->position.offsetY(1)) {
                    rbc->transferCount++;
                    rbc->console << "mvt_it1: " << rbc->mvt_it << "\n";

                    getScheduler()->trace("transferCount: " + to_string(rbc->transferCount),
                                          rbc->module->blockId, Color(MAGENTA));

                    if (mustHandleBridgeOnAdd(pos)) {  // suppose that there is a bridge
                        if (rbc->transferCount == 8)
                            return;
                        else if (rbc->transferCount == 9) {
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
                    if (rbc->transferCount < 10) {
                        setMvtItToNextModule(bc);
                        rbc->console << "mvt_it2: " << rbc->mvt_it << "\n";
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
    rbc->fillingState = FULL;
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

        case Direction::UP: {
            Init::getNeighborMMSeedPos(rbc->seedPosition, rbc->MMPosition, Direction::UP,
                                       rbc->seedPosition);
            rbc->MMPosition = rbc->MMPosition.offsetZ(1);
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
    } else if(direction == Direction::UP) {
        if(Zeven) {
            if(mmShape == BACKFRONT and rbc->mvt_it >= 61) return true;
        }
    }

    return false;
}

void Fill_Operation::updateProbingPoints(BaseSimulator::BlockCode *bc, vector<Catoms3D::Catoms3DBlock *> &latchingPoints,
                                         const Cell3DPosition &targetPos) const {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);
    Operation::updateProbingPoints(bc, latchingPoints, targetPos);
    switch (direction) {
        case Direction::LEFT: {
            if (rbc.relativePos() == Cell3DPosition(1, -1, 1) or
                rbc.relativePos() == Cell3DPosition(1, 0, 1)) {
                latchingPoints.clear();
                return;
            }
            if (mmShape == BACKFRONT and isZeven() and
                rbc.mvt_it >= 39) {
                latchingPoints.clear();
                return;
            }

             if(mmShape == FRONTBACK and not isZeven() and rbc.mvt_it >= 37) {
                latchingPoints.clear();
                return;
            }
        }
            break;

        case Direction::BACK: {
            if (mmShape == BACKFRONT and
                getPrevOpDirection() != Direction::BACK) {
                if (rbc.relativePos() == Cell3DPosition(0, 1, 2) and rbc.mvt_it >= 34) {
                    latchingPoints.push_back(dynamic_cast<Catoms3D::Catoms3DBlock *>(
                                                     rbc.lattice->getBlock(
                                                             rbc.seedPosition + Cell3DPosition(0, 3, 0))));
                    latchingPoints.push_back(dynamic_cast<Catoms3D::Catoms3DBlock *>(
                                                     rbc.lattice->getBlock(
                                                             rbc.seedPosition + Cell3DPosition(1, 3, 0))));
                    latchingPoints.push_back(dynamic_cast<Catoms3D::Catoms3DBlock *>(
                                                     rbc.lattice->getBlock(
                                                             rbc.seedPosition + Cell3DPosition(1, 3, 1))));
                    return;
                }
                if (rbc.relativePos().pt[1] >= 3) {
                    latchingPoints.clear();
                    return;
                }
            }
            if (mmShape == FRONTBACK and
                rbc.relativePos() == Cell3DPosition(1, 3, 2) and rbc.mvt_it >= 23) {
                // Avoid deadlock when the MM on top is being filled too
                latchingPoints.clear();
                return;
            }
        }
            break;

        default:
            break;
    }
}

/************************************************************************
 ***************************  TRANSFER OPERATION  ***********************
 ***********************************************************************/

Transfer_Operation::Transfer_Operation(Direction _direction, MMShape _mmShape, Direction _prevOpDirection, Direction _nextOpDir, bool _comingFromBack, int Z) 
    :Operation(_direction, _mmShape,_prevOpDirection, Z), comingFromBack(_comingFromBack), nextOpDir(_nextOpDir) {
    switch (direction){
    case Direction::LEFT: {
        if(mmShape == FRONTBACK){
             if(Zeven and nextOpDir == Direction::DOWN){
                localRules.reset(&LocalRules_FB_Transfer_Left_GoingDown);
            } else {
                localRules.reset(&LocalRules_FB_Transfer_Left);
            }
                
            
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
            } else  { //mmsShape = BACKFRONT
                localRules.reset(&LocalRules_BF_Transfer_Up_Zeven);
            }
        } else { //Zodd
            if(mmShape == FRONTBACK) {
                localRules.reset(&LocalRules_FB_Transfer_Up_Zodd);
            } else  { //mmsShape = BACKFRONT
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
                if(prevOpDirection == Direction::LEFT and not Zeven) {
                    localRules.reset(&LocalRules_BF_Transfer_Back_ComingFromLeft_Zodd);
                } else {
                    localRules.reset(&LocalRules_BF_Transfer_Back);
                }
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
                if(mmShape == FRONTBACK and nextOpDir == Direction::DOWN and Zeven) {
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
                    rbc->sendHandleableMessage(
                        new CoordinateMessage(rbc->operation, targetModule, rbc->module->position,
                                              rbc->mvt_it),
                        rbc->module->getInterface(pos), 100, 200);
                    if (rbc->transferCount < 10) {
                        setMvtItToNextModule(bc);
                    }
                    return;
                }
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
                } else if(mmShape == BACKFRONT and getPrevOpDirection() != Direction::BACK) {
                    if (getPrevOpDirection() != Direction::LEFT) {
                        if (rbc->transferCount == 8 and mustHandleBridgeOnAdd(pos)) {
                            // skip to avoid unsupported motion
                            rbc->console << "skip\n";
                            return;
                        }
                    } else {
                        if (mustHandleBridgeOnAdd(pos)) {  // suppose that there is a bridge
                            if (rbc->transferCount == 8)
                                return;
                            else if (rbc->transferCount == 9) {
                                // msg so must jump to next module if previous operation requires
                                // bridging
                                setMvtItToNextModule(bc);
                            }
                        }
                    }

                        //if (rbc->transferCount > 1) return;
                        Cell3DPosition targetModule =
                            rbc->seedPosition + (*localRules)[rbc->mvt_it].currentPosition;
                        if(rbc->transferCount == 10 and getPrevOpDirection() != Direction::LEFT)
                            return;
                        rbc->sendHandleableMessage(
                            new CoordinateMessage(rbc->operation, targetModule,
                                                  rbc->module->position, rbc->mvt_it),
                            rbc->module->getInterface(targetModule), 100, 200);
                        
                        if(rbc->transferCount < 10 and getPrevOpDirection() == Direction::LEFT) {
                            setMvtItToNextModule(bc);
                        } else if(rbc->transferCount < 8){
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
        if( rbc->mvt_it == 2  and mmShape == FRONTBACK and Zeven) return;
        if( rbc->mvt_it == 2  and mmShape == BACKFRONT and not Zeven) return;
        if(rbc->mvt_it == localRules->size() -1 and mmShape == FRONTBACK and not Zeven) return;
        if(rbc->mvt_it == localRules->size() -1 and mmShape == BACKFRONT and  Zeven) return;

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
    if (direction == Direction::LEFT) {
    if(mmShape == FRONTBACK and Zeven and nextOpDir == Direction::DOWN ) {
        if(rbc->mvt_it >= 56) return true;
    } else {
        if(rbc->mvt_it >= 14) return true;
    }
        
    
        
    } else if(direction == Direction::RIGHT) {  
        if(/*Zeven and*/ rbc->mvt_it >= 46) return true;
        else if(rbc->mvt_it == 2) return true;
    } else if (direction == Direction::BACK) {
        if (mmShape == BACKFRONT) {
            if(rbc->getPreviousOpDir() == Direction::RIGHT) {
                if (not comingFromBack and rbc->mvt_it >= 39) return true;
            } else if(rbc->getPreviousOpDir() == Direction::LEFT){
                if (not comingFromBack and rbc->mvt_it >= 43) return true;
            } else {
                if (not comingFromBack and rbc->mvt_it >= 26) return true;
            }
            

           // if (not comingFromBack and rbc->mvt_it >= 26) return true;
            
            //if (not comingFromBack and rbc->mvt_it >= 39) return true;
            if (comingFromBack and rbc->mvt_it > 8) return true;
        } else if (mmShape == FRONTBACK) {
            if ((not comingFromBack and rbc->mvt_it == 4) or rbc->mvt_it == 6) return true;
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

void
Transfer_Operation::updateProbingPoints(BaseSimulator::BlockCode *bc, vector<Catoms3D::Catoms3DBlock *> &latchingPoints,
                                        const Cell3DPosition &targetPos) const {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);
    Operation::updateProbingPoints(bc, latchingPoints, targetPos);
    switch (direction) {
        case Direction::FRONT: {
            if (mmShape == FRONTBACK) {
                if (rbc.relativePos() == Cell3DPosition(-1, -4, 3)) {
                    latchingPoints.push_back(dynamic_cast<Catoms3D::Catoms3DBlock *>(
                                                     rbc.lattice->getBlock(
                                                             rbc.module->position + Cell3DPosition(2, 0, -2))));
                }
            } else {  // BackFront
                if (/*relativePos() == Cell3DPosition(1,0,2) and*/ rbc.lattice->cellHasBlock(
                        rbc.seedPosition + Cell3DPosition(1, -2, 2)) and
                                                                   rbc.relativePos().pt[1] > -2) {
                    latchingPoints.clear();
                    latchingPoints.push_back(dynamic_cast<Catoms3D::Catoms3DBlock *>(
                                                     rbc.lattice->getBlock(
                                                             rbc.seedPosition + Cell3DPosition(1, -2, 2))));
                }
                if (rbc.relativePos() == Cell3DPosition(0, -2, 2)) {
                    latchingPoints.clear();
                    latchingPoints.push_back(dynamic_cast<Catoms3D::Catoms3DBlock *>(
                                                     rbc.lattice->getBlock(
                                                             rbc.seedPosition + Cell3DPosition(1, -3, 1))));
                }
            }
        }
            break;

        case Direction::BACK: {
            if(mmShape == FRONTBACK) {
                if (prevOpDirection == Direction::BACK) {
                    if (isZeven() and rbc.mvt_it > 5 and
                        (*localRules)[rbc.mvt_it].currentPosition == Cell3DPosition(1, 0, 2)) {
                        latchingPoints.clear();
                        latchingPoints.push_back(
                                static_cast<Catoms3DBlock*>(rbc.lattice->getBlock(rbc.module->position.offsetY(2))));
                    } else if (not isZeven() and rbc.mvt_it > 5 and
                               (*localRules)[rbc.mvt_it].nextPosition == Cell3DPosition(1, 0, 2)) {
                        latchingPoints.clear();
                        latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                                                         rbc.lattice->getBlock(rbc.seedPosition + Cell3DPosition(1, 2, 2))));
                    } else if (rbc.mvt_it == 2) {
                        latchingPoints.clear();
                        latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                                                         rbc.lattice->getBlock(rbc.seedPosition + Cell3DPosition(1, 1, 2))));
                    }
                }
                if ((*localRules)[rbc.mvt_it].nextPosition == Cell3DPosition(0, 2, 2)) {
                    //Avoid blocking when FB transfering back then going LEFT
                    latchingPoints.push_back(static_cast<Catoms3DBlock *>(
                                                     rbc.lattice->getBlock(
                                                             rbc.seedPosition + Cell3DPosition(1, 3, 0))));
                    latchingPoints.push_back(static_cast<Catoms3DBlock *>(
                                                     rbc.lattice->getBlock(
                                                             rbc.seedPosition + Cell3DPosition(0, 3, 0))));
                    latchingPoints.push_back(static_cast<Catoms3DBlock *>(
                                                     rbc.lattice->getBlock(
                                                             rbc.seedPosition + Cell3DPosition(-1, 3, 1))));
                }
            }

            if (mmShape == BACKFRONT and
                getPrevOpDirection() != Direction::BACK and
                (rbc.relativePos() == Cell3DPosition(1, 1, 2) or
                 rbc.relativePos() == Cell3DPosition(0, 2, 2))) {
                latchingPoints.push_back(dynamic_cast<Catoms3D::Catoms3DBlock *>(
                                                 rbc.lattice->getBlock(rbc.seedPosition + Cell3DPosition(1, 3, 0))));
                latchingPoints.push_back(dynamic_cast<Catoms3D::Catoms3DBlock *>(
                                                 rbc.lattice->getBlock(rbc.seedPosition + Cell3DPosition(1, 3, 1))));
            }
        } break;

        case Direction::LEFT: {
            if ((*localRules)[rbc.mvt_it].nextPosition == Cell3DPosition(0, -1, 1)) {
                // BACKFRONG
                latchingPoints.clear();
                latchingPoints.push_back(static_cast<Catoms3DBlock*>(rbc.lattice->getBlock(rbc.seedPosition)));
                if (rbc.lattice->cellHasBlock(rbc.seedPosition.offsetY(-1))) {
                    latchingPoints.push_back(
                            static_cast<Catoms3DBlock*>(rbc.lattice->getBlock(rbc.seedPosition.offsetY(-1))));
                }
            } else if ((*localRules)[rbc.mvt_it].nextPosition == Cell3DPosition(0, 0, 1)) {
                // FRONTBACK
                latchingPoints.clear();
                latchingPoints.push_back(static_cast<Catoms3DBlock*>(rbc.lattice->getBlock(rbc.seedPosition)));
                if (rbc.lattice->cellHasBlock(rbc.seedPosition.offsetY(1))) {
                    latchingPoints.push_back(
                            static_cast<Catoms3DBlock*>(rbc.lattice->getBlock(rbc.seedPosition.offsetY(1))));
                }
            }
            if (mmShape == FRONTBACK and isZeven() and getNextOpDir() ==
                                                       Direction::DOWN) {
                if (rbc.relativePos() == Cell3DPosition(1, -1, 1)) {
                    latchingPoints.push_back(dynamic_cast<Catoms3D::Catoms3DBlock *>(
                                                     rbc.lattice->getBlock(
                                                             rbc.seedPosition + Cell3DPosition(-1, -1, 1))));
                }
            }
            if (rbc.relativePos() == Cell3DPosition(1, -1, 1) and getPrevOpDirection() == Direction::DOWN) {
                latchingPoints.clear();
                return;
            }
        }
            break;

        case Direction::RIGHT: {
            if (((rbc.relativePos() == Cell3DPosition(2, 0, 2) or
                    rbc.relativePos() == Cell3DPosition(3, 0, 2)) and
                 mmShape == BACKFRONT and rbc.mvt_it > 3 and isZeven() and
                 getNextOpDir() != Direction::DOWN) or
                ((rbc.relativePos() == Cell3DPosition(2, 1, 2) or
                        rbc.relativePos() == Cell3DPosition(3, 0, 1)) and
                 getMMShape() == BACKFRONT and rbc.mvt_it > 4 and not isZeven() and
                 getNextOpDir() != Direction::DOWN)) {
                latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                                                 rbc.lattice->getBlock(rbc.seedPosition + Cell3DPosition(3, -1, 1))));
                latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                                                 rbc.lattice->getBlock(rbc.seedPosition + Cell3DPosition(4, 0, 0))));
                latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                                                 rbc.lattice->getBlock(rbc.seedPosition + Cell3DPosition(5, 0, 0))));
                latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                                                 rbc.lattice->getBlock(rbc.seedPosition + Cell3DPosition(5, 0, 1))));
                if (rbc.lattice->cellHasBlock(rbc.seedPosition + Cell3DPosition(5, 1, 2)))
                    latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                                                     rbc.lattice->getBlock(rbc.seedPosition + Cell3DPosition(5, 1, 2))));
                if (rbc.lattice->cellHasBlock(rbc.seedPosition + Cell3DPosition(5, 2, 2)))
                    latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                                                     rbc.lattice->getBlock(rbc.seedPosition + Cell3DPosition(5, 2, 2))));
            }

            if (rbc.relativePos() == Cell3DPosition(1, 0, 1) and rbc.mvt_it >= 7 and
                mmShape == BACKFRONT and isZeven()) {
                // BF
                latchingPoints.clear();
                latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                                                 rbc.lattice->getBlock(rbc.seedPosition + Cell3DPosition(2, -1, 2))));
                if (rbc.lattice->cellHasBlock(rbc.seedPosition + Cell3DPosition(3, 0, 2)))
                    latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                                                     rbc.lattice->getBlock(rbc.seedPosition + Cell3DPosition(3, 0, 2))));
            }

            if (rbc.relativePos() == Cell3DPosition(1, -1, 1) and rbc.mvt_it >= 7 and
                mmShape == FRONTBACK) {
                // FB
                latchingPoints.clear();
                latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                                                 rbc.lattice->getBlock(rbc.seedPosition + Cell3DPosition(2, 1, 2))));
                if (rbc.lattice->cellHasBlock(rbc.seedPosition + Cell3DPosition(3, 0, 2)))
                    latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                                                     rbc.lattice->getBlock(rbc.seedPosition + Cell3DPosition(3, 0, 2))));
            }
        }
            break;

        case Direction::DOWN: {
            if (mmShape == FRONTBACK) {
                if (rbc.relativePos() == Cell3DPosition(1, 0, 0)) {
                    // Zeven
                    latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                                                     rbc.lattice->getBlock(rbc.module->position + Cell3DPosition(1, 2, -2))));
                    latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                                                     rbc.lattice->getBlock(rbc.module->position + Cell3DPosition(0, 1, -3))));
                } else if (rbc.relativePos() == Cell3DPosition(1, -1, 0)) {
                    // Zodd
                    latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                                                     rbc.lattice->getBlock(rbc.module->position + Cell3DPosition(1, 1, -2))));
                    latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                                                     rbc.lattice->getBlock(rbc.module->position + Cell3DPosition(0, 0, -3))));
                }
            } else {  // operation.shape = BACKFRONT
                if (rbc.relativePos() == Cell3DPosition(1, 0, 0)) {
                    // Zeven
                    latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                                                     rbc.lattice->getBlock(rbc.module->position + Cell3DPosition(1, -2, -2))));
                    latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                                                     rbc.lattice->getBlock(rbc.module->position + Cell3DPosition(0, -2, -3))));
                } else if (rbc.relativePos() == Cell3DPosition(1, 1, 0)) {
                    // Zodd
                    latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                                                     rbc.lattice->getBlock(rbc.module->position + Cell3DPosition(1, -1, -2))));
                    latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                                                     rbc.lattice->getBlock(rbc.module->position + Cell3DPosition(0, -1, -3))));
                }
            }
        }
            break;
        default:
            break;
    }
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

    case Direction::DOWN: {
        if(mmShape == BACKFRONT) {
            (Zeven) ?  localRules.reset(&LocalRules_BF_Build_Down_Zeven):
                localRules.reset(&LocalRules_BF_Build_Down_Zodd);
        } else { //FRONTBACK
            VS_ASSERT_MSG(false, "Not implemented");
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

    case Direction::LEFT: {
        if(mmShape == BACKFRONT) {
            if(getPrevOpDirection() == Direction::BACK) {
               localRules.reset(&LocalRules_BF_Build_Left_Zodd_ComingFromBack);
            } else {
                localRules.reset(&LocalRules_BF_Build_Left_Zodd);
            }

        } else {
            localRules.reset(&LocalRules_FB_Build_Left_Zodd);
        }
    } break;

    case Direction::RIGHT: {
        if (mmShape == BACKFRONT) {

                // Zeven ? VS_ASSERT_MSG(false, "Not implemented") :
                localRules.reset(&LocalRules_BF_Build_Right_Zodd);

        } else { //FRONTBACK
            //Zeven ? VS_ASSERT_MSG(false, "Not implemented") :
            localRules.reset(&LocalRules_FB_Build_Right_Zodd);
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
        rbc->mvt_it < localRules->size() and (not isComingFromBack() or direction == Direction::RIGHT)) {
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

        //if(rbc->transferCount < 10)
            if(rbc->transferCount == 10 and (direction == Direction::LEFT or direction == Direction::RIGHT))
                return;
            setMvtItToNextModule(bc);

        rbc->console << "mvt_it: " << rbc->mvt_it << "\n";
    } else if (rbc->isCoordinator and pos == (rbc->module->position + Cell3DPosition(0, 1, 1)) and
               (direction == Direction::BACK or direction == Direction::UP or direction == Direction::LEFT) and mmShape == BACKFRONT and isComingFromBack() ) {
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

    case Direction::DOWN: {
        Init::getNeighborMMSeedPos(rbc->seedPosition, rbc->MMPosition, Direction::DOWN, rbc->seedPosition);
        rbc->MMPosition = rbc->MMPosition.offsetZ(-1);
        rbc->initialPosition = rbc->module->position - rbc->seedPosition;
    } break;

     case Direction::BACK: {
        (rbc->shapeState == FRONTBACK) ? rbc->shapeState = BACKFRONT : rbc->shapeState = FRONTBACK;
        Init::getNeighborMMSeedPos(rbc->seedPosition, rbc->MMPosition, Direction::BACK, rbc->seedPosition);
        rbc->MMPosition = rbc->MMPosition.offsetY(1);
        rbc->initialPosition = rbc->module->position - rbc->seedPosition;
     } break;   

     case Direction::LEFT: {
         (rbc->shapeState == FRONTBACK) ? rbc->shapeState = BACKFRONT : rbc->shapeState = FRONTBACK;
         Init::getNeighborMMSeedPos(rbc->seedPosition, rbc->MMPosition, Direction::LEFT,
                                    rbc->seedPosition);
         rbc->MMPosition = rbc->MMPosition.offsetX(-1);
         rbc->initialPosition = rbc->module->position - rbc->seedPosition;
     } break;

    case Direction::RIGHT: {
        (rbc->shapeState == FRONTBACK) ? rbc->shapeState = BACKFRONT : rbc->shapeState = FRONTBACK;
        Init::getNeighborMMSeedPos(rbc->seedPosition, rbc->MMPosition, Direction::RIGHT,
                                   rbc->seedPosition);
        rbc->MMPosition = rbc->MMPosition.offsetX(1);
        rbc->initialPosition = rbc->module->position - rbc->seedPosition;
    } break;
    
    default:
        break;
    }
}

bool Build_Operation::mustSendCoordinateBack(BaseSimulator::BlockCode* bc) {
    RePoStBlockCode* rbc = static_cast<RePoStBlockCode*>(bc);

    switch (direction) {
        case Direction::BACK: {
            if(mmShape == FRONTBACK) {
                if(rbc->mvt_it == 49) return true;
            } else { //BACKFRONT
                if (getPrevOpDirection() == Direction::BACK and rbc->mvt_it >= 37 and
                    rbc->mvt_it < localRules->size() - 1)
                    return true;
                if (rbc->mvt_it >= 42 and rbc->mvt_it < localRules->size() - 1) return true;
            }
        } break;

        case Direction::LEFT: {
            if(mmShape == FRONTBACK) {
                if(rbc->mvt_it >= 83 and rbc->mvt_it < localRules->size() - 1) return true;
            } else { //BACKFRONT
                if (rbc->mvt_it == 85) return true;
                if (getPrevOpDirection() != Direction::BACK and rbc->mvt_it >= 74 and
                    rbc->mvt_it < localRules->size() - 1)
                    return true;

            }
        } break;

        case Direction::RIGHT: {
            if (rbc->mvt_it >= 57 and rbc->mvt_it != localRules->size() - 1) return true; //FB & BF
        } break;

        default: return false;
    }
    return false;
}

void Build_Operation::updateProbingPoints(BaseSimulator::BlockCode *bc, vector<Catoms3D::Catoms3DBlock *> &latchingPoints,
                                          const Cell3DPosition &targetPos) const {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);
    Operation::updateProbingPoints(bc, latchingPoints, targetPos);
    switch (direction) {
        case Direction::UP: {
            if (mmShape == FRONTBACK) {
                if (rbc.relativePos() == Cell3DPosition(1, -1, 3) or rbc.relativePos() == Cell3DPosition(2,0,2)) {
                    latchingPoints.clear();
                }
            }
        } break;

        case Direction::BACK: {
            if (mmShape == FRONTBACK) {
                if(rbc.relativePos() == Cell3DPosition(1,-1,1)) {
                    latchingPoints.clear();
                }
            }
        } break;

        default:
            break;
    }
}


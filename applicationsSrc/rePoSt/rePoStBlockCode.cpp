#include "rePoStBlockCode.hpp"
#include "init.hpp"
#include "routing.hpp"
#include "./messages/srcDestMessages.hpp"
#include "./messages/maxFlowMessages.hpp"
#include "./messages/transportMessages.hpp"
#include <fstream>
#include "robots/catoms3D/catoms3DMotionEngine.h"

using namespace Catoms3D;

Catoms3DBlock* RePoStBlockCode::GC = nullptr; 
int RePoStBlockCode::NbOfStreamlines = 0;

RePoStBlockCode::RePoStBlockCode(Catoms3DBlock *host) : Catoms3DBlockCode(host) {
    // @warning Do not remove block below, as a blockcode with a NULL host might be created
    //  for command line parsing
    if (not host) return;

    // Set the module pointer
    module = static_cast<Catoms3DBlock*>(hostBlock);
    
}

void RePoStBlockCode::startup() {
    console << "start\n";
    //operation = new Operation();
    if(not Init::initialMapBuildDone) {
        initialColor = GREEN;
        initialized = false;
        MMPosition = Cell3DPosition(0,0,0);
        shapeState = FRONTBACK;
        GC = module;
        cerr << GC->position << endl;
        Init::buildMM(module, FRONTBACK, Color(GREEN));
        if(initialMap[0][3] == 1) {
            Init::fillMM(GC);
        }
        Init::buildInitialMap(GC->position, initialMap);
        setGreenLight(true);
        rotating = false;
        seedPosition = module->position;
        Cell3DPosition targetModule;
     
        // MetaModuleBlockCode *block49 = static_cast<MetaModuleBlockCode*>(
        //     BaseSimulator::getWorld()->getBlockById(49)->blockCode
        // );
        // operation = new Operation(Direction::LEFT, BACKFRONT);
        // block49->operation = new Dismantle_Operation(Direction::LEFT, FRONTBACK);
        // block49->isCoordinator = true;

        // targetModule = block49->seedPosition + (*block49->operation->localRules)[0].currentPosition;
        // block49->console << "targetModule: " << block49->nearestPositionTo(targetModule) << "\n"; 
        // block49->sendMessage("Coordinate Msg1", new MessageOf<Coordinate>(
        //     COORDINATE_MSG_ID, Coordinate(block49->operation, targetModule, block49->module->position, block49->mvt_it)),
        //     block49->module->getInterface(block49->nearestPositionTo(targetModule)), 100, 200
        // );

        

        // MetaModuleBlockCode *block13 = static_cast<MetaModuleBlockCode*>(
        //     BaseSimulator::getWorld()->getBlockById(13)->blockCode
        // );

        // block13->operation = new Transfer_Operation(Direction::LEFT, BACKFRONT);
        // block13->isCoordinator = true;

        // MetaModuleBlockCode *block3 = static_cast<MetaModuleBlockCode*>(
        //     BaseSimulator::getWorld()->getBlockById(3)->blockCode
        // );
        // block3->operation = new Transfer_Operation(Direction::LEFT, FRONTBACK);
        // block3->isCoordinator = true;

        

        // MetaModuleBlockCode *block23 = static_cast<MetaModuleBlockCode*>(
        //     BaseSimulator::getWorld()->getBlockById(23)->blockCode
        // );
        // block23->operation = new Transfer_Operation(Direction::LEFT, BACKFRONT);
        // block23->isCoordinator = true;

        // // MetaModuleBlockCode *block29 = static_cast<MetaModuleBlockCode*>(
        // //     BaseSimulator::getWorld()->getBlockById(29)->blockCode
        // // );
        // // operation = new Operation(Direction::LEFT, BACKFRONT);
        // // block29->operation = new Dismantle_Operation(Direction::LEFT, BACKFRONT, MMPosition.pt[2], true);
        // // block29->isCoordinator = true;

        // // targetModule = block29->seedPosition + (*block29->operation->localRules)[0].currentPosition;
        // // block29->console << "targetModule: " << block29->nearestPositionTo(targetModule) << "\n"; 
        // // block29->sendMessage("Coordinate Msg1", new MessageOf<Coordinate>(
        // //     COORDINATE_MSG_ID, Coordinate(block29->operation, targetModule, block29->module->position, block29->mvt_it)),
        // //     block29->module->getInterface(block29->nearestPositionTo(targetModule)), 100, 200
        // // );

        // MetaModuleBlockCode *block33 = static_cast<MetaModuleBlockCode*>(
        //     BaseSimulator::getWorld()->getBlockById(33)->blockCode
        // );
        // block33->operation = new Transfer_Operation(Direction::BACK, FRONTBACK);
        // block33->isCoordinator = true;

        // //  MetaModuleBlockCode *block43 = static_cast<MetaModuleBlockCode*>(
        // //     BaseSimulator::getWorld()->getBlockById(43)->blockCode
        // // );
        // // block43->operation = new Build_Operation(Direction::UP, FRONTBACK, block43->MMPosition.pt[2]);
        // // block43->isCoordinator = true;

        // // MetaModuleBlockCode *block323 = static_cast<MetaModuleBlockCode*>(
        // //     BaseSimulator::getWorld()->getBlockById(323)->blockCode
        // // );
        // // block323->operation = new Build_Operation(Direction::UP, FRONTBACK, block323->MMPosition.pt[2]);
        // // block323->isCoordinator = true;

        // MetaModuleBlockCode *block83 = static_cast<MetaModuleBlockCode*>(
        //     BaseSimulator::getWorld()->getBlockById(83)->blockCode
        // );
        // block83->operation = new Transfer_Operation(Direction::BACK, BACKFRONT, true, 0);
        // block83->isCoordinator = true;

        // MetaModuleBlockCode *block113 = static_cast<MetaModuleBlockCode*>(
        //     BaseSimulator::getWorld()->getBlockById(113)->blockCode
        // );
        // block113->operation = new Transfer_Operation(Direction::BACK, FRONTBACK, 0);
        // block113->isCoordinator = true;

        // MetaModuleBlockCode *block133 = static_cast<MetaModuleBlockCode*>(
        //     BaseSimulator::getWorld()->getBlockById(133)->blockCode
        // );
        // block133->operation = new Transfer_Operation(Direction::UP, BACKFRONT, true, block133->MMPosition.pt[2]);
        // block133->isCoordinator = true;

        //  MetaModuleBlockCode *block143 = static_cast<MetaModuleBlockCode*>(
        //     BaseSimulator::getWorld()->getBlockById(143)->blockCode
        // );
        // block143->operation = new Build_Operation(Direction::UP, BACKFRONT,  block143->MMPosition.pt[2]);
        // block143->isCoordinator = true;

        // MetaModuleBlockCode *block153 = static_cast<MetaModuleBlockCode*>(
        //     BaseSimulator::getWorld()->getBlockById(153)->blockCode
        // );
        // block153->operation = new Build_Operation(Direction::UP, BACKFRONT, block153->MMPosition.pt[2]);
        // block153->isCoordinator = true;

    //     /***********************************Y=1*****************************************/
    //     MetaModuleBlockCode *block99 = static_cast<MetaModuleBlockCode*>(
    //         BaseSimulator::getWorld()->getBlockById(99)->blockCode
    //     );
    //     block99->operation = new Dismantle_Operation(Direction::LEFT, BACKFRONT);
    //     block99->isCoordinator = true;
        
    //     targetModule = block99->seedPosition + (*block99->operation->localRules)[0].currentPosition;
    //     block99->console << "targetModule: " << block99->nearestPositionTo(targetModule) << "\n"; 
    //     block99->sendMessage("Coordinate Msg1", new MessageOf<Coordinate>(
    //         COORDINATE_MSG_ID, Coordinate(block99->operation, targetModule, block99->module->position, block99->mvt_it)),
    //         block99->module->getInterface(block99->nearestPositionTo(targetModule)), 100, 200
    //     );
       
    //    MetaModuleBlockCode *block63 = static_cast<MetaModuleBlockCode*>(
    //         BaseSimulator::getWorld()->getBlockById(63)->blockCode
    //     );

    //     block63->operation = new Transfer_Operation(Direction::LEFT, FRONTBACK);
    //     block63->isCoordinator = true;

    //     MetaModuleBlockCode *block53 = static_cast<MetaModuleBlockCode *>(
    //         BaseSimulator::getWorld()->getBlockById(53)->blockCode);

    //     block53->operation = new Fill_Operation(Direction::LEFT, BACKFRONT);
    //     block53->isCoordinator = true;

    //     MetaModuleBlockCode *block73 = static_cast<MetaModuleBlockCode *>(
    //         BaseSimulator::getWorld()->getBlockById(73)->blockCode);

        //     block73->operation = new Transfer_Operation(Direction::BACK, FRONTBACK);
        //     block73->isCoordinator = true;

        //     MetaModuleBlockCode *block103 = static_cast<MetaModuleBlockCode*>(
        //         BaseSimulator::getWorld()->getBlockById(103)->blockCode
        //     );

        //     block103->operation = new Transfer_Operation(Direction::BACK, BACKFRONT, true);
        //     block103->isCoordinator = true;

        //     MetaModuleBlockCode *block123 = static_cast<MetaModuleBlockCode*>(
        //         BaseSimulator::getWorld()->getBlockById(123)->blockCode
        //     );

        //     block123->operation = new Transfer_Operation(Direction::UP, FRONTBACK, false,
        //     block123->MMPosition.pt[2]); block123->isCoordinator = true;

        //        MetaModuleBlockCode *block363 = static_cast<MetaModuleBlockCode*>(
        //         BaseSimulator::getWorld()->getBlockById(363)->blockCode
        //     );

        //     block363->operation = new Transfer_Operation(Direction::UP, FRONTBACK, false,
        //     block363->MMPosition.pt[2]); block363->isCoordinator = true;

        //     MetaModuleBlockCode *block373 = static_cast<MetaModuleBlockCode*>(
        //         BaseSimulator::getWorld()->getBlockById(373)->blockCode
        //     );

        //     block373->operation = new Build_Operation(Direction::UP, FRONTBACK,
        //     block373->MMPosition.pt[2]); block373->isCoordinator = true;

        Init::initialMapBuildDone = true;
        // ofstream file;
        // file.open("nbMovementsPerTimeStep.txt", ios::out);
        // file << timeStep << ',' << 0 << ',' << 0 << '\n';
        // getScheduler()->schedule(
        //                     new InterruptionEvent(getScheduler()->now() + getRoundDuration(),
        //                                           module, IT_MODE_NBMOVEMENTS));
    }

    initialColor = module->color;
    initialized = true;
    if(isPotentialSource()) {
        distance = 1;
    }
    VS_ASSERT(Init::initialMapBuildDone);
      if(targetMap.empty()) return;

    if(module->blockId == GC->blockId) {
        reconfigurationStep = SRCDEST;
        nbOfIterations++;
        nbWaitedAnswers = 0;
        distance = 0;
                 cerr << "Building coordination tree\n";
        getScheduler()->toggle_pause();
        for (auto p : getAdjacentMMSeeds()) {
            Cell3DPosition toMMPosition =
                static_cast<RePoStBlockCode*>(
                    BaseSimulator::getWorld()->getBlockByPosition(p)->blockCode)
                    ->MMPosition;
            console<< toMMPosition << "\n";
            sendHandleableMessage(new GoMessage(MMPosition, toMMPosition, distance),
                                  interfaceTo(MMPosition, toMMPosition), 100, 200);
            nbWaitedAnswers++;
        }
    }

    
  
    // if(isSource() and seedPosition == module->position) {
    //     cerr << MMPosition << ": is source\n";
    //     //VS_ASSERT(mainPathState == NONE);
    //     mainPathState = BFS;
    //     mainPathIn = MMPosition;
    //     mainPathsOld.push_back(MMPosition);
    //     for(auto p: getAdjacentMMSeeds()) {
    //         //cerr << MMPosition << ": " << p << endl;
    //         deficit++;
    //         sendMessage(
    //             "BFS msg",
    //             new MessageOf<BFSdata>(BFS_MSG_ID, BFSdata(MMPosition, MMPosition,
    //                                                        p)),
    //             interfaceTo(p), 100, 200);
    //     }
    // }
    // if(isDestination() and seedPosition == module->position) {
    //     cerr << MMPosition << ": is destination\n";
    // }
    
}

void RePoStBlockCode::reinitialize() {
    cerr << "REINITIALIZE!\n";
    for (auto id_block : BaseSimulator::getWorld()->buildingBlocksMap) {
        RePoStBlockCode* block = static_cast<RePoStBlockCode*>(id_block.second->blockCode);
        block->parentPosition = Cell3DPosition(-1, -1, -1);
        block->childrenPositions.clear();
        block->distance = 0;
        block->isSource = false;
        block->isDestination = false;
        block->mainPathState = block->aug1PathState = block->aug2PathState = NONE;
        block->mainPathIn = block->aug1PathIn = block->aug2PathIn = Cell3DPosition(-1, -1, -1);
        block->mainPathOut.clear();
        block->aug1PathOut.clear();
        block->aug2PathOut.clear();
        block->mainPathsOld.clear();
        block->aug1PathsOld.clear();
        block->aug2PathsOld.clear();
        block->cont_passive = true;
        block->b = true;
        block->res = true;
        block->deficit = 0;
        block->isCoordinator = false;
        block->transferCount = 0;
        if(block->movingState != MOVING ) block->mvt_it = 0;
        NbOfStreamlines = 0;
        NbOfDestinationsReached = 0;
        destinations.clear();
    }
}



void RePoStBlockCode::start_wave() {
    //cont_passive = false;
    nbWaitedAnswers = 0;
    for (auto child : childrenPositions) {
        Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(child);
        sendHandleableMessage(new GoTermMessage(MMPosition, child), interfaceTo(MMPosition, child),
                              100, 200);
        nbWaitedAnswers++;
    }
}

void RePoStBlockCode::return_wave(bool b) {

}

/* -------------------------------------------------------------------------- */

void RePoStBlockCode::setOperation(Cell3DPosition inPosition, Cell3DPosition outPosition) {
    Direction direction;
    Cell3DPosition directionVector = outPosition - MMPosition;
    if (directionVector.pt[0] == -1) direction = Direction::LEFT;
    if (directionVector.pt[0] == 1) direction = Direction::RIGHT;
    if (directionVector.pt[1] == -1) direction = Direction::FRONT;
    if (directionVector.pt[1] == 1) direction = Direction::BACK;
    if (directionVector.pt[2] == -1) direction = Direction::DOWN;
    if (directionVector.pt[2] == 1) direction = Direction::UP;
    if (isSource) {
        (shapeState == FRONTBACK) ? coordinatorPosition = seedPosition + Cell3DPosition(-1, -1, 2)
                                  : coordinatorPosition = seedPosition + Cell3DPosition(-1, 1, 2);
        if (direction == Direction::BACK and shapeState == FRONTBACK) {
            coordinatorPosition = seedPosition + Cell3DPosition(2, 1, 2);
        } else if (direction == Direction::UP) {
            if (MMPosition.pt[2] % 2 != 0) {
                coordinatorPosition = seedPosition + Cell3DPosition(1, 0, 4);
            }
        }
    } else {
        (shapeState == FRONTBACK) ? coordinatorPosition = seedPosition + Cell3DPosition(1, 0, 1)
                                  : coordinatorPosition = seedPosition + Cell3DPosition(1, -1, 1);
    }
    RePoStBlockCode* coordinator = static_cast<RePoStBlockCode*>(
        BaseSimulator::getWorld()->getBlockByPosition(coordinatorPosition)->blockCode);
    coordinator->isCoordinator = true;
    if (isSource) {
        coordinator->operation = new Dismantle_Operation(direction, shapeState, getPreviousOpDir(),
                                                         MMPosition.pt[2], false);
    } else if (isDestination) {
        bool comingFromBack =
            (inPosition.pt[0] == MMPosition.pt[0] and inPosition.pt[1] == MMPosition.pt[1] - 1 and
             shapeState == BACKFRONT);
        coordinator->operation = new Build_Operation(direction, shapeState, getPreviousOpDir(),
                                                     comingFromBack, MMPosition.pt[2]);
    } else {
        bool comingFromBack =
            (inPosition.pt[0] == MMPosition.pt[0] and inPosition.pt[1] == MMPosition.pt[1] - 1 and
             shapeState == BACKFRONT);

        coordinator->operation = new Transfer_Operation(direction, shapeState, getPreviousOpDir(),
                                                        comingFromBack, MMPosition.pt[2]);
    }
}

 /* -------------------------------------------------------------------------- */
 /*                             MOTION COORDINATION                            */
 /* -------------------------------------------------------------------------- */
void RePoStBlockCode::probeGreenLight() {
    VS_ASSERT(operation->localRules != NULL);
    if (operation->getDirection() == Direction::UP /*and operation->isTransfer()*/ and
        (*operation->localRules)[mvt_it].nextPosition == Cell3DPosition(1, 0, 2)) {
        (*operation->localRules)[mvt_it].nextPosition = Cell3DPosition(2, 0, 2);
    }

    if ((*operation->localRules)[mvt_it].nextPosition == Cell3DPosition(2, 0, 2) and
        lattice->cellHasBlock(module->position.offsetX(1)) and
        operation->getDirection() == Direction::UP /*and operation->isTransfer()*/) {
        (*operation->localRules)[mvt_it].nextPosition = Cell3DPosition(1, 0, 2);
    }

    if (operation->getDirection() == Direction::DOWN) {
        if ((*operation->localRules)[mvt_it].nextPosition == Cell3DPosition(1, 1, -2)) {
            (*operation->localRules)[mvt_it].nextPosition = Cell3DPosition(2, 1, -2);
        }

        if ((*operation->localRules)[mvt_it].nextPosition == Cell3DPosition(2, 1, -2) and
            lattice->cellHasBlock((*operation->localRules)[mvt_it].nextPosition + seedPosition)) {
            (*operation->localRules)[mvt_it].nextPosition = Cell3DPosition(1, 1, -2);
        }

        if ((*operation->localRules)[mvt_it].nextPosition == Cell3DPosition(1, -1, -2)) {
            (*operation->localRules)[mvt_it].nextPosition = Cell3DPosition(2, -1, -2);
        }

        if ((*operation->localRules)[mvt_it].nextPosition == Cell3DPosition(2, -1, -2) and
            lattice->cellHasBlock((*operation->localRules)[mvt_it].nextPosition + seedPosition)) {
            (*operation->localRules)[mvt_it].nextPosition = Cell3DPosition(1, -1, -2);
        }
    }

    if(operation->getDirection() == Direction::RIGHT) {
        // When pivot cannot be found when coming from Right then transferring down
        if(operation->isZeven() and operation->getMMShape() == FRONTBACK) {
            if(mvt_it >= 13 and (*operation->localRules)[mvt_it].nextPosition == Cell3DPosition(5,0,1)
                and not lattice->cellHasBlock(seedPosition + Cell3DPosition(5,0,0))) {
                (*operation->localRules)[mvt_it].nextPosition = Cell3DPosition(5,0,0);
            } 
        } else if (not operation->isZeven() and operation->getMMShape() == BACKFRONT) {
             if(mvt_it >= 13 and (*operation->localRules)[mvt_it].nextPosition == Cell3DPosition(5,-1,1)
                and not lattice->cellHasBlock(seedPosition + Cell3DPosition(5,0,0))) {
                (*operation->localRules)[mvt_it].nextPosition = Cell3DPosition(5,0,0);
            } 
        }
        
    }

    if(operation->getDirection() == Direction::BACK) {
        // When pivot cannot be found when coming from back then transferring down
        if(operation->getMMShape() == BACKFRONT) {
            if( (*operation->localRules)[mvt_it].nextPosition == Cell3DPosition(1,3,0)) {
                (*operation->localRules)[mvt_it].nextPosition = Cell3DPosition(1,2,1);
            }
            if(mvt_it >= 7 and (*operation->localRules)[mvt_it].nextPosition == Cell3DPosition(1,2,1)
                and not lattice->cellHasBlock(seedPosition + Cell3DPosition(1,3,0))) {
                     (*operation->localRules)[mvt_it].nextPosition = Cell3DPosition(1,3,0);
                }
        }
    }

    if (relativePos() == Cell3DPosition(4, 0, 2) and
        module->getInterface(module->position.offsetY(1))->isConnected()) {
        // special logic to avoid blocking when coming from right then going back.
        // coordinator is not reached so modules be coordinated without being attached to the
        // coordinator
        if ((operation->isZeven() and (mvt_it == 49 or mvt_it == 53)) 
            or (not operation->isZeven() and mvt_it == 45)) {
            sendHandleableMessage(new CoordinateBackMessage(movingSteps + 2, coordinatorPosition),
                                  interfaceTo(coordinatorPosition), 0, 200);
        }

        updateState();
        coordinatorPosition = seedPosition + Cell3DPosition(1, -1, 1);
        RePoStBlockCode& coordinator =
            *static_cast<RePoStBlockCode*>(lattice->getBlock(coordinatorPosition)->blockCode);
        operation = coordinator.operation;
        coordinator.console << "right_back it1: " << coordinator.mvt_it << "\n";

        while ((*coordinator.operation->localRules)[coordinator.mvt_it].currentPosition !=
               Cell3DPosition(1, 1, 2)) {
            coordinator.mvt_it++;
        }
        mvt_it = coordinator.mvt_it - 1;
        coordinator.console << "right_back it2: " << coordinator.mvt_it << "\n";

        coordinator.operation->setMvtItToNextModule(&coordinator);
        coordinator.console << "right_back it3: " << coordinator.mvt_it << "\n";
        if (not operation->isZeven()) {
            if (coordinator.mvt_it > 42 and coordinator.operation->isBuild())
                coordinator.mvt_it -= 3;
            if (coordinator.mvt_it > 39 and coordinator.operation->isTransfer())
                coordinator.mvt_it -= 3;
        } else {
            if (coordinator.mvt_it > 42 and coordinator.operation->isBuild())
                coordinator.mvt_it -= 4;
            if (coordinator.mvt_it > 39 and coordinator.operation->isTransfer())
                coordinator.mvt_it -= 4;
        }

        coordinator.console << "right_back it4: " << coordinator.mvt_it << "\n";
        if (module->canMoveTo(seedPosition + Cell3DPosition(1, 1, 2))) {
            // module->moveTo(seedPosition + Cell3DPosition(1, 1, 2));
            getScheduler()->schedule(new Catoms3DRotationStartEvent(
                getScheduler()->now() + 150, module, seedPosition + Cell3DPosition(1, 1, 2),
                RotationLinkType::Any, false));
            return;
        } else {
            VS_ASSERT(false);
        }
        return;
    }

    if(relativePos() ==  Cell3DPosition(4,0,2) and
        module->getInterface(module->position.offsetY(-1))->isConnected()) {
        
        if ((operation->isZeven() and mvt_it == 45) or (not operation->isZeven() and mvt_it >= 46) ) {
            sendHandleableMessage(new CoordinateBackMessage(movingSteps + 2, coordinatorPosition),
                                  interfaceTo(coordinatorPosition), 0, 200);
        }
        updateState();
        coordinatorPosition = seedPosition + Cell3DPosition(1, 0, 1);
        RePoStBlockCode& coordinator =
            *static_cast<RePoStBlockCode*>(lattice->getBlock(coordinatorPosition)->blockCode);
        operation = coordinator.operation;
        coordinator.console << "right_back it1: " << coordinator.mvt_it << "\n";

        while ((*coordinator.operation->localRules)[coordinator.mvt_it].currentPosition !=
               Cell3DPosition(1, -1, 2)) {
            coordinator.mvt_it++;
        }
        mvt_it = coordinator.mvt_it - 1;
        coordinator.console << "right_back it2: " << coordinator.mvt_it << "\n";

        coordinator.operation->setMvtItToNextModule(&coordinator);
        coordinator.console << "right_back it3: " << coordinator.mvt_it << "\n";
        if (not operation->isZeven()) {
            if (coordinator.mvt_it > 42 and coordinator.operation->isBuild())
                coordinator.mvt_it -= 3;
            if (coordinator.mvt_it > 39 and coordinator.operation->isTransfer())
                coordinator.mvt_it -= 3;
        } else {
            if (coordinator.mvt_it > 42 and coordinator.operation->isBuild())
                coordinator.mvt_it -= 4;
            if (coordinator.mvt_it > 39 and coordinator.operation->isTransfer())
                coordinator.mvt_it -= 4;
        }
        coordinator.console << "right_back it4: " << coordinator.mvt_it << "\n";
        if (module->canMoveTo(seedPosition + Cell3DPosition(1, -1, 2))) {
            // module->moveTo(seedPosition + Cell3DPosition(1, 1, 2));
            getScheduler()->schedule(new Catoms3DRotationStartEvent(
                getScheduler()->now() + 150, module, seedPosition + Cell3DPosition(1, -1, 2),
                RotationLinkType::Any, false));
            return;
        } else {
            VS_ASSERT(false);
        }
        return;
    }
    rotating = true;
    module->setColor(BLUE);
    LocalMovement lmvt = (*operation->localRules)[mvt_it];
    console << "mvt_it1: " << mvt_it << "\n";

    Cell3DPosition targetPosition = lmvt.nextPosition + seedPosition;
    console << "probeGreenLight: " << targetPosition << "\n";
    if(not greenLightIsOn) setGreenLight(true);


    Catoms3DBlock* pivot = customFindMotionPivot(module, targetPosition);
    if (not pivot or pivot->getState() == BuildingBlock::State::ACTUATING) {
        notFindingPivot = true;
        getScheduler()->schedule(new InterruptionEvent(getScheduler()->now() + getRoundDuration(),
                                                       module, IT_MODE_FINDING_PIVOT));
        stringstream info;
        info << " reattempt finding pivot for " << targetPosition;
        scheduler->trace(info.str(), module->blockId, PINK);
        return;
    }

    notFindingPivot = false;
    // VS_ASSERT(pivot);
    console << "pivot: " << pivot->position << "\n";
    vector<Catoms3DBlock*> latchingPoints = findNextLatchingPoints(targetPosition, pivot->position);
    console << "latchingPoints: ";
    for(auto lp: latchingPoints) {
        console << '(' << lp->position << ") ";
    }
    console << "\n";
    
    if(latchingPoints.empty()) {
        module->moveTo(targetPosition);
    } else {
        nbWaitedAnswers = 0;
        for(auto lp: latchingPoints) {
            nbWaitedAnswers++;
            sendHandleableMessage(new PLSMessage(module->position, lp->position),
                module->getInterface(nearestPositionTo(lp->position)), 100, 200);
        }
    }

    
    // if (module->getInterface(pivot->position)->isConnected())
    //     sendHandleableMessage(new PLSMessage(module->position, targetPosition),
    //          module->getInterface(pivot->position), 100, 200);
}

bool RePoStBlockCode::isAdjacentToPosition(const Cell3DPosition& pos) const {
    return lattice->cellsAreAdjacent(module->position, pos);
}

Catoms3DBlock* RePoStBlockCode::customFindMotionPivot(const Catoms3DBlock* m,
                                                            const Cell3DPosition& tPos,
                                                            RotationLinkType faceReq) {
    const auto &allLinkPairs =
        Catoms3DMotionEngine::findPivotLinkPairsForTargetCell(m, tPos, faceReq);

    for (const auto& pair : allLinkPairs) {
        // Additional rule compared to Catoms3DMotionEngine::customFindMotionPivot:
        //  Make sure that pivot is not mobile
        if (static_cast<RePoStBlockCode *>(pair.first->blockCode)->movingState == MOVING or
            pair.first->getState() == BuildingBlock::State::MOVING)
            continue;

        if (pair.second->getMRLT() == faceReq or faceReq == RotationLinkType::Any)
            return pair.first;
    }
    return NULL;
}

Catoms3DBlock* RePoStBlockCode::findTargetLightAmongNeighbors(
    const Cell3DPosition& targetPos, const Cell3DPosition& srcPos,
    P2PNetworkInterface* sender) const {

    Cell3DPosition except;
    if (sender) {
        module->getNeighborPos(module->getInterfaceId(sender), except);
    }
    for (const auto& cell : lattice->getActiveNeighborCells(module->position)) {
        if (lattice->cellsAreAdjacent(cell, targetPos) and cell != srcPos) {
            if (sender and cell == except) continue;
            RePoStBlockCode* block = static_cast<RePoStBlockCode*>(
                BaseSimulator::getWorld()->getBlockByPosition(cell)->blockCode);
            if (block->rotating) continue;

            return static_cast<Catoms3DBlock*>(lattice->getBlock(cell));
        }
    }
    return NULL;
}

Direction RePoStBlockCode::getPreviousOpDir() {
    if(not lattice->cellHasBlock(seedPosition)) return Direction::UNDEFINED;
    if( static_cast<RePoStBlockCode*>(lattice->getBlock(seedPosition)->blockCode)->mainPathIn == Cell3DPosition(-1,-1,-1)){
        return Direction::UNDEFINED;
    }
    //if(not operation) return Direction::UNDEFINED;
    
//    if(not operation->isTransfer()) VS_ASSERT(false);
    
    if(static_cast<RePoStBlockCode*>(lattice->getBlock(seedPosition)->blockCode)->isSource)
        return Direction::UNDEFINED;
    Cell3DPosition prevDirVector = MMPosition - 
        static_cast<RePoStBlockCode*>(lattice->getBlock(seedPosition)->blockCode)->mainPathIn;
    if (prevDirVector.pt[0] == -1) return Direction::LEFT;
    if (prevDirVector.pt[0] == 1) return  Direction::RIGHT;
    if (prevDirVector.pt[1] == -1)return Direction::FRONT;
    if (prevDirVector.pt[1] == 1) return  Direction::BACK;
    if (prevDirVector.pt[2] == -1)return Direction::DOWN;
    if (prevDirVector.pt[2] == 1) return  Direction::UP;
}

Direction RePoStBlockCode::getNextOpDir() {
    if(not lattice->cellHasBlock(seedPosition) or not operation)
        return Direction::UNDEFINED;
    if (static_cast<RePoStBlockCode*>(lattice->getBlock(seedPosition)->blockCode)
            ->mainPathOut.empty())
        return Direction::UNDEFINED;
    Cell3DPosition nextSeed;
    Init::getNeighborMMSeedPos(seedPosition, MMPosition, operation->getDirection(), nextSeed);
    Cell3DPosition nextCoordinatorPos =static_cast<RePoStBlockCode*>(lattice->getBlock(nextSeed)->blockCode)->coordinatorPosition; 
    return static_cast<RePoStBlockCode*>(lattice->getBlock(nextCoordinatorPos)->blockCode)
        ->operation->getDirection();
}

vector<Catoms3DBlock*> RePoStBlockCode::findNextLatchingPoints(const Cell3DPosition& targetPos,
                                                               const Cell3DPosition& pivotPos) {
    vector<Catoms3DBlock*> latchingPoints;
    for (auto lp : lattice->getActiveNeighborCells(targetPos)) {
        RePoStBlockCode& rlp = *static_cast<RePoStBlockCode*>(lattice->getBlock(lp)->blockCode);
        if (rlp.operation) {
            if (rlp.operation->isDismantle() and rlp.movingState == WAITING) {
                continue;
            }
        }
        if (lp != pivotPos and lp != module->position and not rlp.rotating) {
            latchingPoints.push_back(static_cast<Catoms3DBlock*>(lattice->getBlock(lp)));
        }
    }
    if (targetPos - seedPosition == Cell3DPosition(-2, 1, 2) and
        lattice->cellHasBlock(seedPosition + Cell3DPosition(-3, 0, 2)) and
        operation->isTransfer() and operation->getDirection() == Direction::LEFT) {
        // Avoid blocking when passing the bridge if the next operation direction is back
        RePoStBlockCode& rlp = *static_cast<RePoStBlockCode*>(
            lattice->getBlock(seedPosition + Cell3DPosition(-3, 0, 2))->blockCode);
        if (rlp.movingState == WAITING) {
            latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                lattice->getBlock(seedPosition + Cell3DPosition(-3, 0, 2))));
        }
    }
    /* ------------------ Avoid blocking when coming from back ------------------ */
    if (operation->getDirection() == BACK and shapeState == BACKFRONT) {
        if (relativePos() == Cell3DPosition(0, 2, 2) and
            lattice->cellHasBlock(module->position + Cell3DPosition(1, 1, -1))) {
            latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                lattice->getBlock(module->position + Cell3DPosition(1, 1, -1))));
        }
        if (relativePos() == Cell3DPosition(0, 2, 1) and operation->isTransfer()) {
            latchingPoints.clear();
        }
        if (relativePos() == Cell3DPosition(-1, 3, 3)) {
            latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                lattice->getBlock(module->position + Cell3DPosition(2, 0, -2))));
        }
    }
    /* -------------------------------------------------------------------------- */
    /* ------- Avoid deadlock when transfering left and coming from below ------- */

    if (operation->isTransfer() and operation->getDirection() == Direction::LEFT) {
        if ((*operation->localRules)[mvt_it].nextPosition == Cell3DPosition(0, -1, 1)) {
            // BACKFRONG
            latchingPoints.clear();
            latchingPoints.push_back(static_cast<Catoms3DBlock*>(lattice->getBlock(seedPosition)));
            if (lattice->cellHasBlock(seedPosition.offsetY(-1))) {
                latchingPoints.push_back(
                    static_cast<Catoms3DBlock*>(lattice->getBlock(seedPosition.offsetY(-1))));
            }
        } else if ((*operation->localRules)[mvt_it].nextPosition == Cell3DPosition(0, 0, 1)) {
            // FRONTBACK
            latchingPoints.clear();
            latchingPoints.push_back(static_cast<Catoms3DBlock*>(lattice->getBlock(seedPosition)));
            if (lattice->cellHasBlock(seedPosition.offsetY(1))) {
                latchingPoints.push_back(
                    static_cast<Catoms3DBlock*>(lattice->getBlock(seedPosition.offsetY(1))));
            }
        }
    }
    /* -------------------------------------------------------------------------- */

     /* ------ Avoid blocking when FB dismantling left then transfering down ----- */
    if (operation->isDismantle() and operation->getDirection() == Direction::LEFT) {
        if (targetPos - seedPosition == Cell3DPosition(-1, 0, 2) and
            lattice->cellHasBlock(seedPosition + Cell3DPosition(-2, 0,1))) {

            latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                lattice->getBlock(seedPosition + Cell3DPosition(-2, 0, 1))));
        }
    }
    /* -------------------------------------------------------------------------- */

    /* ------------------ Avoid blocking when transfering down ------------------ */
    if(operation->isTransfer() and operation->getDirection() == Direction::DOWN) {
        //FRONTBACK
        if (operation->getMMShape() == FRONTBACK) {
            if (relativePos() == Cell3DPosition(1, 0, 0)) {
                // Zeven
                latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                    lattice->getBlock(module->position + Cell3DPosition(1, 2, -2))));
                latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                    lattice->getBlock(module->position + Cell3DPosition(0, 1, -3))));
            } else if (relativePos() == Cell3DPosition(1, -1, 0)) {
                // Zodd
                latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                    lattice->getBlock(module->position + Cell3DPosition(1, 1, -2))));
                latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                    lattice->getBlock(module->position + Cell3DPosition(0, 0, -3))));
            }
        } else { // operation.shape = BACKFRONT
            if(relativePos() == Cell3DPosition(1, 0, 0)) {
                // Zeven
                latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                    lattice->getBlock(module->position + Cell3DPosition(1, -2, -2))));
                latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                    lattice->getBlock(module->position + Cell3DPosition(0, -2, -3))));
            }else if(relativePos() == Cell3DPosition(1, 1, 0)) {
                // Zodd
                latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                    lattice->getBlock(module->position + Cell3DPosition(1, -1, -2))));
                latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                    lattice->getBlock(module->position + Cell3DPosition(0, -1, -3))));
            }
        }
    }
    /* -------------------------------------------------------------------------- */

    /* ---------- Avoid deadlock when transfering down then going back ---------- */
    if (operation->isTransfer() and operation->getDirection() == Direction::BACK) {
        if (not static_cast<Transfer_Operation*>(operation)->isComingFromBack() and
            operation->getMMShape() == FRONTBACK) {
            if (operation->isZeven() and mvt_it > 5 and
                (*operation->localRules)[mvt_it].currentPosition == Cell3DPosition(1, 0, 2)) {
                latchingPoints.clear();
                latchingPoints.push_back(
                    static_cast<Catoms3DBlock*>(lattice->getBlock(module->position.offsetY(2))));
            } else if (not operation->isZeven() and mvt_it > 5 and
                       (*operation->localRules)[mvt_it].nextPosition == Cell3DPosition(1, 0, 2)) {
                latchingPoints.clear();
                latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                    lattice->getBlock(seedPosition + Cell3DPosition(1, 2, 2))));
            } else if (mvt_it == 2) {
                latchingPoints.clear();
                latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                    lattice->getBlock(seedPosition + Cell3DPosition(1, 1, 2))));
            }
        }
    }
    /* -------------------------------------------------------------------------- */

    /* ------------------ Avoid blocking when transfering right ----------------- */
    if (operation->isTransfer() and operation->getDirection() == Direction::RIGHT) {
        if (((relativePos() == Cell3DPosition(2, 0, 2) or
              relativePos() == Cell3DPosition(3, 0, 2)) and
             operation->  getMMShape() == BACKFRONT and mvt_it > 3 and operation->isZeven() and
             getNextOpDir() != Direction::DOWN) or
             ((relativePos() == Cell3DPosition(2, 1, 2) or
              relativePos() == Cell3DPosition(3, 0, 1)) and
             operation->getMMShape() == BACKFRONT and mvt_it > 4 and not operation->isZeven() and
             getNextOpDir() != Direction::DOWN)) {
            latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                lattice->getBlock(seedPosition + Cell3DPosition(3, -1, 1))));
            latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                lattice->getBlock(seedPosition + Cell3DPosition(4, 0, 0))));
            latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                lattice->getBlock(seedPosition + Cell3DPosition(5, 0, 0))));
            latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                lattice->getBlock(seedPosition + Cell3DPosition(5, 0, 1))));
            if (lattice->cellHasBlock(seedPosition + Cell3DPosition(5, 1, 2)))
                latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                    lattice->getBlock(seedPosition + Cell3DPosition(5, 1, 2))));
            if (lattice->cellHasBlock(seedPosition + Cell3DPosition(5, 2, 2)))
                latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                    lattice->getBlock(seedPosition + Cell3DPosition(5, 2, 2))));
        }


        if (relativePos() == Cell3DPosition(1, 0, 1) and mvt_it >= 7 and
            operation->getMMShape() == BACKFRONT and operation->isZeven()) {
            // BF
            latchingPoints.clear();
            latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                lattice->getBlock(seedPosition + Cell3DPosition(2, -1, 2))));
            if (lattice->cellHasBlock(seedPosition + Cell3DPosition(3, 0, 2)))
                latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                    lattice->getBlock(seedPosition + Cell3DPosition(3, 0, 2))));
        }

        if(relativePos() == Cell3DPosition(1,-1,1) and mvt_it >= 7 and operation->getMMShape() == FRONTBACK) {
            //FB
            latchingPoints.clear();
            latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                    lattice->getBlock(seedPosition + Cell3DPosition(2,1,2))));
            if(lattice->cellHasBlock(seedPosition + Cell3DPosition(3,0,2)))
                latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                        lattice->getBlock(seedPosition + Cell3DPosition(3,0,2))));
        }
    }

    /* -------------------------------------------------------------------------- */

    if(operation->isBuild()) {
        switch(operation->getDirection()) {
            case Direction::UP: {
                if(operation->getMMShape() == FRONTBACK) {
                    if(relativePos() == Cell3DPosition(1,-1,3)) {
                        latchingPoints.clear();
                    }
                }
            } break;

            default: break;
        }
    }

    if(operation->isTransfer()) {
         switch(operation->getDirection()) {
            case Direction::FRONT: {
                if(operation->getMMShape() == FRONTBACK) {
                     if (relativePos() == Cell3DPosition(-1, -4, 3)) {
                        latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                            lattice->getBlock(module->position + Cell3DPosition(2 , 0, -2))));
                    }
                } else { //BackFront
                    if(/*relativePos() == Cell3DPosition(1,0,2) and*/ lattice->cellHasBlock(seedPosition + Cell3DPosition(1,-2,2)) and 
                    relativePos().pt[1] > -2) {
                        latchingPoints.clear();
                        latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                            lattice->getBlock(seedPosition + Cell3DPosition(1,-2,2))));
                    }
                    if(relativePos() == Cell3DPosition(0,-2,2)) {
                   
                        latchingPoints.clear();
                        latchingPoints.push_back(static_cast<Catoms3DBlock*>(
                            lattice->getBlock(seedPosition + Cell3DPosition(1,-3,1))));
                    }
                }
            } break;

            default: break;
        }
    }
    return latchingPoints;
}

vector<Cell3DPosition> RePoStBlockCode::getAdjacentMMSeeds() {
    vector<Cell3DPosition> adjacentMMSeeds;
    for(int d = Direction::UP; d <= Direction::BACK; d++) {
        Cell3DPosition adjacentSeed;
        Init::getNeighborMMSeedPos(seedPosition, MMPosition, static_cast<Direction>(d), adjacentSeed);
        if(lattice->cellHasBlock(adjacentSeed)) {
            //sendMessage(_msg.get(), interfaceTo(adjacentSeed), 100, 200);
          adjacentMMSeeds.push_back(adjacentSeed);
        }
    }
    return adjacentMMSeeds;
}
/* -------------------------------------------------------------------------- */
bool RePoStBlockCode::setGreenLight(bool onoff) {
    stringstream info;
    info << " light turned ";

    if (onoff) {
        info << "green: ";
        greenLightIsOn = true;
        // module->setColor(initialColor);
        module->setColor(GREY);
        // Resume flow if needed
        if (not awaitingSources.empty()) {
            for (auto as : awaitingSources) {
                if (lattice->cellHasBlock(as))
                    sendHandleableMessage(new GLOMessage(as),
                                          module->getInterface(nearestPositionTo(as)), 100, 200);
            }
            awaitingSources.clear();
        }

    } else {
        info << "red: ";
        greenLightIsOn = false;
        // module->setColor(RED);
    }

    getScheduler()->trace(info.str(), module->blockId, onoff ? GREEN : RED);
}

bool RePoStBlockCode::isPotentialDestination() {
    //Check if an adjacent position is in target shape
    for (auto adjPos : getAdjacentMMPositions()) {
        if (inTargetShape(adjPos) and not inInitialShape(adjPos) and not
            lattice->cellHasBlock(getSeedPositionFromMMPosition(adjPos))) {
            if(find(destinations.begin(), destinations.end(), adjPos) != destinations.end()) {
                continue;
            }
            if(adjPos.pt[0] == MMPosition.pt[0] -1) continue;
            destinationOut = adjPos;
            return true;
        }
    }
    return false;
}

bool RePoStBlockCode::isPotentialSource() {
    if(inInitialShape(MMPosition) and not inTargetShape(MMPosition)) {
        return true;
    } else {
        return false;
    }
}

bool RePoStBlockCode::inTargetShape(Cell3DPosition pos) {
    for (auto targetPos : targetMap) {
        Cell3DPosition tp = Cell3DPosition(targetPos[0], targetPos[1], targetPos[2]);
        if (tp == pos) {
            return true;
        }
    }
    return false;
}

bool RePoStBlockCode::inInitialShape(Cell3DPosition pos) {
    for (auto initialPos : initialMap) {
        Cell3DPosition ip = Cell3DPosition(initialPos[0], initialPos[1], initialPos[2]);
        if (ip == pos) {
            return true;
        }
    }
    return false;
}

vector<Cell3DPosition> RePoStBlockCode::getAdjacentMMPositions() {
    vector<Cell3DPosition> adjacentPos;
    const vector<Cell3DPosition> relativePositions = {
        Cell3DPosition(1, 0, 0),  Cell3DPosition(-1, 0, 0), Cell3DPosition(0, 1, 0),
        Cell3DPosition(0, -1, 0), Cell3DPosition(0, 0, 1),  Cell3DPosition(0, 0, -1)};
    for(auto pos: relativePositions) {
        adjacentPos.push_back(MMPosition + pos);
    }
    return adjacentPos;
}


void RePoStBlockCode::updateState() {
    console << "Update State!!\n";
    operation->updateState(this);
    if(not operation->isTransfer() and not operation->isDismantle()) {
        operation =new Operation();
    }
   
    mvt_it = 0;
    isCoordinator = false;
    // module->setColor(initialColor);
    if(not greenLightIsOn) {
        setGreenLight(true);
    }
}

bool RePoStBlockCode::isInMM(Cell3DPosition& neighborPosition) {
    Cell3DPosition position = (neighborPosition - seedPosition);
    if (shapeState == FRONTBACK) {
        bool inBorder =
            (find(FrontBackMM.begin(), FrontBackMM.end(), position) != FrontBackMM.end());
        bool inFill =
            (find(FillingPositions_FrontBack_Zeven.begin(), FillingPositions_FrontBack_Zeven.end(),
                  position) != FillingPositions_FrontBack_Zeven.end());
        if (inBorder) return true;
    } else if (shapeState == BACKFRONT) {
        bool inBorder =
            (find(BackFrontMM.begin(), BackFrontMM.end(), position) != BackFrontMM.end());
        bool inFill =
            (find(FillingPositions_BackFront_Zeven.begin(), FillingPositions_BackFront_Zeven.end(),
                  position) != FillingPositions_BackFront_Zeven.end());
        if (inBorder) return true;
    } else {
    }
    return false;
}

Cell3DPosition RePoStBlockCode::nearestPositionTo(Cell3DPosition& targetPosition,
                                                  P2PNetworkInterface* except) {
    int distance = INT32_MAX;
    Cell3DPosition nearest;
    if (isAdjacentToPosition(targetPosition) and module->getInterface(targetPosition)) {
        return targetPosition;
    }
    for (auto neighPos : lattice->getActiveNeighborCells(module->position)) {
        RePoStBlockCode* neighBlock = static_cast<RePoStBlockCode*>(
            BaseSimulator::getWorld()->getBlockByPosition(neighPos)->blockCode);
        if (neighBlock->module->getState() == BuildingBlock::State::MOVING or
            neighBlock->movingState == MOVING)
            continue;
        if (except != nullptr) {
            if (module->getInterface(neighPos) == except) {
                continue;
            }
        }
        VS_ASSERT(neighBlock->module->getState() != BuildingBlock::State::MOVING);
        int d = BaseSimulator::getWorld()->lattice->getCellDistance(neighPos, targetPosition);
        if (d < distance) {
            distance = d;
            nearest = neighPos;
        }
    }
    if (distance == INT32_MAX) {
        module->getNeighborPos(module->getInterfaceId(except), nearest);
        return nearest;
    }
    VS_ASSERT(distance != DBL_MAX and module->getInterface(nearest)->isConnected());
    return nearest;
}

Cell3DPosition RePoStBlockCode::nextInBorder(P2PNetworkInterface* sender = nullptr) {
    Cell3DPosition currentPosition = module->position - seedPosition;
    vector<Cell3DPosition> possibleNexts;
    switch (shapeState) {
    case FRONTBACK: {
        int i;
        for(i = 0; i < FrontBackMM.size(); i++) {
            if(FrontBackMM[i] == currentPosition)
                break;
        }
        if(i == 0) {
            possibleNexts.push_back(FrontBackMM[1]);
            possibleNexts.push_back(FrontBackMM[FrontBackMM.size()-1]);
        } else if(i == FrontBackMM.size()-1) {
            possibleNexts.push_back(FrontBackMM[0]);
            possibleNexts.push_back(FrontBackMM[i-1]);
        } else {
            possibleNexts.push_back(FrontBackMM[i+1]);
            possibleNexts.push_back(FrontBackMM[i-1]);
        }
        break;
    }
    case BACKFRONT: {
        int i;
        for(i = 0; i < BackFrontMM.size(); i++) {
            if(BackFrontMM[i] == currentPosition)
                break;
        }
        if(i == 0) {
            possibleNexts.push_back(BackFrontMM[1]);
            possibleNexts.push_back(BackFrontMM[BackFrontMM.size()-1]);
        } else if(i == BackFrontMM.size()-1) {
            possibleNexts.push_back(BackFrontMM[0]);
            possibleNexts.push_back(BackFrontMM[i-1]);
        } else {
            possibleNexts.push_back(BackFrontMM[i+1]);
            possibleNexts.push_back(BackFrontMM[i-1]);
        }
        break;
    }
    default:
        break;
    }
    if(sender != nullptr)
        for(auto next: possibleNexts) {
            if(module->getInterface(next+seedPosition) != sender )
                return next;
        }
    return possibleNexts[0];
}

Cell3DPosition RePoStBlockCode::getSeedPositionFromMMPosition(Cell3DPosition &MMPos) {
    Cell3DPosition seedPos = Cell3DPosition();
    seedPos.set(GC->position.pt[0] + (MMPos.pt[0] * 4), GC->position.pt[1] + (MMPos.pt[1] * 3),
                GC->position.pt[2] + (MMPos.pt[2] * 4));
    if (MMPos.pt[2] % 2 != 0) 
        seedPos =  seedPos.offsetY(-1);
    return seedPos;
}

P2PNetworkInterface * RePoStBlockCode::interfaceTo(Cell3DPosition& dstPos, P2PNetworkInterface *sender) {
    if(sender)
        return module->getInterface(nearestPositionTo(dstPos, sender));
    else 
        return module->getInterface(nearestPositionTo(dstPos));
}

P2PNetworkInterface* RePoStBlockCode::interfaceTo(Cell3DPosition& fromMM, Cell3DPosition& toMM,
                                                  P2PNetworkInterface* sender) {
    Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(toMM);
    if (not lattice->cellHasBlock(toSeedPosition)) {
        cerr << toSeedPosition << "\n";
        VS_ASSERT_MSG(false, "not connected");
    }
    if (lattice->cellsAreAdjacent(module->position, toSeedPosition))
        return module->getInterface(toSeedPosition);
    Cell3DPosition fromSeedPosition = getSeedPositionFromMMPosition(fromMM);
    Direction direction;
    Cell3DPosition directionVector = toMM - fromMM;
    if (directionVector.pt[0] == -1) direction = Direction::LEFT;
    if (directionVector.pt[0] == 1) direction = Direction::RIGHT;
    if (directionVector.pt[1] == -1) direction = Direction::FRONT;
    if (directionVector.pt[1] == 1) direction = Direction::BACK;
    if (directionVector.pt[2] == -1) direction = Direction::DOWN;
    if (directionVector.pt[2] == 1) direction = Direction::UP;
    RePoStBlockCode* fromSeed = static_cast<RePoStBlockCode*>(
        BaseSimulator::getWorld()->getBlockByPosition(fromSeedPosition)->blockCode);
    vector<Cell3DPosition> routingVector =
        getRoutingVector(direction, fromSeed->shapeState, fromSeed->MMPosition.pt[2]);
    Cell3DPosition relativePosition = module->position - fromSeedPosition;
    vector<Cell3DPosition>::iterator it =
        find(routingVector.begin(), routingVector.end(), relativePosition);
    Cell3DPosition nextPos;
    if (it != routingVector.end()) {
        nextPos = *(it + 1);
    } else {
        VS_ASSERT(false);
    }
    VS_ASSERT(module->getInterface(nextPos + fromSeedPosition));
    return module->getInterface(nextPos + fromSeedPosition);
}

void RePoStBlockCode::onMotionEnd() {
    console << " has reached its destination"
            << "\n";

    LocalMovement lmvt = (*operation->localRules)[mvt_it];
    console << lmvt.nextPosition << "\n";
    movingState = lmvt.state;
    movingSteps++;
    console << "movingSteps: " << movingSteps << "\n"; 
    if (movingState == MOVING) {
        mvt_it++;
        probeGreenLight();
    } else if (movingState == WAITING or movingState == IN_POSITION) {
        transferCount = 0;
        rotating = false;
        if (operation->mustSendCoordinateBack(this)) {
            sendingCoordinateBack = true;
            sendHandleableMessage(new CoordinateBackMessage(movingSteps, coordinatorPosition),
                module->getInterface(nearestPositionTo(coordinatorPosition)), 100, 200);
        }
        console << "coordinator position: " << coordinatorPosition << "\n";
        movingSteps = 0;
        P2PNetworkInterface* pivotItf = module->getInterface(pivotPosition);  
        // if (operation->isFill() or operation->isBuild() ) {
        //     for(auto neighbor: module->getNeighbors()) {
        //         sendHandleableMessage(new FTRMessage(), module->getP2PNetworkInterfaceByBlockRef(neighbor),
        //             100, 200);
        //     }
        // } else if (pivotItf and pivotItf->isConnected()) {
        //     sendHandleableMessage(new FTRMessage(), module->getInterface(pivotPosition), 100, 200);
                
        // }
        for(auto neighbor: module->getNeighbors()) {
            sendHandleableMessage(new FTRMessage(), module->getP2PNetworkInterfaceByBlockRef(neighbor),
                100, 200);
        }
        if (movingState == IN_POSITION) {
            console << "mvt_it in pos: " << mvt_it << "\n";
            if((operation->isBuild() or operation->isFill()) and mvt_it == (*operation->localRules).size() -1) {
                cerr << "Destination reached" << endl;
                NbOfDestinationsReached++;
                cerr << NbOfDestinationsReached << " " << NbOfStreamlines << endl;
                if(NbOfDestinationsReached == NbOfStreamlines) {

                    cerr << "REINITIALIZE" << endl;
                    reinitialize();
                    RePoStBlockCode* seedMM = static_cast<RePoStBlockCode*>(GC->blockCode);
                    reconfigurationStep = SRCDEST;
                    nbOfIterations++;
                    seedMM->nbWaitedAnswers = 0;
                    seedMM->distance = 0;
                    for (auto p : seedMM->getAdjacentMMSeeds()) {
                        Cell3DPosition toMMPosition =
                            static_cast<RePoStBlockCode*>(
                                BaseSimulator::getWorld()->getBlockByPosition(p)->blockCode)
                                ->MMPosition;
                        seedMM->sendHandleableMessage(new GoMessage(seedMM->MMPosition, toMMPosition, seedMM->distance),
                        seedMM->interfaceTo(seedMM->MMPosition, toMMPosition), 100, 200);
                        seedMM->nbWaitedAnswers++;
                    }
                    
                }
            }
            updateState();
        }
    }
}

int RePoStBlockCode::sendHandleableMessage(HandleableMessage* msg, P2PNetworkInterface* dest,
                                           Time t0, Time dt) {
    return BlockCode::sendMessage(msg, dest, t0, dt);
}

void RePoStBlockCode::processLocalEvent(EventPtr pev) {
    std::shared_ptr<Message> message;
    stringstream info;
    if (pev->eventType == EVENT_RECEIVE_MESSAGE) {
        message = (std::static_pointer_cast<NetworkInterfaceReceiveEvent>(pev))->message;

        if (message->type >= 1008 and message->type <= 1013 and module->position == seedPosition) {
            state = ACTIVE;
            cont_passive = false;
            Cell3DPosition fromMMPosition;
            HandleableMessage* msg;
            switch (message->type) {
                case 1008:
                    fromMMPosition = static_cast<BFSMessage*>(message.get())->getFromMMPosition();
                    break;
                case 1009:
                    fromMMPosition =
                        static_cast<ConfirmEdgeMessage*>(message.get())->getFromMMPosition();
                    break;
                case 1010:
                    fromMMPosition =
                        static_cast<ConfirmPathMessage*>(message.get())->getFromMMPosition();
                    break;
                case 1011:
                    fromMMPosition =
                        static_cast<ConfirmStreamlineMessage*>(message.get())->getFromMMPosition();
                    break;
                case 1012:
                    fromMMPosition =
                        static_cast<AvailableMessage*>(message.get())->getFromMMPosition();
                    break;
                case 1013:
                    fromMMPosition =
                        static_cast<CutOffMessage*>(message.get())->getFromMMPosition();
                    break;

                default: {
                    VS_ASSERT(false);
                }
            }
            console << "defecit: " << deficit << "\n";
            sendHandleableMessage(new AckMessage(MMPosition, fromMMPosition),
                                  interfaceTo(MMPosition, fromMMPosition), 100, 200);
        }
        if (message->isMessageHandleable()) {
            std::shared_ptr<HandleableMessage> hMsg =
                (std::static_pointer_cast<HandleableMessage>(message));
            console << " received " << hMsg->getName() << " from "
                    << message->sourceInterface->hostBlock->blockId << " at "
                    << getScheduler()->now() << "\n";
            hMsg->handle(this);
        }
    }
    // Do not remove line below
    
    Catoms3DBlockCode::processLocalEvent(pev);
    if(not initialized) {
        return;
    } 
    switch (pev->eventType) {

        case EVENT_ROTATION3D_START: {
            VS_ASSERT(module->pivot);
            pivotPosition = module->pivot->position;
            break;
        }
        case EVENT_ROTATION3D_END: {
            break;
        }
        case EVENT_ADD_NEIGHBOR: {
            // Do something when a neighbor is added to an interface of the module
             
            uint64_t face = Catoms3DWorld::getWorld()->lattice->
                getOppositeDirection((std::static_pointer_cast<AddNeighborEvent>(pev))
                                    ->face);
            if(module->getNeighborBlock(face) == NULL) return;
            Cell3DPosition& pos = module->getNeighborBlock(face)->position;
            
            RePoStBlockCode *posBlock = static_cast<RePoStBlockCode*>(
                BaseSimulator::getWorld()->getBlockByPosition(pos)->blockCode
            );
            console << "ADD NEIGHBOR: " << pos << "\n";
            if(not rotating and posBlock->rotating or (!posBlock->rotating and isCoordinator) ) {
                setGreenLight(false);
            }
            if(not operation) return;
            operation->handleAddNeighborEvent(this, pos);
        

            /**Special logic when the end position of previous transfer back with FB shape operation is (0,1,1) relative to the coordinator
             Specify if the module must move to the starting position if next operation is not transfer back BF **/
            if (isCoordinator and pos == module->position + Cell3DPosition(0, 1, 1)) {
                if ((not operation->isTransfer() and not operation->isFill() and
                     operation->getDirection() != Direction::UP and not operation->isBuild()) or
                    (operation->isTransfer() and operation->getDirection() == Direction::UP and
                     static_cast<Transfer_Operation*>(operation)->isComingFromBack()) /*or
                    (operation->isBuild() and operation->getDirection() == Direction::BACK and
                     operation->getMMShape() == BACKFRONT and not
                        static_cast<Build_Operation*>(operation)->isComingFromBack())*/) {
                    if (posBlock->module->canMoveTo(module->position.offsetY(1)) and not posBlock->sendingCoordinateBack) {
                        console << "move pos BF\n";
                        posBlock->module->moveTo(module->position.offsetY(1));
                    } else {
                        // Wait until it can move to the desired position
                        getScheduler()->schedule(new InterruptionEvent(
                            getScheduler()->now() + getRoundDuration(), posBlock->module,
                            IT_MODE_TRANSFERBACK_REACHCOORDINATOR));
                    }
                }
            }

            if(isCoordinator and pos == module->position + Cell3DPosition(0,0,1) and shapeState == FRONTBACK) {
                if (operation->getDirection() != Direction::FRONT and getPreviousOpDir() == Direction::FRONT) {
                     posBlock->movingSteps--;
                     if (posBlock->module->canMoveTo(module->position.offsetY(-1)) and not posBlock->sendingCoordinateBack) {
                            console << "move pos FB\n";
                        posBlock->module->moveTo(module->position.offsetY(-1));
                    } else {
                        posBlock->console << "SendingCoordnateBack: " << posBlock->sendingCoordinateBack << "\n";
                        // Wait until it can move to the desired position
                        getScheduler()->schedule(new InterruptionEvent(
                            getScheduler()->now() + getRoundDuration(), posBlock->module,
                            IT_MODE_TRANSFERBACK_REACHCOORDINATOR));
                    }
                }
            }

            break;
        }

        case EVENT_REMOVE_NEIGHBOR: {
            // Do something when a neighbor is removed from an interface of the module
            
            if (not rotating and movingState != MOVING) {

                uint64_t face = Catoms3DWorld::getWorld()->lattice->getOppositeDirection(
                    (std::static_pointer_cast<RemoveNeighborEvent>(pev))->face);

                Cell3DPosition pos;
                
                if (module->getNeighborPos(face, pos) and (module->getState() <= 3) ) {
                    console << "REMOVE NEIGHBOR: " << pos << "\n";
                    bool hasMovingNeighbors = false;
                    for(auto n: lattice->getActiveNeighborCells(module->position)) {
                        if(n == pos) continue;
                        if(static_cast<RePoStBlockCode*>(lattice->getBlock(n)->blockCode)->movingState == MOVING) {
                            hasMovingNeighbors = true;
                            break;
                        }
                    }
                    //if(not hasMovingNeighbors)
                        setGreenLight(true);
                }
            }
            break;
        }
        case EVENT_PIVOT_ACTUATION_END: {
            if(not rotating) {
                std::shared_ptr<PivotActuationEndEvent> paee =
                    std::static_pointer_cast<PivotActuationEndEvent>(pev);
                uint64_t face = Catoms3DWorld::getWorld()->
                        lattice->getOppositeDirection(paee->toConP);
                Cell3DPosition pos;
            }
            

            break;
        }
        case EVENT_INTERRUPTION: {
            std::shared_ptr<InterruptionEvent> itev =
                std::static_pointer_cast<InterruptionEvent>(pev);

            switch(itev->mode) {
                case IT_MODE_FINDING_PIVOT: {
                    if (!rotating or module->getState() == BuildingBlock::State::MOVING) return;
                    // VS_ASSERT(++notFindingPivotCount < 10);
                    VS_ASSERT(operation->localRules.get());
                    probeGreenLight();  // the GC starts the algorithm
                    // module->setColor(MAGENTA);
                } break;

                case IT_MODE_TRANSFERBACK: {
                 
                    if(not awaitingCoordinator or not operation) return;
                    Cell3DPosition targetModule =
                        seedPosition + (*operation->localRules)[mvt_it].currentPosition;
                    console << mvt_it  << " " << operation->localRules->size() << "\n";
                    if (not lattice->cellHasBlock(targetModule) and isCoordinator and mvt_it < operation->localRules->size()) {
                           getScheduler()->trace("TransferBack", module->blockId, RED);
                        getScheduler()->schedule(
                            new InterruptionEvent(getScheduler()->now() + getRoundDuration(),
                                                  module, IT_MODE_TRANSFERBACK));
                    } else {
                        if(lattice->cellHasBlock(targetModule) and isAdjacentToPosition(targetModule) and mvt_it < operation->localRules->size()) {
                            VS_ASSERT(operation);
                            sendHandleableMessage(new CoordinateMessage(operation, targetModule, module->position, mvt_it),
                                module->getInterface(targetModule), 100, 200);
                        }
                        awaitingCoordinator = false;
                    }
                } break;

                case IT_MODE_TRANSFERBACK_REACHCOORDINATOR: {
                    getScheduler()->trace("ReachCoordinator", module->blockId, RED);
                    if (shapeState == BACKFRONT and
                        module->canMoveTo(module->position.offsetZ(-1)) and
                        not sendingCoordinateBack) {
                        module->moveTo(module->position.offsetZ(-1));
                    } else if (shapeState == FRONTBACK and
                               module->canMoveTo(module->position + Cell3DPosition(0, -1, -1)) and
                               not sendingCoordinateBack) {
                        module->moveTo(module->position + Cell3DPosition(0, -1, -1));

                    } else {
                        console << "Trying to reach coordinator\n";
                        console << "SendingCoordnateBack: " << sendingCoordinateBack << "\n";
                        getScheduler()->schedule(
                            new InterruptionEvent(getScheduler()->now() + getRoundDuration(),
                                                  module, IT_MODE_TRANSFERBACK_REACHCOORDINATOR));
                    }
                } break;

                case IT_MODE_TERMINATION: {
                    stringstream ss;
                    ss << MMPosition << ": " << state << " ; " << deficit << "\n";
                    getScheduler()->trace(ss.str(), module->blockId, RED);
                     if (state == PASSIVE and deficit == 0) {
                        b = cont_passive;
                        cont_passive = true;
                        if (parentPosition != Cell3DPosition(-1, -1, -1)) {
                            Cell3DPosition parentSeedPosition = getSeedPositionFromMMPosition(parentPosition);
                            sendHandleableMessage(
                                new BackTermMessage(MMPosition, parentPosition, res and b),
                                interfaceTo(MMPosition, parentPosition), 100, 200);
                        } else {
                            start_wave();
                        }
                    } else {
                        getScheduler()->schedule(
                            new InterruptionEvent(getScheduler()->now() + 500, module, IT_MODE_TERMINATION));
                    }
                } break;

                case IT_MODE_NBMOVEMENTS: {
                    if (reconfigurationStep != DONE) {
                        timeStep++;
                        int nbOfModulesInMvt = 0;
                        for (auto id_block : BaseSimulator::getWorld()->buildingBlocksMap) {
                            RePoStBlockCode* MMBlock =
                                static_cast<RePoStBlockCode*>(id_block.second->blockCode);

                            if (MMBlock->movingState == MOVING or MMBlock->movingState == WAITING) {
                                nbOfModulesInMvt++;
                            }
                        }
                        ofstream file;
                        file.open("nbMovementsPerTimeStep.txt", ios::out | ios::app);
                        file << timeStep << ',' << nbOfModulesInMvt << ',' << NbOfStreamlines
                             << '\n';
                        getScheduler()->schedule(
                            new InterruptionEvent(getScheduler()->now() + getRoundDuration(),
                                                  module, IT_MODE_NBMOVEMENTS));
                    }
                } break;
            }
        }
    }
}

/// ADVANCED BLOCKCODE FUNCTIONS BELOW

void RePoStBlockCode::onBlockSelected() {
    // Debug stuff:
    cerr << endl << "--- PRINT MODULE " << *module << "---" << endl;
    cerr << "isSeed: " << (seedPosition == module->position ? "true": "false") << endl;
    cerr << "isCoordinator: " << (isCoordinator ? "true": "false") << endl;
    cerr << "coordinatorPosition: " << coordinatorPosition << endl;
    cerr << "ShapeState: " << shapeState << endl;
    cerr << "seedPosition: " << seedPosition << endl;
    cerr << "MMPostion: " << MMPosition << endl;
    cerr << "CurrentPos: " << module->position - seedPosition << endl;
    cerr << "mvt_it: " << mvt_it << endl;
    cerr << "MovingState: " << movingState << endl;
    cerr << "GreenLight: " << greenLightIsOn << endl;
    cerr << "isSource: " << isSource << endl;
    cerr << "isDestination: " << isPotentialDestination() << endl;
    // cerr << "parentPosition: " << parentPosition << endl;
    // cerr << "childrenPostions: ";
    // for(auto c: childrenPositions) cerr << c << "; ";
    // cerr << endl;
    // cerr << "distance: " << distance << endl;
    // cerr << "mainPathState: " << mainPathState << endl;
    // // cerr << "aug1PathState: " << aug1PathState << endl;
    // // cerr << "aug2PathState: " << aug2PathState << endl;
    cerr << "mainPathIn: " << mainPathIn << endl;
    cerr << "mainPathOut: ";
    for(auto out: mainPathOut) cerr << out << " | "; 
    cerr << endl;
    // if(isDestination) cerr << "destinationFor: " << destinationOut << endl;
    if(operation) {
        if(operation->isTransfer())
        cerr << "prevOpDir: " << operation->getPrevOpDirection() << endl;
    }
    // cerr << endl;
    // cerr << "aug1PathIn: " << aug1PathIn << endl;
    // cerr << "aug1PathOut: ";
    // for(auto out: aug1PathOut) cerr << out << " | ";
    // cerr << endl;
    // cerr << "aug2PathIn: " << aug2PathIn << endl;
    // cerr << "aug2PathOut: ";
    // for(auto out: aug2PathOut) cerr << out << " | ";
    // cerr << endl;
    // cerr << "mainPathsOld: ";
    // for(auto old: mainPathsOld) cerr << old << " | ";
    // cerr << endl;
    // cerr << "aug1PathsOld: ";
    // for(auto old: aug1PathsOld) cerr << old << " | ";
    // cerr << endl;
    // cerr << "aug2PathsOld: ";
    // for(auto old: aug2PathsOld) cerr << old << " | ";
    // cerr << endl;
    // cerr << "deficit: " << deficit << endl;
    // cerr << "state: " << state << endl;
}

void RePoStBlockCode::onUserKeyPressed(unsigned char c, int x, int y) {
    if(c == 'C' and Init::initialMapBuildDone) {
        showSrcAndDst = not showSrcAndDst;
        // color sources in RED, destinations in GREEN in other MMs in White
        for(auto id_block: BaseSimulator::getWorld()->buildingBlocksMap) {
            RePoStBlockCode* block = static_cast<RePoStBlockCode*>(id_block.second->blockCode);
            if(showSrcAndDst) {
                switchModulesColors();
            } else {
                block->module->setColor(block->initialColor);
            }
        }
    }
  
    if(!BaseSimulator::getWorld()->selectedGlBlock) {
        cerr << "No block selected!" << endl;
        return;
    }
    int blockId = BaseSimulator::getWorld()->selectedGlBlock->blockId;
    RePoStBlockCode* block = static_cast<RePoStBlockCode*>(
        BaseSimulator::getWorld()->getBlockById(blockId)->blockCode
    );
    
    ofstream file;
    // if(c == 'J') {
    //     seedPosition = Cell3DPosition(2,19,14);
    //     file.open("routes.txt", ios::out | ios::app);
    //     if(!file.is_open()) return;
    //     file << "Cell3DPosition" <<  block->module->position - block->seedPosition << ", ";
    //     return;
    // }
    file.open("FB_Build_Back.txt", ios::out | ios::app);
    seedPosition = Cell3DPosition(10,11,8);
    if(!file.is_open()) return; 

    if(c == 'o') {
        file << "{LocalMovement(" << "Cell3DPosition" << block->module->position - block->seedPosition;
    }
    if(c == 'p') {
        file << ", Cell3DPosition" << block->module->position - block->seedPosition
                << ", MOVING)},\n";
    }
}

void RePoStBlockCode::switchModulesColors() {
    showSrcAndDst = true;
    // color sources in RED, destinations in GREEN in other MMs in White
    for (auto id_block : BaseSimulator::getWorld()->buildingBlocksMap) {
        RePoStBlockCode* block = static_cast<RePoStBlockCode*>(id_block.second->blockCode);
        if (static_cast<RePoStBlockCode*>(
                BaseSimulator::getWorld()->getBlockByPosition(block->seedPosition)->blockCode)
                ->isSource) {
            block->module->setColor(RED);
        } else if (block->isDestination or
                   static_cast<RePoStBlockCode*>(BaseSimulator::getWorld()
                                                     ->getBlockByPosition(block->seedPosition)
                                                     ->blockCode)
                       ->isDestination) {
            block->module->setColor(GREEN);
        } else {
            if (not static_cast<RePoStBlockCode*>(BaseSimulator::getWorld()
                                                      ->getBlockByPosition(block->seedPosition)
                                                      ->blockCode)
                        ->isDestination)
                block->getModule()->setColor(GREY);
        }
    }
}

void RePoStBlockCode::onAssertTriggered() {
    console << " has triggered an assert" << "\n";

    // Print debugging some info if needed below
    // ...
}

bool RePoStBlockCode::parseUserCommandLineArgument(int &argc, char **argv[]) {
    /* Reading the command line */
    if ((argc > 0) && ((*argv)[0][0] == '-')) {
        switch((*argv)[0][1]) {

            // Single character example: -b
            case 'b':   {
                cout << "-b option provided" << endl;
                return true;
            } break;

            // Composite argument example: --foo 13
            case '-': {
                string varg = string((*argv)[0] + 2); // argv[0] without "--"
                if (varg == string("foo")) { //
                    int fooArg;
                    try {
                        fooArg = stoi((*argv)[1]);
                        argc--;
                        (*argv)++;
                    } catch(std::logic_error&) {
                        stringstream err;
                        err << "foo must be an integer. Found foo = " << argv[1] << endl;
                        throw CLIParsingError(err.str());
                    }

                    cout << "--foo option provided with value: " << fooArg << endl;
                } else return false;

                return true;
            }

            default: cerr << "Unrecognized command line argument: " << (*argv)[0] << endl;
        }
    }

    return false;
}

void RePoStBlockCode::parseUserElements(TiXmlDocument *config) {
    TiXmlElement *worldElement = 
        config->RootElement()->NextSiblingElement();
    TiXmlElement *MMblocksElement = worldElement->FirstChildElement()->NextSiblingElement()->NextSiblingElement()->NextSiblingElement();
    TiXmlElement *blockElement = MMblocksElement->FirstChildElement();
    cerr << "Parsing MM positions\n";
    for (; blockElement != NULL; blockElement = blockElement->NextSiblingElement()) {
        string pos = blockElement->Attribute("position");

        int start = 0;
        int end = pos.find(",");
        int i = 0;
        array<int, 4> coord = {0};
        while (end != -1) {
            coord[i] = stoi(pos.substr(start, end - start));
            // cerr << i << ": " << coord[i] << endl;
            i++;

            start = end + 1;
            end = pos.find(",", start);
        }
        coord[i] = stoi(pos.substr(start, end - start));
        const char* filled = blockElement->Attribute("filled");
        // string filled =  blockElement->Attribute("filled");
        if (filled) {
            if (filled == string("true")) {
                coord[3] = 1;
            }
        }
        initialMap.push_back(coord);
    }

    // Parsing target positions
    TiXmlElement *MMtargetsElement = MMblocksElement->NextSiblingElement();
    if(not MMtargetsElement) {
        cerr << "No target positions are provided!!" << endl;
        return;
    }
    blockElement =  MMtargetsElement->FirstChildElement();
    for (; blockElement != NULL; blockElement = blockElement->NextSiblingElement()) {
        string pos = blockElement->Attribute("position");

        int start = 0;
        int end = pos.find(",");
        int i = 0;
        array<int, 4> coord = {0};
        while (end != -1) {
            coord[i] = stoi(pos.substr(start, end - start));
            // cerr << i << ": " << coord[i] << endl;
            i++;

            start = end + 1;
            end = pos.find(",", start);
        }
        coord[i] = stoi(pos.substr(start, end - start));

        const char* filled = blockElement->Attribute("filled");
        // string filled =  blockElement->Attribute("filled");
        if (filled) {
            if (filled == string("true")) {
                coord[3] = 1;
            }
        }
        targetMap.push_back(coord);
    }
}

string RePoStBlockCode::onInterfaceDraw() {
    stringstream trace;
    // trace << "Nb of modules " << BaseSimulator::getWorld()->getNbBlocks();
    return trace.str();
}
 
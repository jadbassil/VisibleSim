#include "metaModuleBlockCode.hpp"
#include "init.hpp"
#include "messages.hpp"
#include <fstream>
#include "robots/catoms3D/catoms3DMotionEngine.h"

using namespace Catoms3D;

MetaModuleBlockCode::MetaModuleBlockCode(Catoms3DBlock *host) : Catoms3DBlockCode(host) {
    // @warning Do not remove block below, as a blockcode with a NULL host might be created
    //  for command line parsing
    if (not host) return;

    // Registers a callback (handleSampleMessage) to the message of type SAMPLE_MSG_ID
    addMessageEventFunc2(COORDINATE_MSG_ID,
                         std::bind(&MetaModuleBlockCode::handleCoordinateMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(COORDINATEBACK_MSG_ID,
                         std::bind(&MetaModuleBlockCode::handleCoordinateBackMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));

    addMessageEventFunc2(PLS_MSG_ID,
                         std::bind(&MetaModuleBlockCode::handlePLSMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(GLO_MSG_ID,
                         std::bind(&MetaModuleBlockCode::handleGLOMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(FTR_MSG_ID,
                         std::bind(&MetaModuleBlockCode::handleFTRMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    // Set the module pointer
    module = static_cast<Catoms3DBlock*>(hostBlock);
}

void MetaModuleBlockCode::startup() {
    console << "start\n";
    //operation = new Operation();
    if(not Init::initialMapBuildDone) {
        initialColor = GREEN;
        initialized = false;
        MMPosition = Cell3DPosition(0,0,0);
        shapeState = FRONTBACK;
        seed = module;
        cout << seed->position << endl;
        Init::buildMM(module, FRONTBACK, Color(GREEN));
        if(initialMap[0][3] == 1) {
            Init::fillMM(seed);
        }
        Init::buildInitialMap(seed->position, initialMap);
        setGreenLight(true);
        rotating = false;
        seedPosition = module->position;
        Cell3DPosition targetModule;
     
        MetaModuleBlockCode *block49 = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockById(49)->blockCode
        );
        operation = new Operation(Direction::LEFT, BACKFRONT);
        block49->operation = new Dismantle_Operation(Direction::LEFT, FRONTBACK);
        block49->isCoordinator = true;

        targetModule = block49->seedPosition + (*block49->operation->localRules)[0].currentPosition;
        block49->console << "targetModule: " << block49->nearestPositionTo(targetModule) << "\n"; 
        block49->sendMessage("Coordinate Msg1", new MessageOf<Coordinate>(
            COORDINATE_MSG_ID, Coordinate(block49->operation, targetModule, block49->module->position, block49->mvt_it)),
            block49->module->getInterface(block49->nearestPositionTo(targetModule)), 100, 200
        );

        

        MetaModuleBlockCode *block13 = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockById(13)->blockCode
        );

        block13->operation = new Transfer_Operation(Direction::LEFT, BACKFRONT);
        block13->isCoordinator = true;

        MetaModuleBlockCode *block3 = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockById(3)->blockCode
        );
        block3->operation = new Transfer_Operation(Direction::LEFT, FRONTBACK);
        block3->isCoordinator = true;

        

        MetaModuleBlockCode *block23 = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockById(23)->blockCode
        );
        block23->operation = new Transfer_Operation(Direction::LEFT, BACKFRONT);
        block23->isCoordinator = true;

        // MetaModuleBlockCode *block29 = static_cast<MetaModuleBlockCode*>(
        //     BaseSimulator::getWorld()->getBlockById(29)->blockCode
        // );
        // operation = new Operation(Direction::LEFT, BACKFRONT);
        // block29->operation = new Dismantle_Operation(Direction::LEFT, BACKFRONT, MMPosition.pt[2], true);
        // block29->isCoordinator = true;

        // targetModule = block29->seedPosition + (*block29->operation->localRules)[0].currentPosition;
        // block29->console << "targetModule: " << block29->nearestPositionTo(targetModule) << "\n"; 
        // block29->sendMessage("Coordinate Msg1", new MessageOf<Coordinate>(
        //     COORDINATE_MSG_ID, Coordinate(block29->operation, targetModule, block29->module->position, block29->mvt_it)),
        //     block29->module->getInterface(block29->nearestPositionTo(targetModule)), 100, 200
        // );

        MetaModuleBlockCode *block33 = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockById(33)->blockCode
        );
        block33->operation = new Transfer_Operation(Direction::BACK, FRONTBACK);
        block33->isCoordinator = true;

        //  MetaModuleBlockCode *block43 = static_cast<MetaModuleBlockCode*>(
        //     BaseSimulator::getWorld()->getBlockById(43)->blockCode
        // );
        // block43->operation = new Build_Operation(Direction::UP, FRONTBACK, block43->MMPosition.pt[2]);
        // block43->isCoordinator = true;

        // MetaModuleBlockCode *block323 = static_cast<MetaModuleBlockCode*>(
        //     BaseSimulator::getWorld()->getBlockById(323)->blockCode
        // );
        // block323->operation = new Build_Operation(Direction::UP, FRONTBACK, block323->MMPosition.pt[2]);
        // block323->isCoordinator = true;

        MetaModuleBlockCode *block83 = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockById(83)->blockCode
        );
        block83->operation = new Fill_Operation(Direction::BACK, BACKFRONT, true, 0);
        block83->isCoordinator = true;

        // MetaModuleBlockCode *block113 = static_cast<MetaModuleBlockCode*>(
        //     BaseSimulator::getWorld()->getBlockById(113)->blockCode
        // );
        // block113->operation = new Fill_Operation(Direction::BACK, FRONTBACK, 0);
        // block113->isCoordinator = true;

        // MetaModuleBlockCode *block133 = static_cast<MetaModuleBlockCode*>(
        //     BaseSimulator::getWorld()->getBlockById(133)->blockCode
        // );
        // block133->operation = new Transfer_Operation(Direction::UP, BACKFRONT, block133->MMPosition.pt[2]);
        // block133->isCoordinator = true;

         MetaModuleBlockCode *block243 = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockById(243)->blockCode
        );
        block243->operation = new Transfer_Operation(Direction::UP, BACKFRONT, false,  block243->MMPosition.pt[2]);
        block243->isCoordinator = true;

          MetaModuleBlockCode *block253 = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockById(253)->blockCode
        );
        block253->operation = new Build_Operation(Direction::UP, BACKFRONT, block253->MMPosition.pt[2]);
        block253->isCoordinator = true;

        /***********************************Y=1*****************************************/
        // MetaModuleBlockCode *block99 = static_cast<MetaModuleBlockCode*>(
        //     BaseSimulator::getWorld()->getBlockById(99)->blockCode
        // );
        // block99->operation = new Dismantle_Operation(Direction::LEFT, BACKFRONT);
        // block99->isCoordinator = true;
        
        // targetModule = block99->seedPosition + (*block99->operation->localRules)[0].currentPosition;
        // block99->console << "targetModule: " << block99->nearestPositionTo(targetModule) << "\n"; 
        // block99->sendMessage("Coordinate Msg1", new MessageOf<Coordinate>(
        //     COORDINATE_MSG_ID, Coordinate(block99->operation, targetModule, block99->module->position, block99->mvt_it)),
        //     block99->module->getInterface(block99->nearestPositionTo(targetModule)), 100, 200
        // );
       
       MetaModuleBlockCode *block63 = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockById(63)->blockCode
        );

        block63->operation = new Transfer_Operation(Direction::LEFT, FRONTBACK);
        block63->isCoordinator = true;

         MetaModuleBlockCode *block53 = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockById(53)->blockCode
        );

        block53->operation = new Fill_Operation(Direction::LEFT, BACKFRONT);
        block53->isCoordinator = true;

         MetaModuleBlockCode *block73 = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockById(73)->blockCode
        );

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

    //     block123->operation = new Transfer_Operation(Direction::UP, FRONTBACK, false, block123->MMPosition.pt[2]);
    //     block123->isCoordinator = true;

    //        MetaModuleBlockCode *block363 = static_cast<MetaModuleBlockCode*>(
    //         BaseSimulator::getWorld()->getBlockById(363)->blockCode
    //     );

    //     block363->operation = new Transfer_Operation(Direction::UP, FRONTBACK, false, block363->MMPosition.pt[2]);
    //     block363->isCoordinator = true;

    //     MetaModuleBlockCode *block373 = static_cast<MetaModuleBlockCode*>(
    //         BaseSimulator::getWorld()->getBlockById(373)->blockCode
    //     );

    //     block373->operation = new Build_Operation(Direction::UP, FRONTBACK, block373->MMPosition.pt[2]);
    //     block373->isCoordinator = true;





        Init::initialMapBuildDone = true;
    }
    initialColor = module->color;
    initialized = true;

}

void MetaModuleBlockCode::handleCoordinateMessage(std::shared_ptr<Message> _msg,
                                               P2PNetworkInterface* sender) {                                 
    MessageOf<Coordinate>* msg = static_cast<MessageOf<Coordinate>*>(_msg.get());
    Coordinate *coordinateData = msg->getData();
    console << "Received Coordinate Msg from: " << sender->getConnectedBlockId() 
        << " " << coordinateData->coordinatorPosition   << " " << coordinateData->position<< "\n";
    
    if(module->position == coordinateData->position) {
        console << "Can start moving\n";
        mvt_it = coordinateData->it; 
        coordinatorPosition = coordinateData->coordinatorPosition;
        operation = coordinateData->operation;
        bool bridgeStop = false;
        if(operation->isTransfer() or (operation->isDismantle() and static_cast<Dismantle_Operation*>(operation)->filled)) {
            //Special logic to avoid unsupported motions of bridging modules
            bridgeStop = static_cast<Transfer_Operation*>(operation)->handleBridgeMovements(this);
        } 
        if(bridgeStop) return;
        probeGreenLight();
    } else {        
        if(module->getInterface(nearestPositionTo(coordinateData->position, sender))->isConnected()) {
            sendMessage("Coordinate Msg1",
                     new MessageOf<Coordinate>(COORDINATE_MSG_ID, *coordinateData),
                     module->getInterface(nearestPositionTo(coordinateData->position, sender)), 100, 200);
        } else {
            sendMessage("Coordinate Msg2",
                     new MessageOf<Coordinate>(COORDINATE_MSG_ID, *coordinateData),
                     sender, 100, 200);
        }
    }
}

void MetaModuleBlockCode::handleCoordinateBackMessage(std::shared_ptr<Message> _msg,
                                               P2PNetworkInterface* sender) {
    MessageOf<CoordinateBack> *msg = static_cast<MessageOf<CoordinateBack>*>(_msg.get());
    CoordinateBack *coordinateBackData = msg->getData();
    console << "Received coordinateBack from: " << sender->getConnectedBlockId() << " " << coordinateBackData->coordinatorPosition << "\n";
    if(module->position == coordinateBackData->coordinatorPosition) {
        console << "mvt_it: " << mvt_it << "\n";
        console << "steps: " << coordinateBackData->steps << "\n";

        //Add the number of steps to get to the next module's movements
        for(int i=0; i<coordinateBackData->steps; i++) {
            mvt_it++;
        }
        
        if(mvt_it >= operation->localRules->size()) {
            // operation ended
            isCoordinator = false;
            console << "Movement Done\n";
            return;
        }
        if((*operation->localRules)[mvt_it].currentPosition + seedPosition == module->position) {
            // The coordinator must start its motions (In Dismantle operations)
            LocalMovement lmvt = (*operation->localRules)[mvt_it];
            probeGreenLight();
            return;
        } 
        console << mvt_it << ": Movement ended must switch to next one " 
                << (*operation->localRules)[mvt_it].currentPosition << "\n";
        Cell3DPosition targetModule = seedPosition + (*operation->localRules)[mvt_it].currentPosition;
        if (lattice->cellsAreAdjacent(module->position, targetModule) and
            not module->getInterface(targetModule)->isConnected()) {
            // Received coordinateBack and no module is connected
            // Must wait until the module arrives to the starting position
            awaitingCoordinator = true;
            getScheduler()->schedule(
                new InterruptionEvent(getScheduler()->now() +
                                      getRoundDuration(),
                                      module, IT_MODE_TRANSFERBACK));
            return;
        }
        sendMessage("Coordinate Msg", new MessageOf<Coordinate>(
                COORDINATE_MSG_ID, Coordinate(operation, targetModule, module->position, mvt_it)),
                module->getInterface(nearestPositionTo(targetModule)), 100, 200
        );

    } else { // Forward the message to the coordinator
        sendMessage("CoordinateBack Msg1", 
            new MessageOf<CoordinateBack>(COORDINATEBACK_MSG_ID, *coordinateBackData), 
            module->getInterface(nearestPositionTo(coordinateBackData->coordinatorPosition, sender)),
            100, 200);
    }                     
}

/************************************************************************
 ************************* MOTION COORDINATION **************************
 ***********************************************************************/

void MetaModuleBlockCode::handlePLSMessage(std::shared_ptr<Message> _msg,
                                               P2PNetworkInterface* sender) {
    MessageOf<PLS>* msg = static_cast<MessageOf<PLS>*>(_msg.get());
    Cell3DPosition srcPos = msg->getData()->srcPos;
    Cell3DPosition targetPos = msg->getData()->targetPos;
    console << "Received PLS from: " << sender->getConnectedBlockId() << srcPos <<',' << targetPos << "\n";
    if(movingState != MOVING) {
        bool nextToSender = isAdjacentToPosition(srcPos);
        bool nextToTarget = isAdjacentToPosition(targetPos);
        bool targetNextToSrc = false;
        if(module->getState() == BuildingBlock::State::ACTUATING) {
            getScheduler()->trace("light turned orange", module->blockId, ORANGE);
                moduleAwaitingGo = true;
                awaitingModulePos = srcPos;
                awaitingModuleProbeItf = sender;
                setGreenLight(false);
                module->setColor(DARKORANGE);
                return;
        }
        Catoms3DBlock* targetLightNeighbor = findTargetLightAmongNeighbors(targetPos, srcPos, sender);
        
        if(targetLightNeighbor) { //Check if targetLightNeighbor has already received this msg
            for(const auto cell: lattice->getActiveNeighborCells(targetLightNeighbor->position)) {
                if(module->getInterface(cell) == sender or cell == module->position) continue;
                if(lattice->cellsAreAdjacent(cell, srcPos)) {
                    targetNextToSrc = true;
                }
            }
            if(!targetNextToSrc) {
                if(lattice->cellsAreAdjacent(targetLightNeighbor->position, srcPos)) {
                    targetNextToSrc = true;
                }
            }
            console << "targetNextToSrc: " << targetNextToSrc << "\n";
        }
        if (targetLightNeighbor
            and targetLightNeighbor->position != srcPos and !targetNextToSrc) { // neighbor is target light
            console << "Neighbor is target light: " << targetLightNeighbor->position << "\n";
            console << "target pos: " << targetPos << "\n";
            P2PNetworkInterface* tlitf = module->getInterface(
                targetLightNeighbor->position);

            VS_ASSERT(tlitf and tlitf->isConnected());

            sendMessage("PLS Msg", new MessageOf<PLS>(PLS_MSG_ID, PLS(srcPos, targetPos)), tlitf,  100, 200);
        } else if((not targetLightNeighbor and nextToTarget) or targetNextToSrc) {
            bool mustAvoidBlocking = false;
            if (targetPos - seedPosition == Cell3DPosition(1, 0, 2) and
                lattice->cellHasBlock(seedPosition + Cell3DPosition(1, 1, 2))) {
                // Special test to avoid blocking when a BF meta-module is filling back
                MetaModuleBlockCode *x = static_cast<MetaModuleBlockCode *>(
                    BaseSimulator::getWorld()
                        ->getBlockByPosition(seedPosition + Cell3DPosition(1, 1, 2))
                        ->blockCode);
                if (x->movingState == MOVING) {
                    mustAvoidBlocking = true;
                }
            }
            if (greenLightIsOn
                or (nextToSender
                    and module->getState() != BuildingBlock::State::ACTUATING and not mustAvoidBlocking)) {
                
                P2PNetworkInterface* itf = nextToSender ?
                    module->getInterface(srcPos) : sender;
                VS_ASSERT(itf and itf->isConnected());
                sendMessage("GLO Msg", new MessageOf<PLS>(GLO_MSG_ID, PLS(targetPos, srcPos)),itf, 100, 0);
            } else {
                getScheduler()->trace("light turned orange", module->blockId, ORANGE);
                moduleAwaitingGo = true;
                awaitingModulePos = srcPos;
                awaitingModuleProbeItf = sender;
                module->setColor(DARKORANGE);
            }
        }else { // not neighborNextToTarget and not nextToSender
            module->setColor(BLACK);
            VS_ASSERT_MSG(false, "error: not neighborNextToTarget and not nextToSender");
        }
    } else {
        module->setColor(BLACK);
        VS_ASSERT_MSG(false, "Module is moving. Should not receive this message");
    }                                              
}

void MetaModuleBlockCode::handleGLOMessage(std::shared_ptr<Message> _msg,
                                               P2PNetworkInterface* sender) {
    MessageOf<PLS> *msg = static_cast<MessageOf<PLS>*>(_msg.get());
    Cell3DPosition srcPos = msg->getData()->srcPos;
    Cell3DPosition targetPos = msg->getData()->targetPos;
    console << "Received GLO from: " << sender->getConnectedBlockId() << "\n";
    if (!rotating) { // module is pivot
        bool nextToDest = isAdjacentToPosition(targetPos);
        P2PNetworkInterface* itf;
        Cell3DPosition nnCell = Cell3DPosition(0,0,0);
        if (not nextToDest) {
            for (const auto &nCell: lattice->getActiveNeighborCells(module->position)){
                if (lattice->cellsAreAdjacent(nCell, targetPos)) {
                    nnCell = nCell;
                    continue;
                }
            }
        }
        if (nextToDest) {
            itf = module->getInterface(targetPos);

        } else if (nnCell != Cell3DPosition(0,0,0)) {
            itf = module->getInterface(nnCell);
        } else {
            itf = module->getInterface(nearestPositionTo(targetPos));
        }

        VS_ASSERT(itf and itf->isConnected());

        setGreenLight(false);
       
        sendMessage("GLO Msg", new MessageOf<PLS>(GLO_MSG_ID, PLS(srcPos, targetPos)), itf, 100, 0);
    } else if(module->position == targetPos){
        VS_ASSERT(module->position == targetPos);
        Cell3DPosition targetPosition = (*operation->localRules)[mvt_it].nextPosition + seedPosition;
        if(module->canRotateToPosition(targetPosition)) {
            if ((relativePos() == Cell3DPosition(-1, 1, 2) /**or
                              /ùùrelativePos() == Cell3DPosition(-1, -1, 2)**/) or
                relativePos() == Cell3DPosition(1, 0, 1) or
                relativePos() == Cell3DPosition(1, -1, 1) or
                relativePos() == Cell3DPosition(2, 0, 2) or relativePos() == Cell3DPosition(1,2,2) or relativePos() == Cell3DPosition(1,0,2)) {
                    //if(relativePos() == Cell3DPosition(1,2,2)) VS_ASSERT(false);
                getScheduler()->schedule(
                    new Catoms3DRotationStartEvent(getScheduler()->now(), module, targetPosition,
                                                   RotationLinkType::OctaFace, false));
            } else {
                getScheduler()->schedule(new Catoms3DRotationStartEvent(getScheduler()->now(),
                                                     module, targetPosition,
                                                     RotationLinkType::Any, false));
            }

        } else {
            //retry rotating to next position
            probeGreenLight();
            //VS_ASSERT_MSG(false, "Can not rotate to next position");
        }
        
    }
}

void MetaModuleBlockCode::handleFTRMessage(std::shared_ptr<Message> _msg,
                                               P2PNetworkInterface* sender) {
    MessageOf<Cell3DPosition>* msg = static_cast<MessageOf<Cell3DPosition>*>(_msg.get());
    console << "Received FTR from " << sender->getConnectedBlockId() << "\n";
    Cell3DPosition finalPos = *msg->getData();
    VS_ASSERT(lattice->cellsAreAdjacent(module->position, finalPos));
    if (not greenLightIsOn) {
        console << "test\n";
        setGreenLight(true);
        // for (auto n : lattice->getActiveNeighborCells(module->position)) {
        //     if (n == finalPos) continue;
        //     if (lattice->cellsAreAdjacent(n, finalPos) and module->getInterface(n) != sender) {
        //         sendMessage("FTR message", new MessageOf<Cell3DPosition>(FTR_MSG_ID, finalPos),
        //                     module->getInterface(n), 100, 200);
        //     }
        // }
    }
}

void MetaModuleBlockCode::probeGreenLight() {
        VS_ASSERT(operation->localRules != NULL);
        LocalMovement lmvt = (*operation->localRules)[mvt_it];
        console << lmvt.nextPosition << "\n";
        console << "mvt_it: " << mvt_it << "\n";
        console << lmvt.nextPosition + seedPosition << "\n";
        rotating = true;

        if(lmvt.nextPosition == Cell3DPosition(2,0,2) and 
        module->getInterface(module->position.offsetX(1))->isConnected() and operation->getDirection() == Direction::UP) {
            (*operation->localRules)[mvt_it].nextPosition = Cell3DPosition(1,0,2);
        }
        Cell3DPosition targetPosition = lmvt.nextPosition + seedPosition;
        
        Catoms3DBlock *pivot = customFindMotionPivot(module, targetPosition);
        if (not pivot) {
            notFindingPivot = true;
            getScheduler()->schedule(
                new InterruptionEvent(getScheduler()->now() +
                                      getRoundDuration(),
                                      module, IT_MODE_FINDING_PIVOT));
            stringstream info;
            info << " reattempt finding pivot for " << targetPosition;
            scheduler->trace(info.str(),module->blockId,PINK);
            return;
        }
        notFindingPivot = false;
        //VS_ASSERT(pivot); 
        console << "pivot: " << pivot->position << "\n";
        if(module->getInterface(pivot->position)->isConnected())
        sendMessage("PLS Msg", new MessageOf<PLS>(PLS_MSG_ID, PLS(module->position, targetPosition)),
            module->getInterface(pivot->position), 100, 200);
}

bool MetaModuleBlockCode::isAdjacentToPosition(const Cell3DPosition& pos) const {
    return lattice->cellsAreAdjacent(module->position, pos);
}

Catoms3DBlock* MetaModuleBlockCode::customFindMotionPivot(const Catoms3DBlock* m,
                                                            const Cell3DPosition& tPos,
                                                            RotationLinkType faceReq) {
    const auto &allLinkPairs =
        Catoms3DMotionEngine::findPivotLinkPairsForTargetCell(m, tPos, faceReq);

    for (const auto& pair : allLinkPairs) {
        // Additional rule compared to Catoms3DMotionEngine::customFindMotionPivot:
        //  Make sure that pivot is not mobile
        if (static_cast<MetaModuleBlockCode *>(pair.first->blockCode)->movingState == MOVING or
            pair.first->getState() == BuildingBlock::State::MOVING)
            continue;

        if (pair.second->getMRLT() == faceReq or faceReq == RotationLinkType::Any)
            return pair.first;
    }
    return NULL;
}

Catoms3DBlock* MetaModuleBlockCode::findTargetLightAmongNeighbors(const Cell3DPosition& targetPos,
                                                                    const Cell3DPosition& srcPos,
                                                                    P2PNetworkInterface *sender) const {
    Cell3DPosition except;
    if(sender) {
        module->getNeighborPos(module->getInterfaceId(sender), except);
    }
    for (const auto& cell : lattice->getActiveNeighborCells(module->position)) {
        if (lattice->cellsAreAdjacent(cell, targetPos) and cell != srcPos){
            if(sender and cell == except)
                continue;
            MetaModuleBlockCode *block = static_cast<MetaModuleBlockCode *>(
                BaseSimulator::getWorld()->getBlockByPosition(cell)->blockCode);
            if (block->rotating) continue;

            return static_cast<Catoms3DBlock*>(lattice->getBlock(cell));
        }
            
    }
    return NULL;
}

bool MetaModuleBlockCode::setGreenLight(bool onoff) {
    stringstream info;
    info << " light turned ";

    if (onoff) {
        info << "green: ";
        greenLightIsOn = true;
        module->setColor(initialColor);

        // Resume flow if needed
        if (moduleAwaitingGo) {
            bool nextToModule = isAdjacentToPosition(awaitingModulePos);

            P2PNetworkInterface *itf = nextToModule ? module->getInterface(awaitingModulePos) :
                                                    // Move the message up the branch
                                           awaitingModuleProbeItf;

            VS_ASSERT(itf and itf->isConnected());
            sendMessage("GLO Msg",
                        new MessageOf<PLS>(GLO_MSG_ID, PLS(module->position, awaitingModulePos)),
                        itf, 100, 0);
            moduleAwaitingGo = false;
            awaitingModuleProbeItf = NULL;
        }
    } else {
        info << "red: ";
        greenLightIsOn = false;
        module->setColor(RED);
    }

    getScheduler()->trace(info.str(),module->blockId, onoff ? GREEN : RED);
}



void MetaModuleBlockCode::updateState() {
    console << "Update State!!\n";
    operation->updateState(this);
    if(not operation->isTransfer()) {
        operation =new Operation();
    }
   
    mvt_it = 0;
    isCoordinator = false;
    module->setColor(initialColor);
    if(not greenLightIsOn) {
        setGreenLight(true);
    }
}

bool MetaModuleBlockCode::isInMM(Cell3DPosition &neighborPosition) {
    Cell3DPosition position = (neighborPosition - seedPosition);
    if(shapeState == FRONTBACK) {
        bool inBorder = (find(FrontBackMM.begin(), FrontBackMM.end(), position) != FrontBackMM.end());
        bool inFill = 
            (find(FillingPositions_FrontBack_Zeven.begin(), FillingPositions_FrontBack_Zeven.end(), position) != FillingPositions_FrontBack_Zeven.end());
        if(inBorder)
            return true;
    }else if(shapeState == BACKFRONT) {
        bool inBorder = (find(BackFrontMM.begin(), BackFrontMM.end(), position) != BackFrontMM.end());
        bool inFill = 
            (find(FillingPositions_BackFront_Zeven.begin(), FillingPositions_BackFront_Zeven.end(), position) != FillingPositions_BackFront_Zeven.end());
        if(inBorder )
            return true;
    } else {
        
    }
    return false;
}

Cell3DPosition MetaModuleBlockCode::nearestPositionTo(Cell3DPosition& targetPosition,
                                                      P2PNetworkInterface* except) {
    int distance = INT32_MAX;
    Cell3DPosition nearest;
    for (auto neighPos : lattice->getActiveNeighborCells(module->position)) {
    MetaModuleBlockCode* neighBlock = static_cast<MetaModuleBlockCode*>(
        BaseSimulator::getWorld()->getBlockByPosition(neighPos)->blockCode);
        if(neighBlock->module->getState() ==BuildingBlock::State::MOVING ) continue;
        if (except != nullptr) {
            if (module->getInterface(neighPos) == except) {
                continue;
            }       
        }
        VS_ASSERT(neighBlock->module->getState() != BuildingBlock::State::MOVING);
        int d = BaseSimulator::getWorld()->lattice->getCellDistance(neighPos, targetPosition);
        //int d = neighPos.dist_euclid(targetPosition); 
        if (d < distance ) {
            distance = d;
            nearest = neighPos;
        }
    }
    if (distance == INT32_MAX) {
        module->getNeighborPos(module->getInterfaceId(except), nearest);
        return nearest;
    }
    VS_ASSERT(distance != DBL_MAX);
    return nearest;
}

Cell3DPosition MetaModuleBlockCode::nextInBorder(P2PNetworkInterface* sender = nullptr) {
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

P2PNetworkInterface * MetaModuleBlockCode::interfaceTo(Cell3DPosition& dstPos, P2PNetworkInterface *sender) {
    if(sender)
        return module->getInterface(nearestPositionTo(dstPos, sender));
    else 
        return module->getInterface(nearestPositionTo(dstPos));
}

void MetaModuleBlockCode::onMotionEnd() {
    console << " has reached its destination" << "\n";

    LocalMovement lmvt = (*operation->localRules)[mvt_it];
    console << lmvt.nextPosition << "\n";
    movingState = lmvt.state;
    movingSteps++;
    
  
    if(movingState == MOVING) {        
        mvt_it++;
        probeGreenLight();
    } else if(movingState == WAITING or movingState == IN_POSITION) {
          
        transferCount = 0;
        rotating = false;
        if (operation->mustSendCoordinateBack(this)) {
            sendMessage("CoordinateBack Msg", 
                new MessageOf<CoordinateBack>(COORDINATEBACK_MSG_ID, CoordinateBack(movingSteps, coordinatorPosition)),
                module->getInterface(nearestPositionTo(coordinatorPosition)) ,100, 200);
        }
        console << "coordinator position: " << coordinatorPosition << "\n";
        movingSteps = 0;
        P2PNetworkInterface* pivotItf = module->getInterface(pivotPosition);
        if(operation->isFill()) {
            sendMessageToAllNeighbors("FTR msg", new MessageOf<Cell3DPosition>(FTR_MSG_ID, module->position),
                100, 200, 0);
        }else
         if(pivotItf and pivotItf->isConnected()) {
            sendMessage("FTR msg", new MessageOf<Cell3DPosition>(FTR_MSG_ID, module->position),
                    module->getInterface(pivotPosition), 100, 200);
        }
        if(movingState == IN_POSITION) {
            console << "mvt_it in pos: " << mvt_it << "\n";
            if(mvt_it == operation->localRules->size()-1) {
                cout << module->blockId << ": Op done!!\n";
            }
            updateState();
        }
    }
}



void MetaModuleBlockCode::processLocalEvent(EventPtr pev) {
    std::shared_ptr<Message> message;
    stringstream info;
    
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
            
            //VS_ASSERT(operation); 

            uint64_t face = Catoms3DWorld::getWorld()->lattice->
                getOppositeDirection((std::static_pointer_cast<AddNeighborEvent>(pev))
                                    ->face);
            if(module->getNeighborBlock(face) == NULL) return;
            Cell3DPosition& pos = module->getNeighborBlock(face)->position;
            
            MetaModuleBlockCode *posBlock = static_cast<MetaModuleBlockCode*>(
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
            if(isCoordinator and pos == module->position + Cell3DPosition(0,1,1)) {
                if ((not operation->isTransfer() and not operation->isFill() and operation->getDirection() != Direction::UP) or
                    (operation->isTransfer() and operation->getDirection() == Direction::UP)) {
                    console << "move pos\n";
                    if(posBlock->module->canMoveTo(module->position.offsetY(1))) {
                        posBlock->module->moveTo(module->position.offsetY(1));
                    } else {
                        // Wait until it can move to the desired position
                        getScheduler()->schedule(
                        new InterruptionEvent(getScheduler()->now() +
                                        getRoundDuration(),
                                        posBlock->module, IT_MODE_TRANSFERBACK_REACHCOORDINATOR));
                    }
                }
            }

            break;
        }

        case EVENT_REMOVE_NEIGHBOR: {
            // Do something when a neighbor is removed from an interface of the module
            if(not rotating and movingState != MOVING) {
                 uint64_t face = Catoms3DWorld::getWorld()->
                    lattice->getOppositeDirection((std::static_pointer_cast<RemoveNeighborEvent>(pev))->face);

                    Cell3DPosition pos;
                    if (module->getNeighborPos(face, pos) and (module->getState() <= 3)) {
                         //console << "REMOVE NEIGHBOR: " << pos << "\n";
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
                    if (!rotating) return;
                    // VS_ASSERT(++notFindingPivotCount < 10);
                    VS_ASSERT(operation->localRules.get());
                    probeGreenLight();  // the seed starts the algorithm
                    module->setColor(MAGENTA);
                } break;

                case IT_MODE_TRANSFERBACK: {
                    if(not awaitingCoordinator) return;
                    Cell3DPosition targetModule =
                        seedPosition + (*operation->localRules)[mvt_it].currentPosition;
                    if (not module->getInterface(targetModule)->isConnected()) {
                        getScheduler()->schedule(
                            new InterruptionEvent(getScheduler()->now() + getRoundDuration(),
                                                  module, IT_MODE_TRANSFERBACK));
                    } else {
                        sendMessage(
                            "Coordinate Msg",
                            new MessageOf<Coordinate>(
                                COORDINATE_MSG_ID,
                                Coordinate(operation, targetModule, module->position, mvt_it)),
                            module->getInterface(targetModule), 100, 200);
                        awaitingCoordinator = false;
                    }
                } break;

                case IT_MODE_TRANSFERBACK_REACHCOORDINATOR: {
                    if (module->canMoveTo(module->position.offsetZ(-1))) {
                        module->moveTo(module->position.offsetZ(-1));
                    } else {
                        getScheduler()->schedule(
                            new InterruptionEvent(getScheduler()->now() + getRoundDuration(),
                                                  module, IT_MODE_TRANSFERBACK_REACHCOORDINATOR));
                    }
                } break;
            }
        }
    }
}

/// ADVANCED BLOCKCODE FUNCTIONS BELOW

void MetaModuleBlockCode::onBlockSelected() {
    // Debug stuff:
    cerr << endl << "--- PRINT MODULE " << *module << "---" << endl;
    cerr << "isSeed: " << (seedPosition == module->position ? "true": "false") << endl;
    cerr << "isCoordinator: " << (isCoordinator ? "true": "false") << endl;
    cerr << "coordinatorPosition: " << coordinatorPosition << "\n" << endl;
    cerr << "seedPosition: " << seedPosition << endl;
    cerr << "MMPostion: " << MMPosition << endl;
    cerr << "CurrentPos: " << module->position - seedPosition << endl;
    cerr << "mvt_it: " << mvt_it << endl;
    cerr << "MovingState: " << movingState << endl;
    cerr << "GreenLight: " << greenLightIsOn << endl;
    //cerr << BaseSimulator::getWorld()->lattice->getCellDistance(seed->position, module->position) << endl;
}

void MetaModuleBlockCode::onUserKeyPressed(unsigned char c, int x, int y) {
    if(!BaseSimulator::getWorld()->selectedGlBlock) {
        cerr << "No block selected!" << endl;
        return;
    }
    int blockId = BaseSimulator::getWorld()->selectedGlBlock->blockId;
    MetaModuleBlockCode* block = static_cast<MetaModuleBlockCode*>(
        BaseSimulator::getWorld()->getBlockById(blockId)->blockCode
    );

    ofstream file;
    file.open("BF_Fill_Back_Zeven_ComingFromBack.txt", ios::out | ios::app);
    seedPosition = Cell3DPosition(12,23,10);
    if(!file.is_open()) return;

    if(c == 'o') {
        file << "{LocalMovement(" << "Cell3DPosition" << block->module->position - block->seedPosition;
    }
    if(c == 'p') {
        file << ", Cell3DPosition" << block->module->position - block->seedPosition
                << ", MOVING)},\n";
    }
}

void MetaModuleBlockCode::onAssertTriggered() {
    console << " has triggered an assert" << "\n";

    // Print debugging some info if needed below
    // ...
}

bool MetaModuleBlockCode::parseUserCommandLineArgument(int &argc, char **argv[]) {
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

void MetaModuleBlockCode::parseUserElements(TiXmlDocument *config) {
    TiXmlElement *worldElement = 
        config->RootElement();
    TiXmlElement *MMblocksElement = worldElement->FirstChildElement()->NextSiblingElement();
    TiXmlElement *blockElement = MMblocksElement->FirstChildElement();
    cerr << "Parsing MM positions\n";
    for(; blockElement != NULL; blockElement = blockElement->NextSiblingElement()) {
       
        string pos = blockElement->Attribute("position");
       
        int start = 0;
        int end = pos.find(",");
        int i = 0;
        array<int,4> coord = {0};
        while(end != -1) {
            
            coord[i] = stoi(pos.substr(start, end-start));
            //cerr << i << ": " << coord[i] << endl;
            i++;
            
            start = end + 1;
            end = pos.find(",", start);
            
        }
        coord[i] = stoi(pos.substr(start, end-start));
        

        const char* filled = blockElement->Attribute("filled");
        //string filled =  blockElement->Attribute("filled");
        if(filled) {
            if(filled == string("true")) {
                coord[3] = 1;
            }
            
       }
        initialMap.push_back(coord);
    }

    
}

string MetaModuleBlockCode::onInterfaceDraw() {
    stringstream trace;
    trace << "Nb of modules " << BaseSimulator::getWorld()->getNbBlocks();
    return trace.str();
}

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
    addMessageEventFunc2(SETCOORDINATOR_MSG_ID,
                         std::bind(&MetaModuleBlockCode::handleSetCoordinatorMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
                                   
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
        //Start movement
        //  MetaModuleBlockCode *block19 = static_cast<MetaModuleBlockCode*>(
        //     BaseSimulator::getWorld()->getBlockById(19)->blockCode
        // );
        //  block19->operation->nextOperation = BF_Transfer_Left;
        // //operation = new Operation(Direction::LEFT, BACKFRONT);
        // block19->operation = new Dismantle_Operation(Direction::LEFT, BACKFRONT);
        // block19->setCoordinator(BF_Dismantle_Left);
        // targetModule = block19->seedPosition + (*block19->operation->localRules)[0].currentPosition;
        // block19->console << "targetModule: " << block19->nearestPositionTo(targetModule) << "\n"; 
        // block19->sendMessage("Coordinate Msg1", new MessageOf<Coordinate>(
        //     COORDINATE_MSG_ID, Coordinate(block19->operation, targetModule, block19->module->position, block19->mvt_it)),
        //     block19->module->getInterface(block19->nearestPositionTo(targetModule)), 100, 200
        // );

        MetaModuleBlockCode *block49 = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockById(49)->blockCode
        );
         block49->operation->nextOperation = FB_Transfer_Left;
        //operation = new Operation(Direction::LEFT, BACKFRONT);
        block49->operation = new Dismantle_Operation(Direction::LEFT, FRONTBACK);
        block49->setCoordinator(FB_Dismantle_Left);
        
        targetModule = block49->seedPosition + (*block49->operation->localRules)[0].currentPosition;
        block49->console << "targetModule: " << block49->nearestPositionTo(targetModule) << "\n"; 
        block49->sendMessage("Coordinate Msg1", new MessageOf<Coordinate>(
            COORDINATE_MSG_ID, Coordinate(block49->operation, targetModule, block49->module->position, block49->mvt_it)),
            block49->module->getInterface(block49->nearestPositionTo(targetModule)), 100, 200
        );

        MetaModuleBlockCode *block13 = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockById(13)->blockCode
        );

        block13->operation->nextOperation = BF_Transfer_Left;
        block13->operation = new Transfer_Operation(Direction::LEFT, BACKFRONT, true);
        block13->isCoordinator = true;

        MetaModuleBlockCode *block3 = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockById(3)->blockCode
        );
        block3->operation->nextOperation = BF_Transfer_Left;
        block3->operation = new Transfer_Operation(Direction::LEFT, FRONTBACK);
        block3->isCoordinator = true;
        

        MetaModuleBlockCode *block23 = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockById(23)->blockCode
        );
        block23->operation = new Transfer_Operation(Direction::LEFT, BACKFRONT);
        block23->operation->nextOperation = FB_Build_Up;
        block23->isCoordinator = true;

        MetaModuleBlockCode *block33 = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockById(33)->blockCode
        );
        block33->operation->nextOperation = NO_OPERATION;
        block33->operation = new Build_Operation(Direction::UP, FRONTBACK);
        block33->isCoordinator = true;

        // Y = 1
        MetaModuleBlockCode *block99 = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockById(99)->blockCode
        );
         block99->operation->nextOperation = FB_Transfer_Left;
        //operation = new Operation(Direction::LEFT, BACKFRONT);
        block99->operation = new Dismantle_Operation(Direction::LEFT, BACKFRONT);
        block99->setCoordinator(BF_Dismantle_Left);
        targetModule = block99->seedPosition + (*block99->operation->localRules)[0].currentPosition;
        block99->console << "targetModule: " << block99->nearestPositionTo(targetModule) << "\n"; 
        block99->sendMessage("Coordinate Msg1", new MessageOf<Coordinate>(
            COORDINATE_MSG_ID, Coordinate(block99->operation, targetModule, block99->module->position, block99->mvt_it)),
            block99->module->getInterface(block99->nearestPositionTo(targetModule)), 100, 200
        );

        MetaModuleBlockCode *block63 = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockById(63)->blockCode
        );

        block63->operation->nextOperation = BF_Transfer_Left;
        block63->operation = new Transfer_Operation(Direction::LEFT, FRONTBACK, true);
        block63->isCoordinator = true;

        MetaModuleBlockCode *block53 = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockById(53)->blockCode
        );
        block53->operation->nextOperation = BF_Transfer_Left;
        block53->operation = new Transfer_Operation(Direction::LEFT, BACKFRONT);
        block53->isCoordinator = true;
        

        MetaModuleBlockCode *block73 = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockById(73)->blockCode
        );
        block73->operation = new Transfer_Operation(Direction::LEFT, FRONTBACK);
        block73->operation->nextOperation = FB_Build_Up;
        block73->isCoordinator = true;

        MetaModuleBlockCode *block83 = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockById(83)->blockCode
        );
        block83->operation->nextOperation = NO_OPERATION;
        block83->operation = new Build_Operation(Direction::UP, BACKFRONT);
        block83->isCoordinator = true;


        Init::initialMapBuildDone = true;
    }
    initialized = true;
}

void MetaModuleBlockCode::handleSetCoordinatorMessage(std::shared_ptr<Message> _msg,
                                               P2PNetworkInterface* sender) {
    MessageOf<SetCoordinator>* msg = static_cast<MessageOf<SetCoordinator>*>(_msg.get());
    SetCoordinator *data = msg->getData();
    operation = &data->operation;
    if(isCoordinator) {
        VS_ASSERT(isCoordinator);
        return;
    }
    if(data->coordinatorPosition == module->position) {
        console << "Coordinator for Next Movement\n ";
        //setCoordinator(operation);
        
    } else {
        sendMessage("SetCoordinator Msg", new MessageOf<SetCoordinator>( SETCOORDINATOR_MSG_ID, *data),
                    module->getInterface(nearestPositionTo( data->coordinatorPosition, sender)),
                    100, 200);
    }
    
    
}

void MetaModuleBlockCode::handleCoordinateMessage(std::shared_ptr<Message> _msg,
                                               P2PNetworkInterface* sender) {                                 
    MessageOf<Coordinate>* msg = static_cast<MessageOf<Coordinate>*>(_msg.get());
    Coordinate *coordinateData = msg->getData();
    console << "Received Coordinate Msg from: " << sender->getConnectedBlockId() 
        << " " << coordinateData->coordinatorPosition  << "\n";
    
    if(module->position  == coordinateData->position) {
        console << "Can start moving\n";
        mvt_it = coordinateData->it; 
        coordinatorPosition = coordinateData->coordinatorPosition;
        operation = coordinateData->operation;
        bool bridgeStop = false;
        if(operation->isTransfer()) {
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
        for(int i=0; i<coordinateBackData->steps; i++) {
            mvt_it++;
        }
        if(mvt_it == operation->localRules->size()) {
            isCoordinator = false;
            localRules = nullptr;
            console << "Movement Done\n";
            return;
        }
        if((*operation->localRules)[mvt_it].currentPosition + seedPosition == module->position) {
            LocalMovement lmvt = (*operation->localRules)[mvt_it];
            probeGreenLight();
            // console << lmvt.nextPosition << "\n";
            // movingState = lmvt.state;
            // console << lmvt.nextPosition + seedPosition << "\n";
            // movingSteps++;
            // module->moveTo(seedPosition + lmvt.nextPosition);
            return;
        } 
        console << mvt_it << ": Movement ended must switch to next one " 
                << (*operation->localRules)[mvt_it].currentPosition << "\n";
        Cell3DPosition targetModule = seedPosition + (*operation->localRules)[mvt_it].currentPosition;
        sendMessage("Coordinate Msg", new MessageOf<Coordinate>(
                COORDINATE_MSG_ID, Coordinate(operation, targetModule, module->position, mvt_it)),
                module->getInterface(nearestPositionTo(targetModule)), 100, 200
        );
        //  sendMessage("Coordinate Msg", new MessageOf<Coordinate>(
        //         COORDINATE_MSG_ID, Coordinate(operation, targetModule, module->position, mvt_it)),
        //        interfaceTo(targetModule), 100, 200
        //);
    } else {
        // if(isCoordinator and operation == FB_Fill_Left) return;
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
        }
        console << "targetNextToSrc: " << targetNextToSrc << "\n";
        if (targetLightNeighbor
            and targetLightNeighbor->position != srcPos and !targetNextToSrc) { // neighbor is target light
            console << "Neighbor is target light: " << targetLightNeighbor->position << "\n";
            console << "target light pos: " << targetLightNeighbor->position << "\n";
            P2PNetworkInterface* tlitf = module->getInterface(
                targetLightNeighbor->position);

            VS_ASSERT(tlitf and tlitf->isConnected());

            sendMessage("PLS Msg", new MessageOf<PLS>(PLS_MSG_ID, PLS(srcPos, targetPos)), tlitf,  100, 200);
        } else if((not targetLightNeighbor and nextToTarget) or targetNextToSrc) {
            if (greenLightIsOn
                or (nextToSender
                    and module->getState() != BuildingBlock::State::ACTUATING)) {
                
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
        //rotating = true;
        // LocalMovement lmvt = (*localRules)[mvt_it];
        // console << lmvt.nextPosition << "\n";
        // movingState = lmvt.state;
        // movingSteps++;
        // Sender should be pivot to be used for next motion
        //Catoms3DBlock* pivot = static_cast<Catoms3DBlock*>(msg->sourceInterface->hostBlock);
        //VS_ASSERT(pivot and pivot !=module);
        //scheduleRotationTo(mabc.stepTargetPos, pivot);
        //operation->localRules.reset(&LocalRules_BF_Dismantle_Left);
        //cout << (*operation->localRules.get())[0] << endl;
        Cell3DPosition targetPosition = (*operation->localRules)[mvt_it].nextPosition + seedPosition;
        module->moveTo(targetPosition);
        // scheduler->schedule(new Catoms3DRotationStartEvent(getScheduler()->now(),
        //                                              module, targetPos,
        //                                              RotationLinkType::OctaFace, false));
    }
}

void MetaModuleBlockCode::handleFTRMessage(std::shared_ptr<Message> _msg,
                                               P2PNetworkInterface* sender) {
    MessageOf<Cell3DPosition>* msg = static_cast<MessageOf<Cell3DPosition>*>(_msg.get());
    Cell3DPosition finalPos = *msg->getData();
    VS_ASSERT(lattice->cellsAreAdjacent(module->position, finalPos));
    if (not greenLightIsOn) {
        setGreenLight(true);
    }
}

void MetaModuleBlockCode::probeGreenLight() {
        VS_ASSERT(operation->localRules != NULL);
        LocalMovement lmvt = (*operation->localRules)[mvt_it];
        console << lmvt.nextPosition << "\n";
        console << "mvt_it: " << mvt_it << "\n";
        // movingState = lmvt.state;
        console << lmvt.nextPosition + seedPosition << "\n";
        // movingSteps++;
        rotating = true;
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
        sendMessage("PLS Msg", new MessageOf<PLS>(PLS_MSG_ID, PLS(module->position, targetPosition)),
            module->getInterface(pivot->position), 100, 200);
        // module->moveTo(seedPosition + lmvt.nextPosition);
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
        if (static_cast<MetaModuleBlockCode*>(pair.first->blockCode)->movingState == MOVING)
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
            MetaModuleBlockCode* block = static_cast<MetaModuleBlockCode*>(BaseSimulator::getWorld()->getBlockByPosition(cell)->blockCode);
            if(block->rotating) continue;
  
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
        module->setColor(GREEN);

        // Resume flow if needed
        if (moduleAwaitingGo) {
            bool nextToModule = isAdjacentToPosition(awaitingModulePos);

            P2PNetworkInterface* itf = nextToModule ?
                module->getInterface(awaitingModulePos) :
                // Move the message up the branch
                awaitingModuleProbeItf;

            VS_ASSERT(itf and itf->isConnected());
            // sendMessage(new GreenLightIsOnMessage(catom->position, awaitingModulePos),
            //             itf, MSG_DELAY_MC, 0);
            sendMessage("GLO Msg", new MessageOf<PLS>(GLO_MSG_ID, PLS(module->position, awaitingModulePos)), itf, 100, 0);
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
    currentMovement = NO_MOVEMENT;
    if(not operation->isTransfer()) {
        operation =new Operation();
        operation->nextOperation = NO_OPERATION;
    }
   
    mvt_it = 0;
    isCoordinator = false;
    localRules = nullptr;
    if(not greenLightIsOn) {
        setGreenLight(true);
    }
}

bool MetaModuleBlockCode::isInMM(Cell3DPosition &neighborPosition) {
    Cell3DPosition position = (neighborPosition - seedPosition);
    // bool isInOpened = (find(OpenedPositions.begin(), OpenedPositions.end(), position) != OpenedPositions.end());
    // if(isInOpened)
    //     return true;
    if(shapeState == FRONTBACK) {
        bool inBorder = (find(FrontBackMM.begin(), FrontBackMM.end(), position) != FrontBackMM.end());
        bool inFill = 
            (find(FillingPositions_FrontBack.begin(), FillingPositions_FrontBack.end(), position) != FillingPositions_FrontBack.end());
        if(inBorder)
            return true;
    }else if(shapeState == BACKFRONT) {
        bool inBorder = (find(BackFrontMM.begin(), BackFrontMM.end(), position) != BackFrontMM.end());
        bool inFill = 
            (find(FillingPositions_BackFront.begin(), FillingPositions_BackFront.end(), position) != FillingPositions_BackFront.end());
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
    // MetaModuleBlockCode* dstBlock = static_cast<MetaModuleBlockCode*>(
    //     BaseSimulator::getWorld()->getBlockByPosition(targetPosition)->blockCode);
    for (auto neighPos : lattice->getActiveNeighborCells(module->position)) {
    MetaModuleBlockCode* neighBlock = static_cast<MetaModuleBlockCode*>(
        BaseSimulator::getWorld()->getBlockByPosition(neighPos)->blockCode);
        if (except != nullptr) {
            if (module->getInterface(neighPos) == except) {
                console << neighPos << "\n";
                continue;
            }
                
        }
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

 void MetaModuleBlockCode::setCoordinator(MMOperation op) {
    isCoordinator = true;
    // mvt_it = 0;
    // operation = op;
    // switch (operation) {
    // case BF_Dismantle_Left: {
    //     console << "Start FB_Dismantle_Left operation\n";
    //     //Init::getNeighborMMSeedPos(seedPosition, MMPosition,  NeighborPos::LEFT, targetSeed);
    //     localRules = &LocalRules_BF_Dismantle_Left;
    //     Cell3DPosition targetModule = seedPosition + (*localRules)[0].currentPosition;
    //     sendMessage("Coordinate Msg", new MessageOf<Coordinate>(
    //         COORDINATE_MSG_ID, Coordinate(operation, operation->nextOperation,  targetModule, module->position, mvt_it)),
    //         module->getInterface(nearestPositionTo(targetModule)), 100, 200
    //     ); 
    //     break;
    // }
    // default:
    //     break;
    // }
 }



void MetaModuleBlockCode::onMotionEnd() {
    console << " has reached its destination" << "\n";
    console << "op:" << operation->op << "\n";
    LocalMovement lmvt = (*operation->localRules)[mvt_it];
    console << lmvt.nextPosition << "\n";
    movingState = lmvt.state;
    movingSteps++;
  
    if(movingState == MOVING) {        
        mvt_it++;
        probeGreenLight();
    } else if(movingState == WAITING or movingState == IN_POSITION) {
          
        transferCount = 0;
        //setGreenLight(true);
        rotating = false;
        if((!isCoordinator and operation->isDismantle()) 
            or (operation->isTransfer() 
                and (operation->getDirection() == Direction::LEFT or operation->getDirection() == Direction::RIGHT)
                and mvt_it >= 14
                )
            // or (operation->isBuild()
            //     and (operation->getDirection() == Direction::UP)
            //     and mvt_it >= 51
            //     )
        ) {
            sendMessage("CoordinateBack Msg", 
                new MessageOf<CoordinateBack>(COORDINATEBACK_MSG_ID, CoordinateBack(movingSteps, coordinatorPosition)),
                module->getInterface(nearestPositionTo(coordinatorPosition)) ,100, 200);
        } 
        console << "coordinator position: " << coordinatorPosition << "\n";
        movingSteps = 0;
        P2PNetworkInterface* pivotItf = module->getInterface(pivotPosition);
        VS_ASSERT(pivotItf and pivotItf->isConnected());
        //sendMessageToAllNeighbors("FTR msg",new MessageOf<Cell3DPosition>(FTR_MSG_ID, module->position), 100, 0, 0);
        sendMessage("FTR msg", new MessageOf<Cell3DPosition>(FTR_MSG_ID, module->position),
                    module->getInterface(pivotPosition), 100, 200);

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
            
            VS_ASSERT(operation); 
            uint64_t face = Catoms3DWorld::getWorld()->lattice->
                getOppositeDirection((std::static_pointer_cast<AddNeighborEvent>(pev))
                                    ->face);
            if(module->getNeighborBlock(face) == NULL) return;
            Cell3DPosition& pos = module->getNeighborBlock(face)->position;
            
            MetaModuleBlockCode *posBlock = static_cast<MetaModuleBlockCode*>(
                BaseSimulator::getWorld()->getBlockByPosition(pos)->blockCode
            );
            console << "ADD NEIGHBOR: " << pos << "\n";
            if(not rotating and posBlock->rotating or (!posBlock->rotating and isCoordinator) and module->getState()) {
                setGreenLight(false);
            }
            operation->handleAddNeighborEvent(this, pos);             
            break;
        }

        case EVENT_REMOVE_NEIGHBOR: {
            // Do something when a neighbor is removed from an interface of the module
            if(not rotating and movingState != MOVING) {
                 uint64_t face = Catoms3DWorld::getWorld()->
                    lattice->getOppositeDirection((std::static_pointer_cast<RemoveNeighborEvent>(pev))->face);

                    Cell3DPosition pos;
                    if (module->getNeighborPos(face, pos) and (module->getState() == BuildingBlock::State::ALIVE)) {
                         console << "REMOVE NEIGHBOR: " << pos << "\n";
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
                if (module->getNeighborPos(face, pos) and
                    (module->getState() == BuildingBlock::State::ALIVE)) {
                    console << "REMOVE NEIGHBOR: " << pos << "\n";
                    setGreenLight(true);
                }  
            }
            

            break;
        }
        case EVENT_INTERRUPTION: {
            std::shared_ptr<InterruptionEvent> itev =
                std::static_pointer_cast<InterruptionEvent>(pev);

            switch(itev->mode) {
                case IT_MODE_FINDING_PIVOT:
                    if(!rotating) return;
                    // VS_ASSERT(++notFindingPivotCount < 10);
                    VS_ASSERT(operation->localRules.get());
                    probeGreenLight(); // the seed starts the algorithm
                    module->setColor(MAGENTA);
                    break;
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
    // cerr << "possibleMoves: ";
    // for(int i=0; i<module->getNbInterfaces(); i++) {
    //     Cell3DPosition p;
    //     if(module->getNeighborPos(i, p) and module->canMoveTo(p)) {
    //         cerr << p << "; ";
    //     }
    // }
    // cerr << endl;
    // cerr << movingState << endl;
    cerr << "CurrentPos: " << module->position - seedPosition << endl;
    cerr << "mvt_it: " << mvt_it << endl;
    cerr << "operation: " << operation->op << endl;
    cerr << "MovingState: " << movingState << endl;
    cerr << BaseSimulator::getWorld()->lattice->getCellDistance(seed->position, module->position) << endl;
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
    file.open("BF_build_up.txt", ios::out | ios::app);
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
    trace << "Some value " << 123;
    return trace.str();
}

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
    if(module->blockId == 1) {
      
        initialized = false;
        MMPosition = Cell3DPosition(0,0,0);
        shapeState = FRONTBACK;
        seed = module;
        cout << seed->position << endl;
        Init::buildMM(module, FRONTBACK, Color(GREEN));
        if(initialMap[0][3] == 1) {
            Init::fillMM(seed);
        }
        Init::buildInitialMap(seed->position);
        seedPosition = module->position;
        //Start movement
        // currentMovement = BFtoFB_RtoL_EtoE;
        // Cell3DPosition coordinatorPos = seedPosition + Cell3DPosition(2,1,2);
        //     sendMessage("SetCoordinator Msg", new MessageOf<SetCoordinator>( SETCOORDINATOR_MSG_ID,
        //         SetCoordinator(coordinatorPos, currentMovement)),
        //         module->getInterface(nearestPositionTo(coordinatorPos)),
        //         100, 200);

        MetaModuleBlockCode *block19 = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockById(19)->blockCode
        );
       // block19->operation = BF_Dismantle_Left;
        block19->nextOperation = FB_Transfer_Left;
        //operation = new Operation(Direction::LEFT, BACKFRONT);
        block19->operation = new Dismantle_Operation(Direction::LEFT, BACKFRONT);
        block19->setCoordinator(BF_Dismantle_Left);
        Cell3DPosition targetModule = block19->seedPosition + (*block19->operation->localRules)[0].currentPosition;
        console << "targetModule: " << nearestPositionTo(targetModule) << "\n"; 
        block19->sendMessage("Coordinate Msg1", new MessageOf<Coordinate>(
            COORDINATE_MSG_ID, Coordinate(block19->operation, targetModule, block19->module->position, block19->mvt_it)),
            block19->module->getInterface(block19->nearestPositionTo(targetModule)), 100, 200
        );

        MetaModuleBlockCode *block4 = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockById(4)->blockCode
        );
        //block4->operation = FB_Transfer_Left;
        block4->nextOperation = BF_Transfer_Left;
        block4->operation = new Transfer_Operation(Direction::LEFT, FRONTBACK);
        //block4->operation = new Fill_Operation(Direction::LEFT, FRONTBACK);
        //block4->operation = FB_Fill_Left;
        block4->isCoordinator = true;
        

        MetaModuleBlockCode *block24 = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockById(24)->blockCode
        );
        //block24->operation = BF_Transfer_Left;
        block24->operation = new Transfer_Operation(Direction::LEFT, BACKFRONT, true);
        block24->nextOperation = FB_Fill_Left;
        block24->isCoordinator = true;

        MetaModuleBlockCode *block34 = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockById(34)->blockCode
        );
        //block34->operation = FB_Fill_Left;
        block34->nextOperation = NO_OPERATION;

        block34->operation = new Fill_Operation(Direction::LEFT, FRONTBACK);
        block34->isCoordinator = true;

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
        << " " << coordinateData->position  << "\n";
    
    if(module->position  == coordinateData->position) {
        console << "Can start moving\n";
        mvt_it = coordinateData->it; 
        //Not so Dummy solution for bridging modules movements
        bool bridgeStop = false;
        if(operation->isTransfer()) {
            bridgeStop = static_cast<Transfer_Operation*>(operation)->handleBridgeMovements(this); 
        }      
        if(bridgeStop) return;
        
        coordinatorPosition = coordinateData->coordinatorPosition;
        operation = coordinateData->operation; 
        rotating = true;
        probeGreenLight();
    } else {        
        if(module->getInterface(nearestPositionTo(coordinateData->position, sender))->isConnected()) {
            sendMessage("Coordinate Msg",
                     new MessageOf<Coordinate>(COORDINATE_MSG_ID, *coordinateData),
                     module->getInterface(nearestPositionTo(coordinateData->position, sender)), 100, 200);
        } else {
            sendMessage("Coordinate Msg",
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
            rotating = true;
            LocalMovement lmvt = (*operation->localRules)[mvt_it];
            rotating = true;
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

    if(movingState != MOVING) {
        bool nextToSender = isAdjacentToPosition(srcPos);
        bool nextToTarget = isAdjacentToPosition(targetPos);
        bool targetNextToSrc = false;
       
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
            console << "Neighbor is target light\n";
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
        rotating = true;
        // LocalMovement lmvt = (*localRules)[mvt_it];
        // console << lmvt.nextPosition << "\n";
        // movingState = lmvt.state;
        // movingSteps++;
        // Sender should be pivot to be used for next motion
        Catoms3DBlock* pivot = static_cast<Catoms3DBlock*>(msg->sourceInterface->hostBlock);
        VS_ASSERT(pivot and pivot !=module);
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
    operation =new  Operation();
    mvt_it = 0;
    isCoordinator = false;
    localRules = nullptr;
    if(not greenLightIsOn) {
        setGreenLight(true);
    }
}

bool MetaModuleBlockCode::isInMM(Cell3DPosition &neighborPosition) {
    Cell3DPosition position = (neighborPosition - seedPosition);
    bool isInOpened = (find(OpenedPositions.begin(), OpenedPositions.end(), position) != OpenedPositions.end());
    if(isInOpened)
        return true;
    if(shapeState == FRONTBACK) {
        bool inBorder = (find(FrontBackMM.begin(), FrontBackMM.end(), position) != FrontBackMM.end());
        bool inFill = 
            (find(FillingPositions_FrontBack.begin(), FillingPositions_FrontBack.end(), position) != FillingPositions_FrontBack.end());
        if(inBorder or inFill)
            return true;
    }else if(shapeState == BACKFRONT) {
        bool inBorder = (find(BackFrontMM.begin(), BackFrontMM.end(), position) != BackFrontMM.end());
        bool inFill = 
            (find(FillingPositions_BackFront.begin(), FillingPositions_BackFront.end(), position) != FillingPositions_BackFront.end());
        if(inBorder or inFill)
            return true;
    } else {
        
    }
    return false;
}

Cell3DPosition MetaModuleBlockCode::nearestPositionTo(Cell3DPosition &targetPosition, P2PNetworkInterface *except) {
    double distance = DBL_MAX;
    Cell3DPosition nearest;
    for(auto neighPos: lattice->getActiveNeighborCells(module->position)) {
        if(except != nullptr) {
            if(module->getInterface(neighPos) == except) {
                continue;
            }
        }
        double d = neighPos.dist_euclid(targetPosition);
        if(d < distance) {
            distance = d; 
            nearest = neighPos;
        }
    }
    if(distance == DBL_MAX) {
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
    //         COORDINATE_MSG_ID, Coordinate(operation, nextOperation,  targetModule, module->position, mvt_it)),
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
        rotating = false;
        transferCount = 0;
        setGreenLight(true);
        if((!isCoordinator and operation->isDismantle()) 
            or (operation->isTransfer() 
                and (operation->getDirection() == Direction::LEFT or operation->getDirection() == Direction::RIGHT)
                and mvt_it >= 16 
                )
        ) {
            sendMessage("CoordinateBack Msg", 
                new MessageOf<CoordinateBack>(COORDINATEBACK_MSG_ID, CoordinateBack(movingSteps, coordinatorPosition)),
                module->getInterface(nearestPositionTo(coordinatorPosition)) ,100, 200);
        } 
        console << "coordinator position: " << coordinatorPosition << "\n";
        movingSteps = 0;
        P2PNetworkInterface* pivotItf = module->getInterface(pivotPosition);
        VS_ASSERT(pivotItf and pivotItf->isConnected());
        sendMessageToAllNeighbors("FTR msg",new MessageOf<Cell3DPosition>(FTR_MSG_ID, module->position), 100, 0, 0);
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
            Cell3DPosition& pos = module->getNeighborBlock(face)->position;
            if(not rotating and !isInMM(pos)) {
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
    file.open("BB_Transfer_Left.txt", ios::out | ios::app);
    seedPosition = Cell3DPosition(6,10,10);
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

string MetaModuleBlockCode::onInterfaceDraw() {
    stringstream trace;
    trace << "Some value " << 123;
    return trace.str();
}

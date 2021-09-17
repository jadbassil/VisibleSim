#include "metaModuleBlockCode.hpp"
#include "init.hpp"
#include "messages.hpp"
#include "routing.hpp"
#include <fstream>
#include "robots/catoms3D/catoms3DMotionEngine.h"

using namespace Catoms3D;

MetaModuleBlockCode::MetaModuleBlockCode(Catoms3DBlock *host) : Catoms3DBlockCode(host) {
    // @warning Do not remove block below, as a blockcode with a NULL host might be created
    //  for command line parsing
    if (not host) return;

    // Register callbacks to all messages
    addMessageEventFunc2(GO_MSG_ID,
                         std::bind(&MetaModuleBlockCode::handleGoMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(BACK_MSG_ID,
                         std::bind(&MetaModuleBlockCode::handleBackMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(GOTERM_MSG_ID,
                         std::bind(&MetaModuleBlockCode::handleGoTermMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(BACKTERM_MSG_ID,
                         std::bind(&MetaModuleBlockCode::handleBackTermMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(ACK_MSG_ID,
                         std::bind(&MetaModuleBlockCode::handleAckMessage, this,
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

    addMessageEventFunc2(BFS_MSG_ID,
                         std::bind(&MetaModuleBlockCode::handleBFSMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(CONFIRMEDGE_MSG_ID,
                         std::bind(&MetaModuleBlockCode::handleConfirmEdgeMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(CONFIRMPATH_MSG_ID,
                         std::bind(&MetaModuleBlockCode::handleConfirmPathMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(CONFIRMSTREAMLINE_MSG_ID,
                         std::bind(&MetaModuleBlockCode::handleConfirmStreamlineMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(AVAILABLE_MSG_ID,
                         std::bind(&MetaModuleBlockCode::handleAvailableMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(CUTOFF_MSG_ID, std::bind(&MetaModuleBlockCode::handleCutOffMessage, this,
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
        cerr << seed->position << endl;
        Init::buildMM(module, FRONTBACK, Color(GREEN));
        if(initialMap[0][3] == 1) {
            Init::fillMM(seed);
        }
        Init::buildInitialMap(seed->position, initialMap);
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
    }

    initialColor = module->color;
    initialized = true;
    if(isPotentialSource()) {
        distance = 1;
    }
    VS_ASSERT(Init::initialMapBuildDone);
      if(targetMap.empty()) return;

    if(module->blockId == seed->blockId) {
        reconfigurationStep = SRCDEST;
        nbWaitedAnswers = 0;
        distance = 0;
                 cerr << "Building coordination tree\n";
        for (auto p : getAdjacentMMSeeds()) {
            Cell3DPosition toMMPosition =
                static_cast<MetaModuleBlockCode*>(
                    BaseSimulator::getWorld()->getBlockByPosition(p)->blockCode)
                    ->MMPosition;
   
            sendMessage(
                "Go msg",
                new MessageOf<GOdata>(GO_MSG_ID, GOdata(MMPosition, toMMPosition, distance)),
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

/* -------------------------------------------------------------------------- */
/*                           COORDINATION TREE BUILD                          */
/* -------------------------------------------------------------------------- */
void MetaModuleBlockCode::handleGoMessage(std::shared_ptr<Message> _msg,
                                                  P2PNetworkInterface* sender) {
    MessageOf<GOdata>* msg = static_cast<MessageOf<GOdata>*>(_msg.get());
    GOdata *data = msg->getData();
    console << "Rec. Go msg <" << data->fromMMPosition << "," << data->toMMPosition << ","
            << data->distance << "> from " << sender->getConnectedBlockId() << "\n";
    Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(data->toMMPosition);
    if(module->position != toSeedPosition) {
        if( not interfaceTo(data->fromMMPosition, data->toMMPosition)) VS_ASSERT(false);
        sendMessage(
            "Go msg",
            new MessageOf<GOdata>(GO_MSG_ID, *data),
            interfaceTo(data->fromMMPosition, data->toMMPosition), 100, 200);
        return;
    }
    
    if(parentPosition == Cell3DPosition(-1,-1,-1) and module->blockId != seed->blockId) {
        parentPosition = data->fromMMPosition;
        // distance = data->distance + 1;
        isPotentialSource() ? distance = data->distance + 1: 
            distance = data->distance;
        console << "distance1: " << distance << "\n";
        nbWaitedAnswers = 0;
        for(auto p: getAdjacentMMSeeds()) {
            Cell3DPosition toMMPosition =
                static_cast<MetaModuleBlockCode*>(
                    BaseSimulator::getWorld()->getBlockByPosition(p)->blockCode)
                    ->MMPosition;
            if (toMMPosition == data->fromMMPosition) continue;
            sendMessage(
                "Go msg",
                new MessageOf<GOdata>(GO_MSG_ID, GOdata(MMPosition, toMMPosition, distance)),
                interfaceTo(MMPosition, toMMPosition), 100, 200);
            nbWaitedAnswers++;
        }
        if(nbWaitedAnswers == 0) {
            Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(parentPosition);
            sendMessage(
                "Back msg",
                new MessageOf<Backdata>(BACK_MSG_ID, Backdata(MMPosition, parentPosition, true)),
                interfaceTo(MMPosition, parentPosition), 100, 200);
        } 
    } else if( (not isPotentialSource() and data->distance < distance) or (isPotentialSource() and data->distance + 1 < distance)) {
        Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(parentPosition);
        sendMessage(
                "Back msg",
                new MessageOf<Backdata>(BACK_MSG_ID, Backdata(MMPosition, parentPosition, false)),
                interfaceTo(MMPosition, parentPosition), 100, 200);
        parentPosition = data->fromMMPosition;
        childrenPositions.clear();
        isPotentialSource() ? distance = data->distance + 1: 
            distance = data->distance;
        console << "distance2 : " << distance << "\n";

        // nbWaitedAnswers = 0;
        for(auto p: getAdjacentMMSeeds()) {
            Cell3DPosition toMMPosition =
                static_cast<MetaModuleBlockCode*>(
                    BaseSimulator::getWorld()->getBlockByPosition(p)->blockCode)
                    ->MMPosition;
            if(toMMPosition == data->fromMMPosition) continue;
            sendMessage(
                "Go msg",
                new MessageOf<GOdata>(GO_MSG_ID, GOdata(MMPosition, toMMPosition, distance)),
                interfaceTo(MMPosition, toMMPosition), 100, 200);
            nbWaitedAnswers++;
        }
        if(nbWaitedAnswers == 0) {
            Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(parentPosition);
            sendMessage(
                "Back msg",
                new MessageOf<Backdata>(BACK_MSG_ID, Backdata(MMPosition, parentPosition, true)),
                interfaceTo(MMPosition, parentPosition), 100, 200);
        }
    } else {
        Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(data->fromMMPosition);
        sendMessage(
            "Back msg1",
            new MessageOf<Backdata>(BACK_MSG_ID, Backdata(MMPosition, data->fromMMPosition, false)),
            interfaceTo(MMPosition, data->fromMMPosition), 100, 200);
    }
    console << "parent: " << parentPosition << "\n";
}

void MetaModuleBlockCode::handleBackMessage(std::shared_ptr<Message> _msg,
                                                  P2PNetworkInterface* sender) {
    MessageOf<Backdata>* msg = static_cast<MessageOf<Backdata>*>(_msg.get());
    Backdata *data = msg->getData();
    console << "Rec. Back msg <" << data->fromMMPosition << "," << data->toMMPosition << ","
            << data->isChild << "> from " << sender->getConnectedBlockId() << "\n";
    Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(data->toMMPosition);
    if(module->position != toSeedPosition) {
        sendMessage(
            "Back msg",
            new MessageOf<Backdata>(BACK_MSG_ID, *data),
            interfaceTo(data->fromMMPosition, data->toMMPosition), 100, 200);
        return;
    }
    nbWaitedAnswers--;
    console << "nbWaitedAnswers: " << nbWaitedAnswers << "\n";
    vector<Cell3DPosition>::iterator it =
            find(childrenPositions.begin(), childrenPositions.end(), data->fromMMPosition);
    if(data->isChild and it == childrenPositions.end()) {
        childrenPositions.push_back(data->fromMMPosition);
    } else if(not data->isChild){  
        if(it != childrenPositions.end()) {
            childrenPositions.erase(it);
        }
    }
    if(nbWaitedAnswers == 0) {
        if(parentPosition == Cell3DPosition(-1,-1,-1) and module->position == seed->position) {
            cerr << module->blockId << ": Coordination Tree is Built\n";
            reconfigurationStep = MAXFLOW;
            for (auto block : BaseSimulator::getWorld()->buildingBlocksMap) {
                MetaModuleBlockCode *MMblock = static_cast<MetaModuleBlockCode*>(block.second->blockCode);
                if (MMblock->isPotentialSource() and
                    MMblock->seedPosition == MMblock->module->position and
                    MMblock->childrenPositions.empty()) {

                    MMblock->isSource = true;
                    cerr << MMblock->MMPosition << ": is source\n";
                    // VS_ASSERT(mainPathState == NONE);
                    MMblock->state = ACTIVE;
                    MMblock->mainPathState = BFS;
                    MMblock->mainPathIn = MMblock->MMPosition;
                    MMblock->mainPathsOld.push_back(MMblock->MMPosition);
                    for (auto p : MMblock->getAdjacentMMSeeds()) {
                        // cerr << MMPosition << ": " << p << endl;
                        MetaModuleBlockCode* toSeed = static_cast<MetaModuleBlockCode*>(
                            BaseSimulator::getWorld()->getBlockByPosition(p)->blockCode);
                        MMblock->deficit++;
                        MMblock->sendMessage(
                            "BFS msg",
                            new MessageOf<BFSdata>(BFS_MSG_ID, BFSdata(MMblock->MMPosition, MMblock->MMPosition, toSeed->MMPosition)),
                            MMblock->interfaceTo(MMblock->MMPosition, toSeed->MMPosition), 100, 200);
                    }
                }
                if (MMblock->isPotentialDestination() and MMblock->seedPosition == MMblock->module->position) {
                    cerr << MMblock->MMPosition << ": is destination\n";
                }                
            }
            start_wave();
        } else {
            Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(parentPosition);
            sendMessage(
                "Back msg",
                new MessageOf<Backdata>(BACK_MSG_ID, Backdata(MMPosition, parentPosition, true)),
                interfaceTo(MMPosition, parentPosition), 100, 200);
        }
    }
}

void MetaModuleBlockCode::handleGoTermMessage(std::shared_ptr<Message> _msg,
                                                  P2PNetworkInterface* sender) {
    MessageOf<FromToMsg>* msg = static_cast<MessageOf<FromToMsg>*>(_msg.get()); 
    Cell3DPosition fromMMPosition = msg->getData()->fromMMPosition; 
    Cell3DPosition toMMPosition = msg->getData()->toMMPosition;   
    Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(toMMPosition);
    console << "Rec. GoTerm Msg form: "<< sender->getConnectedBlockId() << "\n"; 
    if (module->position != toSeedPosition) {
        sendMessage("GoTerm Msg", new MessageOf<FromToMsg>(GOTERM_MSG_ID, FromToMsg(fromMMPosition, toMMPosition)),
                    interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    nbWaitedAnswers = 0;
    for (auto child : childrenPositions) {
        Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(child);
        sendMessage("GoTerm Msg", new MessageOf<FromToMsg>(GOTERM_MSG_ID, FromToMsg(MMPosition, child)),
                    interfaceTo(MMPosition, child), 100, 200);
        nbWaitedAnswers++;
    }
    if(nbWaitedAnswers == 0) {
        if(state == PASSIVE and deficit == 0) {
            b = cont_passive;
            Cell3DPosition parentSeedPosition = getSeedPositionFromMMPosition(parentPosition);
            sendMessage("BackTerm Msg1",
                        new MessageOf<pair<FromToMsg, bool>>(
                            BACKTERM_MSG_ID, make_pair(FromToMsg(MMPosition, parentPosition), b)),
                        interfaceTo(MMPosition, parentPosition), 100, 200);
            cont_passive = true;
        } else {
            getScheduler()->schedule(
                new InterruptionEvent(getScheduler()->now() + 500, module, IT_MODE_TERMINATION));
        }
    }
}

void MetaModuleBlockCode::handleBackTermMessage(std::shared_ptr<Message> _msg,
                                                  P2PNetworkInterface* sender) {
    MessageOf<pair<FromToMsg,bool>>* msg = static_cast<MessageOf<pair<FromToMsg,bool>>*>(_msg.get());
    Cell3DPosition fromMMPosition = msg->getData()->first.fromMMPosition;
    Cell3DPosition toMMPosition = msg->getData()->first.toMMPosition;
    Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(toMMPosition);
    if (module->position != toSeedPosition) {
        sendMessage("BackTerm Msg2", new MessageOf<pair<FromToMsg,bool>>(BACKTERM_MSG_ID, make_pair(msg->getData()->first, msg->getData()->second)),
                interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    res = res and msg->getData()->second;
    nbWaitedAnswers--;
    console << "Rec. BackTerm " << nbWaitedAnswers << "\n";
    if (nbWaitedAnswers == 0) {
        if (state == PASSIVE and deficit == 0) {
            b = cont_passive;
            cont_passive = true;
            if (parentPosition != Cell3DPosition(-1, -1, -1)) {
                Cell3DPosition parentSeedPosition = getSeedPositionFromMMPosition(parentPosition);
                sendMessage("BackTerm Msg3",
                            new MessageOf<pair<FromToMsg, bool>>(
                                BACKTERM_MSG_ID, make_pair(FromToMsg(MMPosition, parentPosition), res and b)),
                            interfaceTo(MMPosition, parentPosition), 100, 200);
            } else {
                console << "res: " << res << "; b: " << b << "\n";
                if (res and b) {
                    cerr << "MaxFlow Terminated!!" << endl;
                    cerr << "Starting Modules Transportation" << endl;
                    reconfigurationStep = TRANSPORT;
                    for (auto id_block : BaseSimulator::getWorld()->buildingBlocksMap) {
                        MetaModuleBlockCode* block =
                            static_cast<MetaModuleBlockCode*>(id_block.second->blockCode);
                        if (block->isSource and block->mainPathState == Streamline and
                            block->module->position == block->seedPosition) {
                            MetaModuleBlockCode* coord = static_cast<MetaModuleBlockCode*>(
                                BaseSimulator::getWorld()
                                    ->getBlockByPosition(block->coordinatorPosition)
                                    ->blockCode);
                            Cell3DPosition targetModule =
                                block->seedPosition +
                                (*coord->operation->localRules)[0].currentPosition;
                            coord->console
                                << "targetModule: " << coord->nearestPositionTo(targetModule)
                                << "\n";
                            coord->sendMessage(
                                "Coordinate Msg1",
                                new MessageOf<Coordinate>(
                                    COORDINATE_MSG_ID,
                                    Coordinate(coord->operation, targetModule,
                                               coord->module->position, coord->mvt_it)),
                                coord->module->getInterface(coord->nearestPositionTo(targetModule)),
                                100, 200);
                        }
                    }
                } else {
                    start_wave();
                }
            }
        } else {
            getScheduler()->schedule(
                new InterruptionEvent(getScheduler()->now() + 500, module, IT_MODE_TERMINATION));
        }
    }
    res = true;
}

void MetaModuleBlockCode::reinitialize() {
    for (auto id_block : BaseSimulator::getWorld()->buildingBlocksMap) {
        MetaModuleBlockCode* block = static_cast<MetaModuleBlockCode*>(id_block.second->blockCode);
        block->parentPosition = Cell3DPosition(-1, -1, -1);
        block->childrenPositions.clear();
        block->distance = 0;
        block->isSource = false;
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
    }
}

void MetaModuleBlockCode::handleAckMessage(std::shared_ptr<Message> _msg,
                                                  P2PNetworkInterface* sender) {
    MessageOf<FromToMsg>* msg = static_cast<MessageOf<FromToMsg>*>(_msg.get()); 
    Cell3DPosition fromMMPosition = msg->getData()->fromMMPosition;
    Cell3DPosition toMMPosition = msg->getData()->toMMPosition;

    Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(toMMPosition);
    if (module->position != toSeedPosition) {
        sendMessage("Ack Msg", new MessageOf<FromToMsg>(ACK_MSG_ID, FromToMsg(fromMMPosition, toMMPosition)),
                    interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    } else {
        console << "Rec: Ack from: "  << fromMMPosition << "\n";
        deficit--;
    }
}

void MetaModuleBlockCode::start_wave() {
    //cont_passive = false;
    nbWaitedAnswers = 0;
    for (auto child : childrenPositions) {
        Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(child);
        sendMessage("GoTerm Msg", new MessageOf<FromToMsg>(GOTERM_MSG_ID, FromToMsg(MMPosition, child)),
                    interfaceTo(MMPosition, child), 100, 200);
        nbWaitedAnswers++;
    }
}

void MetaModuleBlockCode::return_wave(bool b) {

}

/* -------------------------------------------------------------------------- */


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
    MetaModuleBlockCode* senderMM = static_cast<MetaModuleBlockCode*>(
        BaseSimulator::getWorld()->getBlockById(sender->getConnectedBlockId())->blockCode);
    if(senderMM->sendingCoordinateBack) {
            senderMM->sendingCoordinateBack = false;
    } 
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

        if((*operation->localRules)[mvt_it-1].state == MOVING and operation->isTransfer()) {
            operation->setMvtItToNextModule(module->blockCode);
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
                    module->getInterface(
                        nearestPositionTo(coordinateBackData->coordinatorPosition, sender)),
                    100, 200);
    }                     
}

void MetaModuleBlockCode::setOperation(Cell3DPosition inPosition, Cell3DPosition outPosition) {
    Direction direction;
    Cell3DPosition directionVector = outPosition - MMPosition;
    if (directionVector.pt[0] == -1) direction = Direction::LEFT;
    if (directionVector.pt[0] == 1) direction = Direction::RIGHT;
    if (directionVector.pt[1] == -1) direction = Direction::FRONT;
    if (directionVector.pt[1] == 1) direction = Direction::BACK;
    if (directionVector.pt[2] == -1) direction = Direction::DOWN;
    if (directionVector.pt[2] == 1) direction = Direction::UP;
    if(isSource) {
        (shapeState == FRONTBACK) ? coordinatorPosition = seedPosition + Cell3DPosition(-1, -1, 2)
                                  : coordinatorPosition = seedPosition + Cell3DPosition(-1, 1, 2);
        if(direction == Direction::BACK and shapeState == FRONTBACK) {
            coordinatorPosition = seedPosition + Cell3DPosition(2,1,2);
        } else if(direction ==Direction::UP) {
            if(shapeState == BACKFRONT and MMPosition.pt[2] % 2 != 0) {
                coordinatorPosition = seedPosition + Cell3DPosition(1,0,4);
            }
        }
    } else {
        (shapeState == FRONTBACK) ? coordinatorPosition = seedPosition + Cell3DPosition(1, 0, 1)
                              : coordinatorPosition = seedPosition + Cell3DPosition(1, -1, 1);
       
    }
     MetaModuleBlockCode* coordinator = static_cast<MetaModuleBlockCode*>(
            BaseSimulator::getWorld()->getBlockByPosition(coordinatorPosition)->blockCode);
    coordinator->isCoordinator = true;
    if (isSource) {
        coordinator->operation = new Dismantle_Operation(direction, shapeState, MMPosition.pt[2], false);
    } else if (isPotentialDestination()) {
        bool comingFromBack = (inPosition.pt[0] == MMPosition.pt[0] and inPosition.pt[1] == MMPosition.pt[1]-1 and shapeState == BACKFRONT);
       coordinator->operation = new Build_Operation(direction, shapeState, comingFromBack,  MMPosition.pt[2]);
    } else {
        bool comingFromBack = (inPosition.pt[0] == MMPosition.pt[0] and inPosition.pt[1] == MMPosition.pt[1]-1 and shapeState == BACKFRONT);
        coordinator->operation = new Transfer_Operation(direction, shapeState, comingFromBack, MMPosition.pt[2]);
    }

}

 /* -------------------------------------------------------------------------- */
 /*                             MOTION COORDINATION                            */
 /* -------------------------------------------------------------------------- */


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
        console << (module->getState() == BuildingBlock::State::ACTUATING) << "\n";

        if (module->getState() == BuildingBlock::State::ACTUATING) {
            getScheduler()->trace("light turned orange1", module->blockId, ORANGE);
            if (moduleAwaitingGo) setGreenLight(true);
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
            MetaModuleBlockCode *targetLightNeighborMM = static_cast<MetaModuleBlockCode*>(targetLightNeighbor->blockCode);
            if(targetLightNeighborMM->isCoordinator and targetLightNeighborMM->operation->getDirection() == Direction::UP) {
                targetNextToSrc = false;
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

                getScheduler()->trace("light turned orange2", module->blockId, ORANGE);
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
        console << "ERROR: " << targetPos << "\n";
        //VS_ASSERT(itf and itf->isConnected());

        setGreenLight(false);
        if(itf->isConnected())
        sendMessage("GLO Msg", new MessageOf<PLS>(GLO_MSG_ID, PLS(srcPos, targetPos)), itf, 100, 0);
    } else if(module->position == targetPos){
        VS_ASSERT(module->position == targetPos);
        Cell3DPosition targetPosition =
            (*operation->localRules)[mvt_it].nextPosition + seedPosition;
        if (module->canRotateToPosition(targetPosition)) {
            if ((relativePos() == Cell3DPosition(-1, 1, 2) /**or
                              /ùùrelativePos() == Cell3DPosition(-1, -1, 2)**/) or
                relativePos() == Cell3DPosition(1, 0, 1) or
                relativePos() == Cell3DPosition(1, -1, 1) or
                relativePos() == Cell3DPosition(2, 0, 2) or relativePos() == Cell3DPosition(1,2,2) or relativePos() == Cell3DPosition(1,0,2)) {
                // if(relativePos() == Cell3DPosition(1,2,2)) VS_ASSERT(false);
                getScheduler()->schedule(
                    new Catoms3DRotationStartEvent(getScheduler()->now(), module, targetPosition,
                                                   RotationLinkType::OctaFace, false));
            } else {
                getScheduler()->schedule(new Catoms3DRotationStartEvent(
                    getScheduler()->now(), module, targetPosition, RotationLinkType::Any, false));
            }

        } else {
            // retry rotating to next position
            probeGreenLight();
            // VS_ASSERT_MSG(false, "Can not rotate to next position");
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

    if (lmvt.nextPosition == Cell3DPosition(2, 0, 2) and
        module->getInterface(module->position.offsetX(1))->isConnected() and
        operation->getDirection() == Direction::UP) {
        (*operation->localRules)[mvt_it].nextPosition = Cell3DPosition(1, 0, 2);
    }
    Cell3DPosition targetPosition = lmvt.nextPosition + seedPosition;
    if(not greenLightIsOn) setGreenLight(true);
    Catoms3DBlock* pivot = customFindMotionPivot(module, targetPosition);
    if (not pivot) {
        notFindingPivot = true;
        getScheduler()->schedule(new InterruptionEvent(getScheduler()->now() + getRoundDuration(),
                                                       module, IT_MODE_FINDING_PIVOT));
        stringstream info;
        info << " reattempt finding pivot for " << targetPosition;
        scheduler->trace(info.str(), module->blockId, PINK);
        return;
    }
    if(operation->isDismantle() and shapeState == FRONTBACK and operation->getDirection() == Direction::BACK){
        if(lmvt.nextPosition == Cell3DPosition(1,2,2) and lattice->cellHasBlock(seedPosition + Cell3DPosition(1,3,2))) {
            getScheduler()->schedule(new InterruptionEvent(getScheduler()->now() + getRoundDuration(),
                                                       module, IT_MODE_DISMANTLEBACK));
            return;
        }
    }
    notFindingPivot = false;
    // VS_ASSERT(pivot);
    console << "pivot: " << pivot->position << "\n";
    if (module->getInterface(pivot->position)->isConnected())
        sendMessage("PLS Msg",
                    new MessageOf<PLS>(PLS_MSG_ID, PLS(module->position, targetPosition)),
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

Catoms3DBlock* MetaModuleBlockCode::findTargetLightAmongNeighbors(
    const Cell3DPosition& targetPos, const Cell3DPosition& srcPos,
    P2PNetworkInterface* sender) const {

    Cell3DPosition except;
    if (sender) {
        module->getNeighborPos(module->getInterfaceId(sender), except);
    }
    for (const auto& cell : lattice->getActiveNeighborCells(module->position)) {
        if (lattice->cellsAreAdjacent(cell, targetPos) and cell != srcPos) {
            if (sender and cell == except) continue;
            MetaModuleBlockCode* block = static_cast<MetaModuleBlockCode*>(
                BaseSimulator::getWorld()->getBlockByPosition(cell)->blockCode);
            if (block->rotating) continue;

            return static_cast<Catoms3DBlock*>(lattice->getBlock(cell));
        }
    }
    return NULL;
}
/* -------------------------------------------------
    short pt[3]; //!< (x,y,z) values of the vector
    Cell3DPosition();------------------------- */



/* -------------------------------------------------------------------------- */
/*                              MAX-FLOW HANDLERS                             */
/* -------------------------------------------------------------------------- */
void MetaModuleBlockCode::handleBFSMessage(std::shared_ptr<Message> _msg,
                                               P2PNetworkInterface* sender) {
    MessageOf<BFSdata>* msg = static_cast<MessageOf<BFSdata>*>(_msg.get());
    BFSdata data = *msg->getData();
    console << "Rec. BFS <" << data.MMPosition << ", " << data.fromMMPosition << ", " << data.toMMPosition << "> from "
            << sender->getConnectedBlockId() << "\n";
    if(module->position != getSeedPositionFromMMPosition(data.toMMPosition)) {
        //forward the message to seed target
        sendMessage("BFS msg", new MessageOf<BFSdata>(BFS_MSG_ID, data),
                    interfaceTo(data.fromMMPosition, data.toMMPosition), 100, 200);
        return;
    }
    vector<Cell3DPosition> pathsOld;
    pathsOld += mainPathsOld;
    pathsOld += aug1PathsOld;
    pathsOld += aug2PathsOld;
    console << "pathsOld: ";
    for(auto pOld: pathsOld) console << pOld << "; ";
    console << "\n";
    if(mainPathState == NONE and !isIn(pathsOld, data.MMPosition)) {
        mainPathsOld.push_back(data.MMPosition);
        Cell3DPosition fromSeedPosition = getSeedPositionFromMMPosition(data.fromMMPosition);
        deficit++;
        sendMessage("ConfirmEdge msg",
                    new MessageOf<MaxFlowMsgData>(CONFIRMEDGE_MSG_ID, MaxFlowMsgData(MMPosition, data.fromMMPosition)),
                    interfaceTo(MMPosition, data.fromMMPosition), 100, 200);
        if(isPotentialDestination()) {
            mainPathState = ConfPath;
            mainPathIn = data.fromMMPosition;
            mainPathOut.clear();
            //mainPathOut.push_back(MMPosition);
            console << "fromSeedPosition: " << fromSeedPosition << "\n";
            deficit++;
            sendMessage("ConfirmPath msg",
                        new MessageOf<MaxFlowMsgData>(CONFIRMPATH_MSG_ID, MaxFlowMsgData(MMPosition, data.fromMMPosition)),
                        interfaceTo(MMPosition, data.fromMMPosition), 100, 200);
        } else {
            mainPathState = BFS;
            mainPathIn = data.fromMMPosition;
            mainPathOut.clear();
            // sendAround()
            for(auto p: getAdjacentMMSeeds()) {
                console << "Send Around\n";
                MetaModuleBlockCode* toSeed = static_cast<MetaModuleBlockCode*>(
                            BaseSimulator::getWorld()->getBlockByPosition(p)->blockCode);

                deficit++;
                sendMessage(
                    "BFS msg",
                    new MessageOf<BFSdata>(BFS_MSG_ID, BFSdata(data.MMPosition, MMPosition,
                                                            toSeed->MMPosition)),
                    interfaceTo(MMPosition, toSeed->MMPosition), 100, 200);
            }
        }
    } else if (mainPathState == Streamline and aug1PathState == NONE and
               data.fromMMPosition != mainPathIn and data.fromMMPosition != mainPathOut and
               !isIn(pathsOld, data.MMPosition)) {
        aug1PathsOld.push_back(data.MMPosition);
        Cell3DPosition fromSeedPosition = getSeedPositionFromMMPosition(data.fromMMPosition);
        deficit++;
        sendMessage("ConfirmEdge msg1",
                    new MessageOf<MaxFlowMsgData>(
                        CONFIRMEDGE_MSG_ID, MaxFlowMsgData(MMPosition, data.fromMMPosition)),
                    interfaceTo(MMPosition, data.fromMMPosition), 100, 200);
        aug1PathState = BFS;
        aug1PathIn = data.fromMMPosition;
        aug1PathOut.clear();
        Cell3DPosition mainPathInSeedPosition = getSeedPositionFromMMPosition(mainPathIn);
        if(mainPathIn != MMPosition) {
            deficit++;
            sendMessage("BFS msg",
                    new MessageOf<BFSdata>(
                        BFS_MSG_ID, BFSdata(data.MMPosition, MMPosition, mainPathIn)),
                    interfaceTo(MMPosition, mainPathIn), 100, 200);
        }
        
    } else if(mainPathState == Streamline and aug2PathState == NONE and data.fromMMPosition == mainPathOut){
        aug2PathsOld.push_back(data.MMPosition);
        Cell3DPosition fromSeedPosition = getSeedPositionFromMMPosition(data.fromMMPosition);
        deficit++;
        sendMessage("ConfirmEdge msg2",
                    new MessageOf<MaxFlowMsgData>(
                        CONFIRMEDGE_MSG_ID, MaxFlowMsgData(MMPosition, data.fromMMPosition)),
                    interfaceTo(MMPosition, data.fromMMPosition), 100, 200);
        aug2PathState = BFS;
        aug2PathIn = data.fromMMPosition;
        aug2PathOut.clear();
        for(auto p: getAdjacentMMSeeds()) {
            console << "Send Around\n";
            Cell3DPosition seedMMPosition = static_cast<MetaModuleBlockCode*>(
                BaseSimulator::getWorld()->getBlockByPosition(p)->blockCode)->MMPosition;
            if(seedMMPosition == data.fromMMPosition) continue;
            deficit++;
            sendMessage(
                "BFS msg",
                new MessageOf<BFSdata>(BFS_MSG_ID, BFSdata(data.MMPosition, MMPosition,
                                                        seedMMPosition)),
                interfaceTo(MMPosition, seedMMPosition), 100, 200);
        }
    } else {
        console << MMPosition << ": path can no longer be augmented" << "\n";

    }
    state = PASSIVE;
}

void MetaModuleBlockCode::handleConfirmEdgeMessage(std::shared_ptr<Message> _msg,
                                               P2PNetworkInterface* sender) {
    MessageOf<MaxFlowMsgData>* msg = static_cast<MessageOf<MaxFlowMsgData>*>(_msg.get());
    Cell3DPosition fromMMPosition = (*msg->getData()).fromMMPosition;
    Cell3DPosition toMMPosition = (*msg->getData()).toMMPosition;
    console << "Rec. ConfirmEdge msg <" << toMMPosition << ", " << fromMMPosition << "> from "
            << sender->getConnectedBlockId() << "\n";
    Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(toMMPosition);
    if(module->position != toSeedPosition) {
        sendMessage("ConfirmEdge msg", new MessageOf<MaxFlowMsgData>(CONFIRMEDGE_MSG_ID, MaxFlowMsgData(fromMMPosition, toMMPosition)),
            interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    if(mainPathState == BFS) {
        mainPathOut.push_back(fromMMPosition);
    } else if(aug1PathState == BFS) {
        aug1PathOut.push_back(fromMMPosition);
    } else if(aug2PathState == BFS) {
        aug2PathOut.push_back(fromMMPosition);
    } else {
        deficit++;
        Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(fromMMPosition);
        sendMessage("CutOff msg", new MessageOf<MaxFlowMsgData>(CUTOFF_MSG_ID, MaxFlowMsgData(MMPosition, fromMMPosition)),
            interfaceTo(MMPosition, fromMMPosition), 100, 200);
    }
    state = PASSIVE;
}

void MetaModuleBlockCode::handleConfirmPathMessage(std::shared_ptr<Message> _msg,
                                               P2PNetworkInterface* sender) {
    MessageOf<MaxFlowMsgData>* msg = static_cast<MessageOf<MaxFlowMsgData>*>(_msg.get());
    Cell3DPosition fromMMPosition = (*msg->getData()).fromMMPosition;
    Cell3DPosition toMMPosition = (*msg->getData()).toMMPosition;
    console << "Rec. ConfirmPath msg <" << toMMPosition << ", " << fromMMPosition << "> from "
            << sender->getConnectedBlockId() << "\n";
    Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(toMMPosition);
    if (module->position != toSeedPosition) {
        sendMessage("ConfirmPath msg",
                    new MessageOf<MaxFlowMsgData>(CONFIRMPATH_MSG_ID,
                                                  MaxFlowMsgData(fromMMPosition, toMMPosition)),
                    interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    console << "mainPathState: " << mainPathState << "\n";
    console << "aug1PathState: " << aug1PathState << "\n";
    console << "aug2PathState: " << aug2PathState << "\n";
    console << "toSeedPositition: " << toSeedPosition << "\n";
    if(mainPathState == BFS and isIn(mainPathOut, fromMMPosition)) {
        for(auto out: mainPathOut) {
            if(out != fromMMPosition) {
                deficit++;
                Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(out);
                sendMessage("CutOff msgMain", new MessageOf<MaxFlowMsgData>(CUTOFF_MSG_ID, MaxFlowMsgData(MMPosition, out)),
                    interfaceTo(MMPosition, out), 100, 200);
            }
        }
        mainPathOut.clear();
        mainPathOut.push_back(fromMMPosition);
        if(isSource) {
            cerr << "Streamline from source: " << MMPosition << " is established" << endl;
            mainPathState = Streamline;
            Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(fromMMPosition);
            deficit++;
            sendMessage("ConfirmStreamLine msg",
                        new MessageOf<MaxFlowMsgData>(CONFIRMSTREAMLINE_MSG_ID,
                                                      MaxFlowMsgData(MMPosition, fromMMPosition)),
                        interfaceTo(MMPosition, fromMMPosition), 100, 200);
            setOperation(mainPathIn, mainPathOut.front());
        } else {
            mainPathState = ConfPath;
            Cell3DPosition mainPathInSeedPosition = getSeedPositionFromMMPosition(mainPathIn);
            deficit++;
            sendMessage("ConfirmPath msgMain",
                        new MessageOf<MaxFlowMsgData>(CONFIRMPATH_MSG_ID,
                                                      MaxFlowMsgData(MMPosition, mainPathIn)),
                        interfaceTo(MMPosition, mainPathIn), 100, 200);
        }
    }else if(aug1PathState == BFS and fromMMPosition == mainPathIn) {
        aug1PathOut.clear();
        aug1PathOut.push_back(fromMMPosition);
        aug1PathState = ConfPath;
        Cell3DPosition aug1PathInSeedPosition = getSeedPositionFromMMPosition(aug1PathIn);
        deficit++;
        sendMessage("ConfirmPath msgAug1",
                    new MessageOf<MaxFlowMsgData>(CONFIRMPATH_MSG_ID,
                                                  MaxFlowMsgData(MMPosition, aug1PathIn)),
                    interfaceTo(MMPosition, aug1PathIn), 100, 200);
    }else if(aug2PathState == BFS and (aug1PathState != ConfPath or fromMMPosition != mainPathIn)) {
        for(auto out: aug2PathOut) {
            if(out != fromMMPosition) {
                Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(out);
                deficit++;
                sendMessage("CutOff msgAug2", new MessageOf<MaxFlowMsgData>(CUTOFF_MSG_ID, MaxFlowMsgData(MMPosition, out)),
                    interfaceTo(MMPosition, out), 100, 200);
            }
        }
        aug2PathOut.clear();
        aug2PathOut.push_back(fromMMPosition);
        aug2PathState = ConfPath;
        Cell3DPosition aug2PathInSeedPosition = getSeedPositionFromMMPosition(aug2PathIn);
        deficit++;
        sendMessage("ConfirmPath msgAug2",
                    new MessageOf<MaxFlowMsgData>(CONFIRMPATH_MSG_ID,
                                                  MaxFlowMsgData(MMPosition, aug2PathIn)),
                    interfaceTo(MMPosition, aug2PathIn), 100, 200);
    } else {
        console << "test\n";
    }
    state = PASSIVE;
}

void MetaModuleBlockCode::handleConfirmStreamlineMessage(std::shared_ptr<Message> _msg,
                                               P2PNetworkInterface* sender) {
    MessageOf<MaxFlowMsgData>* msg = static_cast<MessageOf<MaxFlowMsgData>*>(_msg.get());
    Cell3DPosition fromMMPosition = (*msg->getData()).fromMMPosition;
    Cell3DPosition toMMPosition = (*msg->getData()).toMMPosition;
    console << "Rec. ConfirmStreamline msg <" << toMMPosition << ", " << fromMMPosition << "> from "
            << sender->getConnectedBlockId() << "\n";
    Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(toMMPosition);
    if (module->position != toSeedPosition) {
        deficit++;
        sendMessage("ConfirmStreamline msg",
                    new MessageOf<MaxFlowMsgData>(CONFIRMSTREAMLINE_MSG_ID,
                                                  MaxFlowMsgData(fromMMPosition, toMMPosition)),
                    interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    if(mainPathState == ConfPath and fromMMPosition == mainPathIn) {
        mainPathState = Streamline;
        if (not isPotentialDestination()) {
            VS_ASSERT(not mainPathOut.empty());
            Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(mainPathOut.front());
            deficit++;
            sendMessage(
                "ConfirmStreamline msg",
                new MessageOf<MaxFlowMsgData>(CONFIRMSTREAMLINE_MSG_ID,
                                              MaxFlowMsgData(MMPosition, mainPathOut.front())),
                interfaceTo(MMPosition, mainPathOut.front()), 100, 200);
        } else {
            NbOfStreamlines++;
        }
        Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(mainPathIn);
        deficit++;
        sendMessage("Available msg",
                new MessageOf<MaxFlowMsgData>(AVAILABLE_MSG_ID,
                                                MaxFlowMsgData(MMPosition, mainPathIn)),
                interfaceTo(MMPosition, mainPathIn), 100, 200);
        for(auto out: mainPathOut) {
            console << "Send Available MainPathOut\n";
            Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(out);
            deficit++;
            sendMessage("Available msg",
                    new MessageOf<MaxFlowMsgData>(AVAILABLE_MSG_ID,
                                                  MaxFlowMsgData(MMPosition, out)),
                    interfaceTo(MMPosition, out), 100, 200);
        } 
    } else if(aug1PathState == ConfPath and fromMMPosition == aug1PathIn){
        mainPathIn = aug1PathIn;
        VS_ASSERT(not aug1PathOut.empty());
        Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(aug1PathOut.front());
        deficit++;
        sendMessage(
                "ConfirmStreamline msg",
                new MessageOf<MaxFlowMsgData>(CONFIRMSTREAMLINE_MSG_ID,
                                              MaxFlowMsgData(MMPosition, aug1PathOut.front())),
                interfaceTo(MMPosition, aug1PathOut.front()), 100, 200);
        toSeedPosition = getSeedPositionFromMMPosition(aug1PathIn);
        deficit++;
        sendMessage("Available msg",
                new MessageOf<MaxFlowMsgData>(AVAILABLE_MSG_ID,
                                                MaxFlowMsgData(MMPosition, aug1PathIn)),
                interfaceTo(MMPosition, aug1PathIn), 100, 200);
        for(auto out: aug1PathOut) {
            console << "Send Around Available aug1PathOut\n";
            toSeedPosition = getSeedPositionFromMMPosition(out);
            deficit++;
            sendMessage("Available msg",
                    new MessageOf<MaxFlowMsgData>(AVAILABLE_MSG_ID,
                                                  MaxFlowMsgData(MMPosition, out)),
                    interfaceTo(MMPosition, out), 100, 200);
        } 
        aug1PathState = NONE;
        aug1PathOut.clear();
        aug1PathIn.set(-1,-1,-1);
    }  else if(aug2PathState == ConfPath and fromMMPosition == aug2PathIn) {
        VS_ASSERT(not aug2PathOut.empty());
        Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(aug2PathOut.front());
        deficit++;
        sendMessage(
                "ConfirmStreamline msg",
                new MessageOf<MaxFlowMsgData>(CONFIRMSTREAMLINE_MSG_ID,
                                              MaxFlowMsgData(MMPosition, aug2PathOut.front())),
                interfaceTo(MMPosition, aug2PathOut.front()), 100, 200);
        toSeedPosition = getSeedPositionFromMMPosition(aug2PathIn);
        deficit++;
        sendMessage("Available msg",
                new MessageOf<MaxFlowMsgData>(AVAILABLE_MSG_ID,
                                                MaxFlowMsgData(MMPosition, aug2PathIn)),
                interfaceTo(MMPosition, aug2PathIn), 100, 200);
        for(auto out: aug2PathOut) {
            console << "Send Around Available aug2PathOut\n";
            toSeedPosition = getSeedPositionFromMMPosition(out);
            deficit++;
            sendMessage("Available msg",
                    new MessageOf<MaxFlowMsgData>(AVAILABLE_MSG_ID,
                                                  MaxFlowMsgData(MMPosition, out)),
                    interfaceTo(MMPosition, out), 100, 200);
        } 
        if(aug2PathOut.front() == mainPathIn) {
            mainPathState = NONE;
            mainPathIn.set(-1,-1,-1);
            mainPathOut.clear();
            aug1PathState = NONE;
            aug1PathOut.clear();
            aug1PathIn.set(-1,-1,-1);
        } else {
            mainPathOut = aug2PathOut;
        }
        aug2PathState = NONE;
        aug2PathOut.clear();
        aug2PathIn.set(-1,-1,-1);
    } 
    if(not isPotentialDestination())
        setOperation(mainPathIn, mainPathOut.front());
    else
        setOperation(mainPathIn, destinationOut);
    state = PASSIVE;
}

void MetaModuleBlockCode::handleAvailableMessage(std::shared_ptr<Message> _msg,
                                               P2PNetworkInterface* sender) {
    MessageOf<MaxFlowMsgData>* msg = static_cast<MessageOf<MaxFlowMsgData>*>(_msg.get());
    Cell3DPosition fromMMPosition = (*msg->getData()).fromMMPosition;
    Cell3DPosition toMMPosition = (*msg->getData()).toMMPosition;
    console << "Rec. Available msg <" << toMMPosition << ", " << fromMMPosition << "> from "
            << sender->getConnectedBlockId() << "\n";
    Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(toMMPosition);
    if (module->position != toSeedPosition) {
        sendMessage("Available msg",
                    new MessageOf<MaxFlowMsgData>(AVAILABLE_MSG_ID,
                                                  MaxFlowMsgData(fromMMPosition, toMMPosition)),
                    interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    if (mainPathState == BFS) {
        if (!mainPathsOld.empty()) {
            Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(fromMMPosition);
            deficit++;
            sendMessage("BFS msg",
                        new MessageOf<BFSdata>(
                            BFS_MSG_ID, BFSdata(mainPathsOld.back(), MMPosition, fromMMPosition)),
                        interfaceTo(MMPosition, fromMMPosition), 100, 200);
        }
    } else if (aug1PathState == BFS and fromMMPosition == mainPathIn) {
        if (!aug1PathsOld.empty()) {
            Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(fromMMPosition);
            deficit++;
            sendMessage("BFS msg",
                        new MessageOf<BFSdata>(
                            BFS_MSG_ID, BFSdata(aug1PathsOld.back(), MMPosition, fromMMPosition)),
                        interfaceTo(MMPosition, fromMMPosition), 100, 200);
        }
    } else if (aug2PathState == BFS and fromMMPosition == mainPathOut) {
        if (!aug2PathsOld.empty()) {
            Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(fromMMPosition);
            deficit++;
            sendMessage("BFS msg",
                        new MessageOf<BFSdata>(
                            BFS_MSG_ID, BFSdata(aug2PathsOld.back(), MMPosition, fromMMPosition)),
                        interfaceTo(MMPosition, fromMMPosition), 100, 200);
        }
    } else {
        console << MMPosition << ": path can no longer be augmented"
                << "\n";
    }
    state = PASSIVE;
}

void MetaModuleBlockCode::handleCutOffMessage(std::shared_ptr<Message> _msg,
                                               P2PNetworkInterface* sender) {
    MessageOf<MaxFlowMsgData>* msg = static_cast<MessageOf<MaxFlowMsgData>*>(_msg.get());
    Cell3DPosition fromMMPosition = (*msg->getData()).fromMMPosition;
    Cell3DPosition toMMPosition = (*msg->getData()).toMMPosition;
    console << "Rec. CutOff msg <" << toMMPosition << ", " << fromMMPosition << "> from "
            << sender->getConnectedBlockId() << "\n";
    bool isMainPathRemoved = false;
    Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(toMMPosition);
    if (module->position != toSeedPosition) {
        sendMessage("CutOff msg",
                    new MessageOf<MaxFlowMsgData>(CUTOFF_MSG_ID,
                                                  MaxFlowMsgData(fromMMPosition, toMMPosition)),
                    interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    console << "mainPathState: " << mainPathState << "\n";
    console << "aug1PathState: " << aug1PathState << "\n";
    console << "aug2PathState: " << aug2PathState << "\n";

    if(mainPathState != NONE and fromMMPosition == mainPathIn) {
        for (auto out : mainPathOut) {
//            VS_ASSERT(mainPathOut.size() == 1); 
            Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(out);
            console << out;
            deficit++;
            sendMessage(
                "CutOff msg1: ",
                new MessageOf<MaxFlowMsgData>(CUTOFF_MSG_ID, MaxFlowMsgData(MMPosition, out)),
                interfaceTo(MMPosition, out), 100, 200);
        }
        mainPathState = NONE;
        mainPathOut.clear();
        mainPathIn.set(-1,-1,-1);
        isMainPathRemoved = true;
    } 
        console << "isMainPathRemoved: " << isMainPathRemoved << "\n";
    if(aug1PathState != NONE and (fromMMPosition == aug1PathIn or isMainPathRemoved)) {
        // 
        // bool test = false;
        // if(aug2PathState == ConfPath) {
        //     VS_ASSERT(false);
        //     aug2PathOut = aug1PathOut;
        //     test = true;
            
        // }
        if(aug1PathOut != aug2PathOut) {
            for (auto out : aug1PathOut) {
                deficit++;
                Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(out);
                sendMessage(
                    "CutOff msg2" ,
                    new MessageOf<MaxFlowMsgData>(CUTOFF_MSG_ID, MaxFlowMsgData(MMPosition, out)),
                    interfaceTo(MMPosition, out), 100, 200);
            }
        }
         
        aug1PathState = NONE;
        aug1PathOut.clear();
        aug1PathIn.set(-1,-1,-1);
       
    }   
    if(aug2PathState != NONE and (fromMMPosition == aug2PathIn or isMainPathRemoved)) {
        for (auto out : aug2PathOut) {
            deficit++;
            Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(out);
            sendMessage(
                "CutOff msg",
                new MessageOf<MaxFlowMsgData>(CUTOFF_MSG_ID, MaxFlowMsgData(MMPosition, out)),
                interfaceTo(MMPosition, out), 100, 200);
        }
        aug2PathState = NONE;
        aug2PathOut.clear();
        aug2PathIn.set(-1,-1,-1);
    }
    for (auto p : getAdjacentMMSeeds()) {
        Cell3DPosition seedMMPosition =
            static_cast<MetaModuleBlockCode*>(
                BaseSimulator::getWorld()->getBlockByPosition(p)->blockCode)
                ->MMPosition;
        console << "Send Around Available: " << seedMMPosition << "\n";
        deficit++;
        sendMessage("Available msg",
                    new MessageOf<MaxFlowMsgData>(AVAILABLE_MSG_ID,
                                                  MaxFlowMsgData(MMPosition, seedMMPosition)),
                    interfaceTo(MMPosition, seedMMPosition), 100, 200);
    }
    console << "mainPathState: " << mainPathState << "\n";
    console << "aug1PathState: " << aug1PathState << "\n";
    console << "aug2PathState: " << aug2PathState << "\n";
    state = PASSIVE;
}

vector<Cell3DPosition> MetaModuleBlockCode::getAdjacentMMSeeds() {
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
bool MetaModuleBlockCode::setGreenLight(bool onoff) {
    stringstream info;
    info << " light turned ";

    if (onoff) {
        info << "green: ";
        greenLightIsOn = true;
        module->setColor(initialColor);

        // Resume flow if needed
        if (moduleAwaitingGo) {
           
            console << "awaitingModulePosition = " << awaitingModulePos << "\n";
            // if (module->getState() == BuildingBlock::State::ACTUATING or
            //     (awaitingModulePos == seedPosition.offsetX(2) and
            //      lattice->cellHasBlock(seedPosition + Cell3DPosition(2, -1, 1)))) {
            //          VS_ASSERT(false);
            //     return false;
            // }
            //  if(module->blockId == 63) {
            //     VS_ASSERT(false);
            // }
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

bool MetaModuleBlockCode::isPotentialDestination() {
    //Check if an adjacent position is in target shape
    for (auto adjPos : getAdjacentMMPositions()) {
        if (inTargetShape(adjPos) and not inInitialShape(adjPos) and not
            lattice->cellHasBlock(getSeedPositionFromMMPosition(adjPos))) {
            destinationOut = adjPos;
            return true;
        }
    }
    return false;
}

bool MetaModuleBlockCode::isPotentialSource() {
    if(inInitialShape(MMPosition) and not inTargetShape(MMPosition)) {
        return true;
    } else {
        return false;
    }
}

bool MetaModuleBlockCode::inTargetShape(Cell3DPosition pos) {
    for (auto targetPos : targetMap) {
        Cell3DPosition tp = Cell3DPosition(targetPos[0], targetPos[1], targetPos[2]);
        if (tp == pos) {
            return true;
        }
    }
    return false;
}

bool MetaModuleBlockCode::inInitialShape(Cell3DPosition pos) {
    for (auto initialPos : initialMap) {
        Cell3DPosition ip = Cell3DPosition(initialPos[0], initialPos[1], initialPos[2]);
        if (ip == pos) {
            return true;
        }
    }
    return false;
}

vector<Cell3DPosition> MetaModuleBlockCode::getAdjacentMMPositions() {
    vector<Cell3DPosition> adjacentPos;
    const vector<Cell3DPosition> relativePositions = {
        Cell3DPosition(1, 0, 0),  Cell3DPosition(-1, 0, 0), Cell3DPosition(0, 1, 0),
        Cell3DPosition(0, -1, 0), Cell3DPosition(0, 0, 1),  Cell3DPosition(0, 0, -1)};
    for(auto pos: relativePositions) {
        adjacentPos.push_back(MMPosition + pos);
    }
    return adjacentPos;
}


void MetaModuleBlockCode::updateState() {
    console << "Update State!!\n";
    operation->updateState(this);
    if(not operation->isTransfer() and not operation->isDismantle()) {
        operation =new Operation();
    }
   
    mvt_it = 0;
    isCoordinator = false;
    module->setColor(initialColor);
    if(not greenLightIsOn) {
        setGreenLight(true);
    }
}

bool MetaModuleBlockCode::isInMM(Cell3DPosition& neighborPosition) {
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
        int d;
        //if(reconfigurationStep == TRANSPORT)
            d = BaseSimulator::getWorld()->lattice->getCellDistance(neighPos, targetPosition);
        // else
        //     d = Init::getDistance(neighPos, targetPosition); 
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

Cell3DPosition MetaModuleBlockCode::getSeedPositionFromMMPosition(Cell3DPosition &MMPos) {
    Cell3DPosition seedPos = Cell3DPosition();
    seedPos.set(seed->position.pt[0] + (MMPos.pt[0] * 4), seed->position.pt[1] + (MMPos.pt[1] * 3),
                seed->position.pt[2] + (MMPos.pt[2] * 4));
    if (MMPos.pt[2] % 2 != 0) 
        seedPos =  seedPos.offsetY(-1);
    return seedPos;
}

P2PNetworkInterface * MetaModuleBlockCode::interfaceTo(Cell3DPosition& dstPos, P2PNetworkInterface *sender) {
    if(sender)
        return module->getInterface(nearestPositionTo(dstPos, sender));
    else 
        return module->getInterface(nearestPositionTo(dstPos));
}

P2PNetworkInterface * MetaModuleBlockCode::interfaceTo(Cell3DPosition& fromMM, Cell3DPosition& toMM, P2PNetworkInterface *sender) {
    Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(toMM);
    if(not lattice->cellHasBlock(toSeedPosition)) {
        VS_ASSERT_MSG(false, "not connected");
    }
    if(lattice->cellsAreAdjacent(module->position, toSeedPosition))
        return module->getInterface(toSeedPosition);
    Cell3DPosition fromSeedPosition = getSeedPositionFromMMPosition(fromMM);
    Direction direction;
    Cell3DPosition directionVector = toMM-fromMM;
    if (directionVector.pt[0] == -1) direction = Direction::LEFT;
    if (directionVector.pt[0] == 1) direction = Direction::RIGHT;
    if (directionVector.pt[1] == -1) direction = Direction::FRONT;
    if (directionVector.pt[1] == 1) direction = Direction::BACK;
    if (directionVector.pt[2] == -1) direction = Direction::DOWN;
    if (directionVector.pt[2] == 1) direction = Direction::UP;
    MetaModuleBlockCode *fromSeed = static_cast<MetaModuleBlockCode*>(BaseSimulator::getWorld()->getBlockByPosition(fromSeedPosition)->blockCode);
    vector<Cell3DPosition> routingVector = getRoutingVector(direction, fromSeed->shapeState, fromSeed->MMPosition.pt[2]);
    Cell3DPosition relativePosition = module->position - fromSeedPosition;
    vector<Cell3DPosition>::iterator it = find(routingVector.begin(), routingVector.end(), relativePosition);
    Cell3DPosition nextPos;
    if(it != routingVector.end()) {
        nextPos = *(it+1);
    } else {
        VS_ASSERT(false);
    }
    return module->getInterface(nextPos + fromSeedPosition);

}

void MetaModuleBlockCode::onMotionEnd() {
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
            sendMessage(
                "CoordinateBack Msg",
                new MessageOf<CoordinateBack>(COORDINATEBACK_MSG_ID,
                                              CoordinateBack(movingSteps, coordinatorPosition)),
                module->getInterface(nearestPositionTo(coordinatorPosition)), 100, 200);
        }
        console << "coordinator position: " << coordinatorPosition << "\n";
        movingSteps = 0;
        P2PNetworkInterface* pivotItf = module->getInterface(pivotPosition);
        if (operation->isFill() or operation->isBuild()) {
            sendMessageToAllNeighbors("FTR msg",
                                      new MessageOf<Cell3DPosition>(FTR_MSG_ID, module->position),
                                      100, 200, 0);
        } else if (pivotItf and pivotItf->isConnected()) {
            sendMessage("FTR msg", new MessageOf<Cell3DPosition>(FTR_MSG_ID, module->position),
                        module->getInterface(pivotPosition), 100, 200);
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
                    MetaModuleBlockCode* seedMM = static_cast<MetaModuleBlockCode*>(seed->blockCode);
                    reconfigurationStep = SRCDEST;
                    seedMM->nbWaitedAnswers = 0;
                    seedMM->distance = 0;
                    for (auto p : seedMM->getAdjacentMMSeeds()) {
                        Cell3DPosition toMMPosition =
                            static_cast<MetaModuleBlockCode*>(
                                BaseSimulator::getWorld()->getBlockByPosition(p)->blockCode)
                                ->MMPosition;
                        seedMM->sendMessage(
                            "Go msg",
                            new MessageOf<GOdata>(GO_MSG_ID, GOdata(seedMM->MMPosition,toMMPosition, seedMM->distance)),
                            seedMM->interfaceTo(seedMM->MMPosition, toMMPosition), 100, 200);
                        seedMM->nbWaitedAnswers++;
                    }
                    
                }
            }
            updateState();
        }
    }
}

void MetaModuleBlockCode::processLocalEvent(EventPtr pev) {
    std::shared_ptr<Message> message;
    stringstream info;
    switch (pev->eventType) {
        case EVENT_RECEIVE_MESSAGE: {
            message =
                (std::static_pointer_cast<NetworkInterfaceReceiveEvent>(pev))->message;
            if(message->type >= 1008 and message->type <= 1013 and module->position == seedPosition) {
                state = ACTIVE;
                cont_passive = false;
                Cell3DPosition fromMMPosition;
                if(message->type == 1008) {
                    MessageOf<BFSdata>* msg = static_cast<MessageOf<BFSdata>*>(message.get());
                    fromMMPosition = msg->getData()->fromMMPosition;
                } else {
                    MessageOf<MaxFlowMsgData>* msg = static_cast<MessageOf<MaxFlowMsgData>*>(message.get());
                    fromMMPosition = msg->getData()->fromMMPosition;
                }
                Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(fromMMPosition);
                sendMessage(
                    "Ack Msg",
                    new MessageOf<FromToMsg>(ACK_MSG_ID, FromToMsg(MMPosition, fromMMPosition)),
                    interfaceTo(MMPosition, fromMMPosition), 100, 200);
            }
        } break;
    }
    // Do not remove line below
    
    Catoms3DBlockCode::processLocalEvent(pev);
    if(not initialized) {
        return;
    } 
    switch (pev->eventType) {
        // case EVENT_RECEIVE_MESSAGE: {
        //     message =
        //         (std::static_pointer_cast<NetworkInterfaceReceiveEvent>(pev))->message;
        //     if(message->type >= 1008 and message->type <= 1013 and module->position == seedPosition) {
        //         state = ACTIVE;
        //         cont_passive = false;
        //         Cell3DPosition fromMMPosition;
        //         if(message->type == 1008) {
        //             MessageOf<BFSdata>* msg = static_cast<MessageOf<BFSdata>*>(message.get());
        //             fromMMPosition = msg->getData()->fromMMPosition;
        //         } else {
        //             MessageOf<MaxFlowMsgData>* msg = static_cast<MessageOf<MaxFlowMsgData>*>(message.get());
        //             fromMMPosition = msg->getData()->fromMMPosition;
        //         }
        //         Cell3DPosition toSeedPosition = getSeedPositionFromMMPosition(fromMMPosition);
        //         sendMessage("Ack Msg", new MessageOf<Cell3DPosition>(ACK_MSG_ID, fromMMPosition), interfaceTo(toSeedPosition), 100, 200);
        //     }
        // } break;

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
            if (isCoordinator and pos == module->position + Cell3DPosition(0, 1, 1)) {
                if ((not operation->isTransfer() and not operation->isFill() and
                     operation->getDirection() != Direction::UP and not operation->isBuild()) or
                    (operation->isTransfer() and operation->getDirection() == Direction::UP and
                     static_cast<Transfer_Operation*>(operation)->isComingFromBack()) or
                    (operation->isBuild() and operation->getDirection() == Direction::BACK and
                     operation->getMMShape() == BACKFRONT and not
                        static_cast<Build_Operation*>(operation)->isComingFromBack())) {
                    if (posBlock->module->canMoveTo(module->position.offsetY(1)) and not posBlock->sendingCoordinateBack) {
                            console << "move pos\n";
                        posBlock->module->moveTo(module->position.offsetY(1));
                    } else {
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
                    getScheduler()->trace("TransferBack", module->blockId, RED);
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
                    getScheduler()->trace("ReachCoordinator", module->blockId, RED);
                    if (module->canMoveTo(module->position.offsetZ(-1)) and not sendingCoordinateBack) {
                        module->moveTo(module->position.offsetZ(-1));
                    } else {
                        console << "Trying to reach coordinator\n";
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
                            sendMessage("BackTerm Msg4",
                                        new MessageOf<pair<FromToMsg, bool>>(BACKTERM_MSG_ID,
                                                                                make_pair(FromToMsg(MMPosition, parentPosition), res and b)),
                                        interfaceTo(MMPosition, parentPosition), 100, 200);
                        } else {
                            start_wave();
                            //VS_ASSERT(false);
                        }

                    } else {
                        getScheduler()->schedule(
                            new InterruptionEvent(getScheduler()->now() + 500, module, IT_MODE_TERMINATION));
                    }
                } break;
                case IT_MODE_DISMANTLEBACK: {
                    if(lattice->cellHasBlock(seedPosition + Cell3DPosition(1,3,2))) {
                        getScheduler()->schedule(new InterruptionEvent(getScheduler()->now() + getRoundDuration(),
                                                       module, IT_MODE_DISMANTLEBACK));
                    } else {
                        probeGreenLight();
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
    cerr << "parentPosition: " << parentPosition << endl;
    cerr << "childrenPostions: ";
    for(auto c: childrenPositions) cerr << c << "; ";
    cerr << endl;
    cerr << "distance: " << distance << endl;
    cerr << "mainPathState: " << mainPathState << endl;
    // cerr << "aug1PathState: " << aug1PathState << endl;
    // cerr << "aug2PathState: " << aug2PathState << endl;
    cerr << "mainPathIn: " << mainPathIn << endl;
    cerr << "mainPathOut: ";
    for(auto out: mainPathOut) cerr << out << " | ";
    cerr << endl;
    cerr << "aug1PathIn: " << aug1PathIn << endl;
    cerr << "aug1PathOut: ";
    for(auto out: aug1PathOut) cerr << out << " | ";
    cerr << endl;
    cerr << "aug2PathIn: " << aug2PathIn << endl;
    cerr << "aug2PathOut: ";
    for(auto out: aug2PathOut) cerr << out << " | ";
    cerr << endl;
    cerr << "mainPathsOld: ";
    for(auto old: mainPathsOld) cerr << old << " | ";
    cerr << endl;
    cerr << "aug1PathsOld: ";
    for(auto old: aug1PathsOld) cerr << old << " | ";
    cerr << endl;
    cerr << "aug2PathsOld: ";
    for(auto old: aug2PathsOld) cerr << old << " | ";
    cerr << endl;
    cerr << "deficit: " << deficit << endl;
    cerr << "state: " << state << endl;
}

void MetaModuleBlockCode::onUserKeyPressed(unsigned char c, int x, int y) {
    if(c == 'C' and Init::initialMapBuildDone) {
        showSrcAndDst = not showSrcAndDst;
        // color sources in RED, destinations in GREEN in other MMs in White
        for(auto id_block: BaseSimulator::getWorld()->buildingBlocksMap) {
            MetaModuleBlockCode* block = static_cast<MetaModuleBlockCode*>(id_block.second->blockCode);
            if(showSrcAndDst) {
                if(block->isPotentialSource()) {
                    block->module->setColor(RED);
                } else if(block->isPotentialDestination()) {
                    block->getModule()->setColor(GREEN);
                } else {
                    block->getModule()->setColor(WHITE);
                }
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
    MetaModuleBlockCode* block = static_cast<MetaModuleBlockCode*>(
        BaseSimulator::getWorld()->getBlockById(blockId)->blockCode
    );
    
    ofstream file;
    if(c == 'J') {
        seedPosition = Cell3DPosition(20,19,14);
        file.open("routes.txt", ios::out | ios::app);
        if(!file.is_open()) return;
        file << "Cell3DPosition" <<  block->module->position - block->seedPosition << ", ";
        return;
    }
    file.open("BF_Dismantle_Left.txt", ios::out | ios::app);
    seedPosition = Cell3DPosition(32,19,6);
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

string MetaModuleBlockCode::onInterfaceDraw() {
    stringstream trace;
    trace << "Nb of modules " << BaseSimulator::getWorld()->getNbBlocks();
    return trace.str();
}

#include "diameterMonitoringBlockCode.hpp"

using namespace BlinkyBlocks;

DiameterMonitoringBlockCode::DiameterMonitoringBlockCode(BlinkyBlocksBlock *host) : BlinkyBlocksBlockCode(host) {
    // @warning Do not remove block below, as a blockcode with a NULL host might be created
    //  for command line parsing
    if (not host) return;

    // Registers a callback (handleSampleMessage) to the message of type SAMPLE_MSG_ID
    addMessageEventFunc2(BFSGO_MSG_ID,
                         std::bind(&DiameterMonitoringBlockCode::handleBfsGoMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(BFSBACK_MSG_ID,
                         std::bind(&DiameterMonitoringBlockCode::handleBfsBackMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(FARTHEST_MSG_ID,
                         std::bind(&DiameterMonitoringBlockCode::handleFarthestMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(NOTIFYDIAMETER_MSG_ID,
                         std::bind(&DiameterMonitoringBlockCode::handleNotifyDiameterMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(ADDEDNEIGHBOR_MSG_ID,
                         std::bind(&DiameterMonitoringBlockCode::handleAddedNeighborMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    // Set the module pointer
    module = static_cast<BlinkyBlocksBlock*>(hostBlock);
  }

void DiameterMonitoringBlockCode::startup() {
    console << "start";
    // Sample distance coloring algorithm below
    parent = nullptr;
    if (module->blockId == 1) { // Master ID is 1
        module->setColor(RED);
        distance = 0;
        round = 1;
        nbWaitedAnswers = sendMessageToAllNeighbors("Sample Broadcast",
                                  new MessageOf<GOBFSMessageData>(BFSGO_MSG_ID,GOBFSMessageData(distance, round)),100,200,0);
    } else {
        distance = -1; // Unknown distance
        hostBlock->setColor(LIGHTGREY);
    }

    // Additional initialization and algorithm start below
    // ...
}

void DiameterMonitoringBlockCode::handleBfsGoMessage(std::shared_ptr<Message> _msg,
                                               P2PNetworkInterface* sender) {
    MessageOf<GOBFSMessageData>* msg = static_cast<MessageOf<GOBFSMessageData>*>(_msg.get());

    int d = msg->getData()->distance + 1;
    round = msg->getData()->round;
    console << " received d =" << d << " from " << sender->getConnectedBlockId() << "\n";
                                    
    if (parent == nullptr || distance > d) {
        console << " updated distance = " << d << "\n";
        distance = d;
        maxDownDistance  = distance;
        parent = sender;
        //module->setColor(Colors[distance % NB_COLORS]);
        if(round == 2) du = distance;
        else if (round == 3) dv = distance;
        // Broadcast to all neighbors but ignore sender
        nbWaitedAnswers = sendMessageToAllNeighbors("BFSGO Broadcast",
                                  new MessageOf<GOBFSMessageData>(BFSGO_MSG_ID,GOBFSMessageData(distance, round)),100,200,1,sender);
        if (nbWaitedAnswers == 0) {
            // Leaf node, send back immediately
            maxDownDistance = distance;
            console << " leaf node, sending back maxDownDistance = " << maxDownDistance << "\n";
            sendMessage("BFSBACK",
                                      new MessageOf<int>(BFSBACK_MSG_ID,maxDownDistance), parent, 100,200);
            parent = nullptr;
            maxDownDistance = 0;
            distance = 0;
        }
    } else {
        sendMessage("BFSBACK",
                                  new MessageOf<int>(BFSBACK_MSG_ID,-1), sender, 100,200);
    }
}

void DiameterMonitoringBlockCode::handleBfsBackMessage(std::shared_ptr<Message> _msg,
                                               P2PNetworkInterface* sender) {
    MessageOf<int>* msg = static_cast<MessageOf<int>*>(_msg.get());

    int d = *msg->getData();
    console << " received BFSBACK d =" << d << " from " << sender->getConnectedBlockId() << "\n";
    if( d > maxDownDistance) {
        maxDownDistance = d;
        interfaceToFarthest = sender;
    }
    nbWaitedAnswers--;
    if (nbWaitedAnswers == 0) {
        if (parent == nullptr) {
            console << " BFS complete at master " << module->blockId << ", diameter is " << maxDownDistance << "\n";
            sendMessage("Farthest",
                                      new MessageOf<int>(FARTHEST_MSG_ID, D), interfaceToFarthest, 100,200);
            interfaceToFarthest = nullptr;
        } else {
            // Send back to parent
            sendMessage("BFSBACK",
                                      new MessageOf<int>(BFSBACK_MSG_ID, maxDownDistance), parent, 100,200);
        }
        parent = nullptr;
        maxDownDistance = 0;
        distance = 0;
    }
}

void DiameterMonitoringBlockCode::handleFarthestMessage(std::shared_ptr<Message> _msg,
                                               P2PNetworkInterface* sender) {
    if(interfaceToFarthest != nullptr) {
        console << " received FARTHEST at " << module->blockId << "\n";
        setColor(CYAN);
        sendMessage("Farthest",
                                  new MessageOf<int>(FARTHEST_MSG_ID, D), interfaceToFarthest, 100,200);
        interfaceToFarthest = nullptr;
    } else {
    
        console << " Diameter monitoring complete at leaf " << module->blockId << " round " << round << "\n";
        round++;
        if(round <= 3){
            //getScheduler()->toggle_pause();
            if(round == 3) {
                dv = distance;
                module->setColor(YELLOW);
            } else if (round ==2) {
                du = distance;
                module->setColor(GREEN);
            }
            nbWaitedAnswers = sendMessageToAllNeighbors("GOMSG round",
                                    new MessageOf<GOBFSMessageData>(BFSGO_MSG_ID,GOBFSMessageData(distance, round)),100,200, 0);
        } else {
            D = dv;
            du = 0;
            cerr << " Diameter monitoring fully complete at leaf " << module->blockId << " with diameter " << D << "\n";
            sendMessageToAllNeighbors("NOTIFYDIAMETER",
                                      new MessageOf<NotifyDiameterMessageData>(NOTIFYDIAMETER_MSG_ID, NotifyDiameterMessageData(D)),100,200, 0);
        }
    }
}

void DiameterMonitoringBlockCode::handleAddedNeighborMessage(std::shared_ptr<Message> _msg,
                                               P2PNetworkInterface* sender) {
    // Handle neighbor addition if needed
    MessageOf<AddedNeighborMessageData>* msg = static_cast<MessageOf<AddedNeighborMessageData>*>(_msg.get());
    AddedNeighborMessageData data = *msg->getData();
    setColor(RED);
    nbAddedNeighborsReceived++;
    if(data.du > maxDu) maxDu = data.du;
    if(data.dv > maxDv) maxDv = data.dv;
    if(nbAddedNeighborsReceived == module->getNbNeighbors()) {
        du = maxDu + 1;
        dv = maxDv + 1;
        D = data.D;
        console << " All neighbors added, computed du=" << du << ", dv=" << dv << ", D=" << D << "\n";
        nbAddedNeighborsReceived = 0;
        maxDu = 0;
        maxDv = 0;
        if (du < D && dv < D) cerr << "STABILITY\n"; 
        if(du > D) {
            D = du;
            dv = 0;
            sendMessageToAllNeighbors("NOTIFYDIAMETER",
                                      new MessageOf<NotifyDiameterMessageData>(NOTIFYDIAMETER_MSG_ID, NotifyDiameterMessageData(D, du, -1)),100,200, 0);
        } else if (dv > D) {
            D = dv;
            du = 0;
            // TODO also update dv
            sendMessageToAllNeighbors("NOTIFYDIAMETER",
                                      new MessageOf<NotifyDiameterMessageData>(NOTIFYDIAMETER_MSG_ID, NotifyDiameterMessageData(D, -1, dv)),100,200, 0);
        }
    }
}

void DiameterMonitoringBlockCode::handleNotifyDiameterMessage(std::shared_ptr<Message> _msg,
                                               P2PNetworkInterface* sender) {
    MessageOf<NotifyDiameterMessageData>* msg = static_cast<MessageOf<NotifyDiameterMessageData>*>(_msg.get());
    NotifyDiameterMessageData data = *msg->getData();
    int d = data.D;
    if(D == -1 or d > D) {
        D = d;
        if(data.dv != -1) du++;
        if(data.du != -1) dv++;
        console << " received NOTIFYDIAMETER d =" << d << " from " << sender->getConnectedBlockId() << "\n";
        sendMessageToAllNeighbors("NOTIFYDIAMETER",
                                  new MessageOf<NotifyDiameterMessageData>(NOTIFYDIAMETER_MSG_ID, NotifyDiameterMessageData(D)),100,200, 1,sender);
        setColor(WHITE);
        if(dv == 0) setColor(YELLOW);
    }
}

void DiameterMonitoringBlockCode::onMotionEnd() {
    console << " has reached its destination" << "\n";

    // do stuff
    // ...
}

void DiameterMonitoringBlockCode::processLocalEvent(EventPtr pev) {
    std::shared_ptr<Message> message;
    stringstream info;

    // Do not remove line below
    BlinkyBlocksBlockCode::processLocalEvent(pev);

    switch (pev->eventType) {
        case EVENT_ADD_NEIGHBOR: {
            if(D == -1) break;
            // Do something when a neighbor is added to an interface of the module
            console << " New neighbor added, sending ADDEDNEIGHBOR message\n";
            uint64_t face = BaseSimulator::getWorld()->lattice->getOppositeDirection((std::static_pointer_cast<AddNeighborEvent>(pev))->face);

            sendMessage("AddedNeighbor",
                        new MessageOf<AddedNeighborMessageData>(ADDEDNEIGHBOR_MSG_ID,
                                                      AddedNeighborMessageData(du, dv, D)),
                        module->getInterface(face), 100, 200);
            break;
        }

        case EVENT_REMOVE_NEIGHBOR: {
            // Do something when a neighbor is removed from an interface of the module
            break;
        }
    }
}

/// ADVANCED BLOCKCODE FUNCTIONS BELOW

void DiameterMonitoringBlockCode::onBlockSelected() {
    // Debug stuff:
    cerr << endl << "--- PRINT MODULE " << *module << "---" << endl;
    cerr << " Distance from master: " << distance << endl;
    cerr << "D: " << D << ", du: " << du << ", dv: " << dv << endl;
}

void DiameterMonitoringBlockCode::onAssertTriggered() {
    console << " has triggered an assert" << "\n";

    // Print debugging some info if needed below
    // ...
}

bool DiameterMonitoringBlockCode::parseUserCommandLineArgument(int &argc, char **argv[]) {
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

string DiameterMonitoringBlockCode::onInterfaceDraw() {
    stringstream trace;
    trace << "Some value " << 123;
    return trace.str();
}

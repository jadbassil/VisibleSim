#include "diameterMonitoringBlockCode.hpp"

using namespace BlinkyBlocks;

DiameterMonitoringBlockCode::DiameterMonitoringBlockCode(BlinkyBlocksBlock* host)
    : BlinkyBlocksBlockCode(host) {
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
    addMessageEventFunc2(INFORMDUDV_MSG_ID,
                         std::bind(&DiameterMonitoringBlockCode::handleInformDuDvMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    // Set the module pointer
    module = static_cast<BlinkyBlocksBlock*>(hostBlock);
}

int DiameterMonitoringBlockCode::DIAMETER = -1;

int DiameterMonitoringBlockCode::calculateRealConfigurationDiameter() {
    // Optimized diameter calculation: For each block, do ONE BFS to find distances to all other blocks
    // The diameter is the maximum eccentricity. This is O(N²) instead of O(N³) by avoiding repeated BFS calls
    unsigned int diameter = 0;
    auto* world = BaseSimulator::getWorld();
    auto* lattice = world->lattice;
    auto& blocksMap = world->buildingBlocksMap;
    
    // For each block, do ONE BFS to find distances to all other blocks
    for (auto &startBlock : blocksMap) {
        const Cell3DPosition& startPos = startBlock.second->position;
        
        // BFS to find all distances from this block
        std::queue<Cell3DPosition> toVisit;
        std::map<Cell3DPosition, unsigned int> distances;
        toVisit.push(startPos);
        distances[startPos] = 0;
        
        unsigned int maxDistance = 0;
        
        while (!toVisit.empty()) {
            Cell3DPosition current = toVisit.front();
            toVisit.pop();
            unsigned int currentDistance = distances[current];
            maxDistance = std::max(maxDistance, currentDistance);
            
            for (const Cell3DPosition &neighbor : lattice->getActiveNeighborCells(current)) {
                if (distances.find(neighbor) == distances.end() && 
                    lattice->cellHasBlock(neighbor) && 
                    lattice->isInGrid(neighbor)) {
                    distances[neighbor] = currentDistance + 1;
                    toVisit.push(neighbor);
                }
            }
        }
        
        diameter = std::max(diameter, maxDistance);
    }
    return (int) diameter;
}


void DiameterMonitoringBlockCode::startup() {
    console << "start";
    // Sample distance coloring algorithm below
    parent = nullptr;
    if (module->blockId == 1) {  // Master ID is 1
        module->setColor(RED);
        distance = 0;
        round = 1;
        nbWaitedAnswers = sendMessageToAllNeighbors(
            "Sample Broadcast",
            new MessageOf<GOBFSMessageData>(BFSGO_MSG_ID, GOBFSMessageData(distance, round)), 100,
            200, 0);
    } else {
        distance = -1;  // Unknown distance
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
        maxDownDistance = distance;
        parent = sender;
        // module->setColor(Colors[distance % NB_COLORS]);
        if (round == 2)
            du = distance;
        else if (round == 3) {
            dv = distance;
            D = -1;
        }
        // Broadcast to all neighbors but ignore sender
        nbWaitedAnswers = sendMessageToAllNeighbors(
            "BFSGO Broadcast",
            new MessageOf<GOBFSMessageData>(BFSGO_MSG_ID, GOBFSMessageData(distance, round)), 100,
            200, 1, sender);
        if (nbWaitedAnswers == 0) {
            // Leaf node, send back immediately
            maxDownDistance = distance;
            console << " leaf node, sending back maxDownDistance = " << maxDownDistance << "\n";
            sendMessage("BFSBACK", new MessageOf<int>(BFSBACK_MSG_ID, maxDownDistance), parent, 100,
                        200);
            parent = nullptr;
            maxDownDistance = 0;
            distance = 0;
        }
    } else {
        sendMessage("BFSBACK", new MessageOf<int>(BFSBACK_MSG_ID, -1), sender, 100, 200);
    }
}

void DiameterMonitoringBlockCode::handleBfsBackMessage(std::shared_ptr<Message> _msg,
                                                       P2PNetworkInterface* sender) {
    MessageOf<int>* msg = static_cast<MessageOf<int>*>(_msg.get());

    int d = *msg->getData();
    console << " received BFSBACK d =" << d << " from " << sender->getConnectedBlockId() << "\n";
    if (d > maxDownDistance) {
        maxDownDistance = d;
        interfaceToFarthest = sender;
    }
    nbWaitedAnswers--;
    if (nbWaitedAnswers == 0) {
        if (parent == nullptr) {
            console << " BFS complete at master " << module->blockId << ", diameter is "
                    << maxDownDistance << "\n";
            sendMessage("Farthest", new MessageOf<int>(FARTHEST_MSG_ID, D), interfaceToFarthest,
                        100, 200);
            interfaceToFarthest = nullptr;
        } else {
            // Send back to parent
            sendMessage("BFSBACK", new MessageOf<int>(BFSBACK_MSG_ID, maxDownDistance), parent, 100,
                        200);
        }
        parent = nullptr;
        maxDownDistance = 0;
        distance = 0;
    }
}

void DiameterMonitoringBlockCode::handleFarthestMessage(std::shared_ptr<Message> _msg,
                                                        P2PNetworkInterface* sender) {
    if (interfaceToFarthest != nullptr) {
        console << " received FARTHEST at " << module->blockId << "\n";
        setColor(CYAN);
        sendMessage("Farthest", new MessageOf<int>(FARTHEST_MSG_ID, D), interfaceToFarthest, 100,
                    200);
        interfaceToFarthest = nullptr;
    } else {
        console << " Diameter monitoring complete at leaf " << module->blockId << " round " << round
                << "\n";
        round++;
        if (round <= 3) {
            // getScheduler()->toggle_pause();
            if (round == 3) {
                dv = distance;
                module->setColor(YELLOW);
            } else if (round == 2) {
                du = distance;
                module->setColor(GREEN);
            }
            nbWaitedAnswers = sendMessageToAllNeighbors(
                "GOMSG round",
                new MessageOf<GOBFSMessageData>(BFSGO_MSG_ID, GOBFSMessageData(distance, round)),
                100, 200, 0);
        } else {
            D = dv;
            du = 0;
            cerr << " Diameter monitoring fully complete at leaf " << module->blockId
                 << " with diameter " << D << "\n";
            cerr << "number of messages: " << StatsCollector::getInstance().getNbProcessedMessages() << "\n";
            cerr << "time: " << getScheduler()->now() << "\n";

            sendMessageToAllNeighbors("NOTIFYDIAMETER",
                                      new MessageOf<NotifyDiameterMessageData>(
                                          NOTIFYDIAMETER_MSG_ID, NotifyDiameterMessageData(D)),
                                      100, 200, 0);
        }
    }
}

void DiameterMonitoringBlockCode::handleInformDuDvMessage(std::shared_ptr<Message> _msg,
                                                          P2PNetworkInterface* sender) {
    MessageOf<InformDuDvMessageData>* msg =
        static_cast<MessageOf<InformDuDvMessageData>*>(_msg.get());
    InformDuDvMessageData data = *msg->getData();
    faceDuDvMap[module->getFaceForNeighborID(sender->getConnectedBlockId())] =
        make_pair(data.du, data.dv);
}

void DiameterMonitoringBlockCode::handleAddedNeighborMessage(std::shared_ptr<Message> _msg,
                                                             P2PNetworkInterface* sender) {
    // Handle neighbor addition if needed
    MessageOf<AddedNeighborMessageData>* msg =
        static_cast<MessageOf<AddedNeighborMessageData>*>(_msg.get());
    AddedNeighborMessageData data = *msg->getData();
    setColor(RED);
    nbAddedNeighborsReceived++;
    if (data.du < minDu) minDu = data.du;
    if (data.dv < minDv) minDv = data.dv;
    if (nbAddedNeighborsReceived == module->getNbNeighbors()) {
        du = minDu + 1;
        dv = minDv + 1;
        D = data.D;
        console << " All neighbors added, computed du=" << du << ", dv=" << dv << ", D=" << D
                << "\n";
        nbAddedNeighborsReceived = 0;
        minDu = 0;
        minDv = 0;

        if (du + dv < D) {
            // BRIDGE RULE must recalculate the diameter
            cerr << "BRIDGE RULE\n";
            round = 1;
            distance = 0;
            nbWaitedAnswers = sendMessageToAllNeighbors(
                "Sample Broadcast",
                new MessageOf<GOBFSMessageData>(BFSGO_MSG_ID, GOBFSMessageData(distance, round)),
                100, 200, 0);
        } else if (du < D and dv < D) {
            // STABILITY RULE
            cerr << " STABILITY RULE\n";
            cerr << " Diameter remains " << D << "\n";
            cerr << "number of messages: " << StatsCollector::getInstance().getNbProcessedMessages() << "\n";
            cerr << "time: " << getScheduler()->now() << "\n";
        } else {
            // GROWTH RULE
            if (du >= D) {
                D = du;
                dv = 0;
                cerr << " GROWTH RULE, new diameter " << D << "\n";
                sendMessageToAllNeighbors(
                    "NOTIFYDIAMETER",
                    new MessageOf<NotifyDiameterMessageData>(NOTIFYDIAMETER_MSG_ID,
                                                             NotifyDiameterMessageData(D, du, -1)),
                    100, 200, 0);
            } else if (dv >= D) {
                D = dv;
                du = 0;
                cerr << " GROWTH RULE, new diameter " << D << "\n";
                sendMessageToAllNeighbors(
                    "NOTIFYDIAMETER",
                    new MessageOf<NotifyDiameterMessageData>(NOTIFYDIAMETER_MSG_ID,
                                                             NotifyDiameterMessageData(D, -1, dv)),
                    100, 200, 0);
            }
        }
    }
}

void DiameterMonitoringBlockCode::handleNotifyDiameterMessage(std::shared_ptr<Message> _msg,
                                                              P2PNetworkInterface* sender) {
    MessageOf<NotifyDiameterMessageData>* msg =
        static_cast<MessageOf<NotifyDiameterMessageData>*>(_msg.get());
    NotifyDiameterMessageData data = *msg->getData();
    int d = data.D;
    if (D == -1 or d > D) {
        D = d;
        if (data.dv != -1) du++;
        if (data.du != -1) dv++;

        console << " received NOTIFYDIAMETER d =" << d << " from " << sender->getConnectedBlockId()
                << "\n";
        sendMessageToAllNeighbors(
            "INFORMDUDV",
            new MessageOf<InformDuDvMessageData>(INFORMDUDV_MSG_ID, InformDuDvMessageData(du, dv)),
            100, 200, 0);
        sendMessageToAllNeighbors(
            "NOTIFYDIAMETER", new MessageOf<NotifyDiameterMessageData>(NOTIFYDIAMETER_MSG_ID, data),
            100, 200, 1, sender);
        setColor(WHITE);
        if (dv == 0 or du == 0) {
            module->setColor(YELLOW);
            cerr << " Diameter monitoring complete at module " << module->blockId
                 << " with diameter " << D << "\n";
            cerr << "number of messages: " << StatsCollector::getInstance().getNbProcessedMessages() << "\n";
            cerr << "time: " << getScheduler()->now() << "\n";
            cerr << "RealDiameter: " << DiameterMonitoringBlockCode::calculateRealConfigurationDiameter() << "\n";
        };
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
            if (D == -1) break;
            // Do something when a neighbor is added to an interface of the module
            console << " New neighbor added, sending ADDEDNEIGHBOR message\n";
            uint64_t face = BaseSimulator::getWorld()->lattice->getOppositeDirection(
                (std::static_pointer_cast<AddNeighborEvent>(pev))->face);

            sendMessage("AddedNeighbor",
                        new MessageOf<AddedNeighborMessageData>(
                            ADDEDNEIGHBOR_MSG_ID, AddedNeighborMessageData(du, dv, D)),
                        module->getInterface(face), 100, 200);
            break;
        }
        case EVENT_REMOVE_NEIGHBOR: {
            // Do something when a neighbor is removed from an interface of the module
            if (D == -1) break;
            uint64_t face = BaseSimulator::getWorld()->lattice->getOppositeDirection(
                (std::static_pointer_cast<RemoveNeighborEvent>(pev))->face);

            console << " Neighbor removed, recalculating diameter\n";
            Cell3DPosition removedPos;
            module->getNeighborPos(face, removedPos);
            bool isMin = true;
            for (auto pos : lattice->getActiveNeighborCells(removedPos)) {
                if (lattice->getBlock(pos)->blockId < module->blockId) {
                    isMin = false;
                    break;
                }
            }
            if (!isMin)
                break;  // Only the module with the lowest ID among its neighbors handles the
                        // removal
            uint64_t oppositeFace = BaseSimulator::getWorld()->lattice->getOppositeDirection(face);
            int duRemoved = faceDuDvMap[face].first;
            int dvRemoved = faceDuDvMap[face].second;
            int duOpposite = faceDuDvMap[oppositeFace].first;
            int dvOpposite = faceDuDvMap[oppositeFace].second;
            console << " Removed neighbor had du=" << duRemoved << ", dv=" << dvRemoved << "\n";
            console << " Opposite face( " << module->getNeighborIDForFace(oppositeFace)
                    << " ) has du=" << duOpposite << ", dv=" << dvOpposite << "\n";
            bool duAffected = true;
            bool dvAffected = true;
            for (auto it = faceDuDvMap.begin(); it != faceDuDvMap.end(); ++it) {
                if (it->first == face) continue;
                if (it->second.first <= duRemoved) duAffected = false;
                if (it->second.second <= dvRemoved) dvAffected = false;
            }
            if (duAffected || dvAffected) {
                cerr << " Diameter needs to be recalculated due to neighbor removal\n";
                round = 1;
                distance = 0;
                nbWaitedAnswers =
                    sendMessageToAllNeighbors("Sample Broadcast",
                                              new MessageOf<GOBFSMessageData>(
                                                  BFSGO_MSG_ID, GOBFSMessageData(distance, round)),
                                              100, 200, 0);
            } else {
                console << " Diameter unaffected by neighbor removal\n";
                cerr << " Diameter remains " << D << "\n";
                cerr << "number of messages: " << getScheduler()->getNbreMessages() << "\n";
                cerr << "time: " << getScheduler()->now() << "\n";
            }
            break;
        }
        default:
            break;
    }
}

/// ADVANCED BLOCKCODE FUNCTIONS BELOW

void DiameterMonitoringBlockCode::onBlockSelected() {
    // Debug stuff:
    cerr << endl << "--- PRINT MODULE " << *module << "---" << endl;
    cerr << " Distance from master: " << distance << endl;
    cerr << "D: " << D << ", du: " << du << ", dv: " << dv << endl;
    // Print du,dv per face
    for (const auto& [face, duDv] : faceDuDvMap) {
        cerr << " Face " << face << ": du=" << duDv.first << ", dv=" << duDv.second << endl;
    }
    cerr << "------------------------" << endl << endl;
    DiameterMonitoringBlockCode::DIAMETER = calculateRealConfigurationDiameter();

}

void DiameterMonitoringBlockCode::onAssertTriggered() {
    console << " has triggered an assert" << "\n";

    // Print debugging some info if needed below
    // ...
}

bool DiameterMonitoringBlockCode::parseUserCommandLineArgument(int& argc, char** argv[]) {
    /* Reading the command line */
    if ((argc > 0) && ((*argv)[0][0] == '-')) {
        switch ((*argv)[0][1]) {
            // Single character example: -b
            case 'b': {
                cout << "-b option provided" << endl;
                return true;
            } break;

            // Composite argument example: --foo 13
            case '-': {
                string varg = string((*argv)[0] + 2);  // argv[0] without "--"
                if (varg == string("foo")) {           //
                    int fooArg;
                    try {
                        fooArg = stoi((*argv)[1]);
                        argc--;
                        (*argv)++;
                    } catch (std::logic_error&) {
                        stringstream err;
                        err << "foo must be an integer. Found foo = " << argv[1] << endl;
                        throw CLIParsingError(err.str());
                    }

                    cout << "--foo option provided with value: " << fooArg << endl;
                } else
                    return false;

                return true;
            }

            default:
                cerr << "Unrecognized command line argument: " << (*argv)[0] << endl;
        }
    }

    return false;
}

string DiameterMonitoringBlockCode::onInterfaceDraw() {
    stringstream trace;
    trace << "Real Diameter " << DiameterMonitoringBlockCode::DIAMETER;
    trace << " - Current D " << D;
    return trace.str();
}

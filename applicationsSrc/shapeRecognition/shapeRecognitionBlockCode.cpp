#include "shapeRecognitionBlockCode.hpp"

using namespace Catoms3D;

ShapeRecognitionBlockCode::ShapeRecognitionBlockCode(Catoms3DBlock *host) : Catoms3DBlockCode(host) {
    // @warning Do not remove block below, as a blockcode with a NULL host might be created
    //  for command line parsing
    if (not host) return;

    // Registers a callback (handleSampleMessage) to the message of type SAMPLE_MSG_ID
    addMessageEventFunc2(SETL_MSG_ID,
                         std::bind(&ShapeRecognitionBlockCode::handleSetLMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));

    addMessageEventFunc2(GETW_MSG_ID,
                         std::bind(&ShapeRecognitionBlockCode::handleGetWMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));

    addMessageEventFunc2(SETW_MSG_ID,
                         std::bind(&ShapeRecognitionBlockCode::handleSetWMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));

    addMessageEventFunc2(CHECKLR_MSG_ID,
                         std::bind(&ShapeRecognitionBlockCode::handleCheckLRMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));

    addMessageEventFunc2(SENDLR_MSG_ID,
                         std::bind(&ShapeRecognitionBlockCode::handleSendLRMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));

    addMessageEventFunc2(BROADCASTRECT_MSG_ID,
                         std::bind(&ShapeRecognitionBlockCode::handleBroadcastRectWMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    // Set the module pointer
    module = static_cast<Catoms3DBlock *>(hostBlock);
}

void ShapeRecognitionBlockCode::startup() {
    console << "start\n";
    l = -1;
    leftL = rightL = -1;
    if (isTop()) {
        module->setColor(RED);
        l = 1;
        if (lattice->cellHasBlock(module->position.offsetY(-1))) {
            sendMessage("SetL msg", new MessageOf<int>(SETL_MSG_ID, l),
                        module->getInterface(
                                lattice->getCellInDirection(module->position, FCCLattice::Direction::C7South)), 100, 0);
        }
        if (isBottom()) {
            console << "Bottom hole reached\n";
            module->setColor(GREEN);

            //search for the width
            if (lattice->cellHasBlock(module->position.offsetX(1))) {
                nbWaitedCheckLR++;
                sendMessage("CheckLR<right> msg", new Message(CHECKLR_MSG_ID),
                            module->getInterface(module->position.offsetX(1)), 100, 0);
            }
            if (lattice->cellHasBlock(module->position.offsetX(-1))) {
                nbWaitedCheckLR++;
                sendMessage("CheckLR<left> msg", new Message(CHECKLR_MSG_ID),
                            module->getInterface(module->position.offsetX(-1)), 100, 0);
            }
        }

    }
}

void ShapeRecognitionBlockCode::handleSetLMessage(std::shared_ptr<Message> _msg,
                                                  P2PNetworkInterface *sender) {
    console << "recv setL from " << sender->getConnectedBlockId() << "\n";

    auto *msg = dynamic_cast<MessageOf<int> *>(_msg.get());

    l = *msg->getData() + 1;
    while (!waiting.empty()) {
        sendMessage("SendLR msg", new MessageOf<int>(SENDLR_MSG_ID, l), waiting.front(), 100, 0);
        waiting.pop();
    }
    if (lattice->cellHasBlock(module->position.offsetY(-1))) {
        sendMessage("SetL msg", new MessageOf<int>(SETL_MSG_ID, l),
                    module->getInterface(lattice->getCellInDirection(module->position, FCCLattice::Direction::C7South)),
                    100, 0);
    } else {
        console << "Bottom hole reached\n";
        module->setColor(GREEN);

        //search for the width
        if (lattice->cellHasBlock(module->position.offsetX(1))) {
            nbWaitedCheckLR++;
            sendMessage("CheckLR<right> msg", new Message(CHECKLR_MSG_ID),
                        module->getInterface(module->position.offsetX(1)), 100, 0);
        }
        if (lattice->cellHasBlock(module->position.offsetX(-1))) {
            nbWaitedCheckLR++;
            sendMessage("CheckLR<left> msg", new Message(CHECKLR_MSG_ID),
                        module->getInterface(module->position.offsetX(-1)), 100, 0);
        }

    }

}

void ShapeRecognitionBlockCode::handleCheckLRMessage(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender) {
    console << "recv checkLR from " << sender->getConnectedBlockId() << "\n";
    if (l == -1) {
        console << "push\n";
        waiting.push(sender);
        return;
    } else {
        if (isBottom())
            sendMessage("sendLR msg", new MessageOf<int>(SENDLR_MSG_ID, l), sender, 100, 0);
        else
            sendMessage("sendLR msg", new MessageOf<int>(SENDLR_MSG_ID, -1), sender, 100, 0);
    }
}

void ShapeRecognitionBlockCode::handleSendLRMessage(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender) {
    {
        auto *msg = dynamic_cast<MessageOf<int> *>(_msg.get());
        console << "recv sendLR msg from " << sender->getConnectedBlockId() << "\n";

        int recvL = *msg->getData();
        nbWaitedCheckLR--;
        if (sender == module->getInterface(module->position.offsetX(1))) {
            rightL = recvL;
        } else {
            leftL = recvL;
        }
        if (nbWaitedCheckLR == 0) {
            if (l == rightL and l == leftL) return;
            if (l != rightL and l != leftL) {
                myRectangle = Rectangle(module->position, l, 1);
                module->setColor(BLUE);
                currentShape.insert(myRectangle);
                colorRectangle(myRectangle);
                sendMessageToAllNeighbors("BroadcastRect msg",
                                          new MessageOf<Rectangle>(BROADCASTRECT_MSG_ID, myRectangle), 100, 0, 0);
            }
            if (l == leftL and rightL != l) {
                sendMessage("GetW message", new MessageOf<int>(GETW_MSG_ID, 1),
                            module->getInterface(module->position.offsetX(-1)), 100, 0);
            }
            if (l == rightL and leftL != l) {
                sendMessage("GetW message", new MessageOf<int>(GETW_MSG_ID, 1),
                            module->getInterface(module->position.offsetX(1)), 100, 0);
            }
        }
    }

}

void ShapeRecognitionBlockCode::handleGetWMessage(std::shared_ptr<Message> _msg,
                                                  P2PNetworkInterface *sender) {
    auto *msg = dynamic_cast<MessageOf<int> *>(_msg.get());
    int recvW = *msg->getData();
    recvW++;
    console << "recv GetW<" << recvW << ">  msg from " << sender->getConnectedBlockId() << "\n";
    if (sender == module->getInterface(module->position.offsetX(1))) { // coming from right
        if (leftL != l) {
            /*  myRectangle = Rectangle(module->position, l, recvW);*/
            sendMessage("SetW message", new MessageOf<int>(SETW_MSG_ID, recvW),
                        module->getInterface(module->position.offsetX(1)), 100, 0);

        } else {
            sendMessage("GetW message", new MessageOf<int>(GETW_MSG_ID, recvW),
                        module->getInterface(module->position.offsetX(-1)), 100, 0);
        }
    } else {
        if (rightL != l) {
/*
            myRectangle = Rectangle(module->position, l, recvW);
*/
            sendMessage("SetW message", new MessageOf<int>(SETW_MSG_ID, recvW),
                        module->getInterface(module->position.offsetX(-1)), 100, 0);

        } else {
            sendMessage("GetW message", new MessageOf<int>(GETW_MSG_ID, recvW),
                        module->getInterface(module->position.offsetX(1)), 100, 0);
        }
    }

}

void ShapeRecognitionBlockCode::handleSetWMessage(std::shared_ptr<Message> _msg,
                                                  P2PNetworkInterface *sender) {
    auto *msg = dynamic_cast<MessageOf<int> *>(_msg.get());
    int recvW = *msg->getData();
    if (sender == module->getInterface(module->position.offsetX(1))) { // coming from right
        if (leftL != l) {
            myRectangle = Rectangle(module->position, l, recvW);
            module->setColor(BLUE);
            currentShape.insert(myRectangle);
            colorRectangle(myRectangle);
            sendMessageToAllNeighbors("BroadcastRect msg", new MessageOf<Rectangle>(BROADCASTRECT_MSG_ID, myRectangle),
                                      100, 0, 0);
        } else {
            sendMessage("SetW message", new MessageOf<int>(SETW_MSG_ID, recvW),
                        module->getInterface(module->position.offsetX(-1)), 100, 0);
        }
    } else {
        if (rightL != l) {
            myRectangle = Rectangle(module->position, l, -recvW);
            module->setColor(BLUE);
            currentShape.insert(myRectangle);
            colorRectangle(myRectangle);
            sendMessageToAllNeighbors("BroadcastRect msg", new MessageOf<Rectangle>(BROADCASTRECT_MSG_ID, myRectangle),
                                      100, 0, 0);
        } else {
            sendMessage("SetW message", new MessageOf<int>(SETW_MSG_ID, recvW),
                        module->getInterface(module->position.offsetX(1)), 100, 0);
        }
    }
}

void ShapeRecognitionBlockCode::handleBroadcastRectWMessage(std::shared_ptr<Message> _msg,
                                                            P2PNetworkInterface *sender) {
    auto *msg = dynamic_cast<MessageOf<Rectangle> *>(_msg.get());
    Rectangle recvRectangle = *msg->getData();
    console << "recv Broadcast " << recvRectangle.base << "\n";
    if (find(currentShape.begin(), currentShape.end(), recvRectangle) != currentShape.end()) return;
    if (isInshape(recvRectangle.base)) return;
    currentShape.insert(recvRectangle);
    sendMessageToAllNeighbors("BroacastRect msg", new MessageOf<Rectangle>(BROADCASTRECT_MSG_ID, recvRectangle), 100, 0,
                              1, sender);
    if(currentShape.size() == 12) {

    }
}

bool ShapeRecognitionBlockCode::isInshape(Cell3DPosition &pos) {
    for (auto r: currentShape) {
        if(Rectangle::isInRectangle(r, pos)) return true;
       /* if (r.w >= 0) {
            if (pos.pt[0] >= r.base.pt[0] and pos.pt[0] <= r.base.pt[0] + r.w - 1
                and pos.pt[1] >= r.base.pt[1] and pos.pt[1] <= r.base.pt[1] + r.l - 1) {
                return true;
            }
        } else {
            if (pos.pt[0] <= r.base.pt[0] and pos.pt[0] >= r.base.pt[0] + r.w + 1
                and pos.pt[1] >= r.base.pt[1] and pos.pt[1] <= r.base.pt[1] + r.l - 1) {
                return true;
            }
        }*/
    }
    return false;
}

void ShapeRecognitionBlockCode::onMotionEnd() {
    console << " has reached its destination" << "\n";

    // do stuff
    // ...
}

void ShapeRecognitionBlockCode::processLocalEvent(EventPtr pev) {
    std::shared_ptr<Message> message;
    stringstream info;

    // Do not remove line below
    Catoms3DBlockCode::processLocalEvent(pev);

    switch (pev->eventType) {
        case EVENT_ADD_NEIGHBOR: {
            // Do something when a neighbor is added to an interface of the module
            break;
        }

        case EVENT_REMOVE_NEIGHBOR: {
            // Do something when a neighbor is removed from an interface of the module
            break;
        }
    }
}

/// ADVANCED BLOCKCODE FUNCTIONS BELOW

void ShapeRecognitionBlockCode::onBlockSelected() {
    // Debug stuff:
    cerr << endl << "--- PRINT MODULE " << *module << "---" << endl;
    cerr << "l: " << l << endl;
    cerr << "nbWaitedCheck: " << nbWaitedCheckLR << endl;
    cerr << "myRectangle: " << myRectangle.base << " (" << myRectangle.l << ", " << myRectangle.w << ")\n";
    cerr << "currentSHape: ";
    for (auto &r: currentShape) {
        cerr << r.base << " (" << r.l << ", " << r.w << ") U ";
    }
    cerr << endl;
    cerr << "currentShape size: " << currentShape.size() << endl;
}

void ShapeRecognitionBlockCode::onAssertTriggered() {
    console << " has triggered an assert" << "\n";

    // Print debugging some info if needed below
    // ...
}

bool ShapeRecognitionBlockCode::parseUserCommandLineArgument(int &argc, char **argv[]) {
    /* Reading the command line */
    if ((argc > 0) && ((*argv)[0][0] == '-')) {
        switch ((*argv)[0][1]) {

            // Single character example: -b
            case 'b': {
                cout << "-b option provided" << endl;
                return true;
            }
                break;

                // Composite argument example: --foo 13
            case '-': {
                string varg = string((*argv)[0] + 2); // argv[0] without "--"
                if (varg == string("foo")) { //
                    int fooArg;
                    try {
                        fooArg = stoi((*argv)[1]);
                        argc--;
                        (*argv)++;
                    } catch (std::logic_error &) {
                        stringstream err;
                        err << "foo must be an integer. Found foo = " << argv[1] << endl;
                        throw CLIParsingError(err.str());
                    }

                    cout << "--foo option provided with value: " << fooArg << endl;
                } else return false;

                return true;
            }

            default:
                cerr << "Unrecognized command line argument: " << (*argv)[0] << endl;
        }
    }

    return false;
}

string ShapeRecognitionBlockCode::onInterfaceDraw() {
    stringstream trace;
    trace << "Number of rectangles: " << currentShape.size();
    return trace.str();
}

bool ShapeRecognitionBlockCode::isBottom() const {
    return !lattice->cellHasBlock(module->position.offsetY(-1));
}

bool ShapeRecognitionBlockCode::isTop() const {
    return !lattice->cellHasBlock(module->position.offsetY(1));
}

int ShapeRecognitionBlockCode::c = 0;

void ShapeRecognitionBlockCode::colorRectangle(Rectangle &r) {
   for(auto b: BaseSimulator::getWorld()->buildingBlocksMap) {
       if(Rectangle::isInRectangle(r, b.second->position)) {
           b.second->setColor(c);
       }
   }
   c++;
}
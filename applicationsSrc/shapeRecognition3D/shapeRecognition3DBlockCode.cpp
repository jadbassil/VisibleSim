#include "shapeRecognition3DBlockCode.hpp"

using namespace BlinkyBlocks;

ostream& operator<<(ostream& stream, const  Box& box) {
    stream << "Box[" << box.base << ", " << box.corner << "]";
    return  stream;
}

int ShapeRecognition3DBlockCode::c = 0;

list<Box> ShapeRecognition3DBlockCode::Boxes;

ShapeRecognition3DBlockCode::ShapeRecognition3DBlockCode(BlinkyBlocksBlock *host) : BlinkyBlocksBlockCode(host) {
    // @warning Do not remove block below, as a blockcode with a NULL host might be created
    //  for command line parsing
    if (not host) return;


    // Set the module pointer
    module = static_cast<BlinkyBlocksBlock *>(hostBlock);
}

void ShapeRecognition3DBlockCode::startup() {
    console << "start\n";
    myBox = Box();
    if (isBackmost()) {
        d = module->position.pt[1];
        if (!isFrontmost()) {
            sendHandleableMessage(new SetDMessage(d), module->getInterface(SCLattice::Direction::Front));
        } else { //FrontMost and backmost
            searchForWidth();
        }
    }
    if (isRightmost()) {
        w = module->position.pt[0];
        if (!isLeftmost()) {
            sendHandleableMessage(new SetWMessage(w), module->getInterface(SCLattice::Direction::Left));
        }
    }
}

bool ShapeRecognition3DBlockCode::isBackmost() const {
    return !lattice->cellHasBlock(module->position.offsetY(1));
}

bool ShapeRecognition3DBlockCode::isFrontmost() const {
    return !lattice->cellHasBlock(module->position.offsetY(-1));
}


[[maybe_unused]] bool ShapeRecognition3DBlockCode::isTopmost() const {
    return !lattice->cellHasBlock(module->position.offsetZ(1));
}

[[maybe_unused]] bool ShapeRecognition3DBlockCode::isBottommost() const {
    return !lattice->cellHasBlock(module->position.offsetZ(-1));
}

bool ShapeRecognition3DBlockCode::isRightmost() const {
    return !lattice->cellHasBlock(module->position.offsetX(1));
}

bool ShapeRecognition3DBlockCode::isLeftmost() const {
    return !lattice->cellHasBlock(module->position.offsetX(-1));
}


int ShapeRecognition3DBlockCode::sendHandleableMessage(HandleableMessage *msg, P2PNetworkInterface *dest) {
    return BlockCode::sendMessage(msg, dest, 100, 0);
}

void ShapeRecognition3DBlockCode::processLocalEvent(EventPtr pev) {
    std::shared_ptr<Message> message;
    stringstream info;

    // Do not remove line below
    BlinkyBlocksBlockCode::processLocalEvent(pev);

    switch (pev->eventType) {
        case EVENT_RECEIVE_MESSAGE: {
            message = (std::static_pointer_cast<NetworkInterfaceReceiveEvent>(pev))->message;
            if (message->isMessageHandleable()) {
                std::shared_ptr<HandleableMessage> hMsg =
                        (std::static_pointer_cast<HandleableMessage>(message));
                console << " received " << hMsg->getName() << " from "
                        << message->sourceInterface->hostBlock->blockId << " at "
                        << getScheduler()->now() << "\n";
                hMsg->handle(this);
            }
        }
            break;

        case EVENT_INTERRUPTION: {
            std::shared_ptr<InterruptionEvent> itev =
                    std::static_pointer_cast<InterruptionEvent>(pev);
            switch(itev->mode) {
                case IT_MODE_FINDW: {
                    if(not isFrontmost()) {
                        if(module->getInterface(SCLattice::Direction::Left)->isConnected()) {
                            auto* leftNeighbor = dynamic_cast<ShapeRecognition3DBlockCode*>(
                                    BaseSimulator::getWorld()->getBlockByPosition(module->position.offsetX(-1))->blockCode);

                            console << "leftD: " << leftNeighbor->hostBlock->blockId << "\n";
                            leftD = leftNeighbor->d;
                        }
                        if(module->getInterface(SCLattice::Direction::Right)->isConnected()) {
                            auto* rightNeighbor = dynamic_cast<ShapeRecognition3DBlockCode*>(
                                    BaseSimulator::getWorld()->getBlockByPosition(module->position.offsetX(1))->blockCode);

                            rightD = rightNeighbor->d;
                        }
                        console << "leftD: " << leftD << " rightD: " << rightD << "\n";
                    }
                    if(d == -1 or (leftD == -1 and module->getInterface(SCLattice::Direction::Left)->isConnected())
                       or (rightD == -1 and module->getInterface(SCLattice::Direction::Right)->isConnected())) {
                        // Reschedule another interruption in 500 us
                        console << "Reschedule\n";
                        getScheduler()->schedule(
                                new InterruptionEvent(getScheduler()->now() + 500, // example delay in us
                                                      module, IT_MODE_FINDW));
                    } else {

                        while (not waitingFindWMsgs.empty()) {
                            std::shared_ptr<Message> msg = static_cast<shared_ptr<Message>>(waitingFindWMsgs.front());
                            std::shared_ptr<HandleableMessage> hMsg =
                                    (std::static_pointer_cast<HandleableMessage>(msg));
                            hMsg->handle(this);
                            waitingFindWMsgs.pop();
                        }
                    }

                } break;
                case IT_MODE_FINDH: {
                    if(not isBottommost()) {
                        if(module->getInterface(SCLattice::Direction::Top)->isConnected()) {
                            auto* topNeighbor = dynamic_cast<ShapeRecognition3DBlockCode*>(
                                    BaseSimulator::getWorld()->getBlockById(module->getInterface(SCLattice::Direction::Top)->getConnectedBlockId())->blockCode);

                            console << "topD: " << topNeighbor->hostBlock->blockId << "\n";
                            topD = topNeighbor->d;
                            topW = topNeighbor->w;
                        }
                        if(module->getInterface(SCLattice::Direction::Bottom)->isConnected()) {
                            auto* bottomNeighbor = dynamic_cast<ShapeRecognition3DBlockCode*>(
                                    BaseSimulator::getWorld()->getBlockById(module->getInterface(SCLattice::Direction::Bottom)->getConnectedBlockId())->blockCode);

                            console << "bottom: " << bottomNeighbor->hostBlock->blockId << "\n";
                            bottomD = bottomNeighbor->d;
                            bottomW = bottomNeighbor->w;
                        }
                        if (d == -1 or w == -1 or ((topD == -1 or topW == -1) and
                                                                  module->getInterface(SCLattice::Direction::Top)->isConnected())
                               or ((bottomD == -1 or bottomW == -1) and
                                   module->getInterface(SCLattice::Direction::Bottom)->isConnected())) {
                            // Reschedule another interruption in 500 us
                            console << "Reschedule\n";
                            getScheduler()->schedule(
                                    new InterruptionEvent(getScheduler()->now() + 500, // example delay in us
                                                          module, IT_MODE_FINDH));
                        } else {

                            while (not waitingFindHMsgs.empty()) {
                                std::shared_ptr<Message> msg = static_cast<shared_ptr<Message>>(waitingFindHMsgs.front());
                                std::shared_ptr<HandleableMessage> hMsg =
                                        (std::static_pointer_cast<HandleableMessage>(msg));
                                hMsg->handle(this);
                                waitingFindHMsgs.pop();
                            }
                        }
                    }
                } break;
            }
        } break;

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

void ShapeRecognition3DBlockCode::onBlockSelected() {
    // Debug stuff:
    cerr << endl << "--- PRINT MODULE " << *module << "---" << endl;
    cerr << "d: " << d << endl;
    cerr << "w: " << w << endl;
    cerr << "h: " << h << endl;
    cerr << "leftD: " << leftD << "; rightD: " << rightD << "; topD: " << topD << "; bottomD: " << bottomD << endl;
    cerr <<  "topW: " << topW << "; bottomW: " << bottomW << endl;
    cerr << "Box:" << myBox << " \n";
    cerr << "Boxes: " << endl;
    combineBoxes();
    for(auto box: ShapeRecognition3DBlockCode::Boxes) {
        cout << box << endl;
    }
    cerr << "nb of boxes: " << Boxes.size() << endl;
}

void ShapeRecognition3DBlockCode::onAssertTriggered() {
    console << " has triggered an assert" << "\n";

    // Print debugging some info if needed below
    // ...
}

bool ShapeRecognition3DBlockCode::parseUserCommandLineArgument(int &argc, char **argv[]) {
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

string ShapeRecognition3DBlockCode::onInterfaceDraw() {
    stringstream trace;
    trace << "Some value " << 123;
    return trace.str();
}

void ShapeRecognition3DBlockCode::colorBox(Box &box) {
    for(auto &block: BaseSimulator::getWorld()->buildingBlocksMap) {
        if(Box::isPositionInBox(box, block.second->position)) {
            block.second->setColor(c);
        }
    }
    c++;
}

void ShapeRecognition3DBlockCode::setMyBox() {
    Cell3DPosition corner(w, d, h);
    Cell3DPosition base = module->position;
    if(corner.pt[0] < base.pt[0]) {
        int tmp = base.pt[0];
        base.pt[0] = corner.pt[0];
        corner.pt[0] = tmp;

    }
    if(corner.pt[2] < base.pt[2]) {
        int tmp = base.pt[2];
        base.pt[2] = corner.pt[2];
        corner.pt[2] = tmp;
    }
    myBox = Box(base, corner);
    console << "Box is Set " << myBox << "\n";
    ShapeRecognition3DBlockCode::colorBox(myBox);
    if(std::find(Boxes.begin(), Boxes.end(),myBox) == Boxes.end()) {
        auto it = Boxes.begin();
        bool myBoxisIn = false;
        while(it!=Boxes.end()) {
            pair<bool, Box> combined = Box::combine(myBox,*it);
            if(combined.first) {
                ShapeRecognition3DBlockCode::Boxes.push_back(combined.second);
            }
            it++;
        }
        it = Boxes.begin();
        while(it!=Boxes.end()) {
            if(Box::isBoxInBox(*it, myBox)) {
                it = Boxes.erase(it);
                continue;
            }  if (Box::isBoxInBox(myBox, *it)) {
                myBoxisIn = true;
            }
            it++;
        }
        if(not myBoxisIn) {
            ShapeRecognition3DBlockCode::Boxes.push_back(myBox);
        }
    }

}

void ShapeRecognition3DBlockCode::searchForHeight() {
    console << "Searching for height\n";
    while (not waitingCheckDW.empty()) {
        P2PNetworkInterface *waitingInt = waitingCheckDW.front();
        waitingCheckDW.pop();
        sendHandleableMessage(new NotifyWMessage(w), waitingInt);
    }
    if (module->getInterface(SCLattice::Direction::Top)->isConnected()) {
        sendHandleableMessage(new CheckWMessage(),
                                    module->getInterface(SCLattice::Direction::Top));
        nbWaitingNotifyW++;
    }
//    if (module->getInterface(SCLattice::Direction::Bottom)->isConnected()) {
//        sendHandleableMessage(new CheckWMessage(),
//                                    module->getInterface(SCLattice::Direction::Bottom));
//        nbWaitingNotifyW++;
//    }
    if(nbWaitingNotifyW == 0){
        //No top or bottom modules connected (No height)
        h = module->position[2];
        setMyBox();
    }
}

void ShapeRecognition3DBlockCode::searchForWidth() {
    for(int i = 0; i < module->getNbInterfaces(); i++) {
        if(module->getInterface(i)->isConnected()) {
            sendHandleableMessage(new CheckDMessage(),
                                  module->getInterface(i));
            nbWaitingNotifyD++;
        }
    }
}

void ShapeRecognition3DBlockCode::combineBoxes() {
    for(auto b1: ShapeRecognition3DBlockCode::Boxes) {
        for(auto b2: ShapeRecognition3DBlockCode::Boxes) {
            if(b1 != b2) {
                pair<bool, Box> combined = Box::combine(b1,b2);
                if(combined.first) {
                    ShapeRecognition3DBlockCode::Boxes.push_back(combined.second);
                }
            }
        }
    }
    auto it = Boxes.begin();
    while (it != Boxes.end()) {
        bool erased = false;
        for(auto b: Boxes) {
            if(*it != b) {
                if(Box::isBoxInBox(*it, b)) {
                    it =  Boxes.erase(it);
                    erased = true;
                    break;
                }
            }
        }
        if(not erased)
            it++;
    }
}

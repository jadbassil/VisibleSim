#include "shapeRecognition3DBlockCode.hpp"

using namespace BlinkyBlocks;

ostream& operator<<(ostream& stream, const  Box& box) {
    stream << "Box[" << box.base << ", " << box.corner << "]";
    return  stream;
}

int ShapeRecognition3DBlockCode::c = 0;

ShapeRecognition3DBlockCode::ShapeRecognition3DBlockCode(BlinkyBlocksBlock *host) : BlinkyBlocksBlockCode(host) {
    // @warning Do not remove block below, as a blockcode with a NULL host might be created
    //  for command line parsing
    if (not host) return;


    // Set the module pointer
    module = static_cast<BlinkyBlocksBlock *>(hostBlock);
}

void ShapeRecognition3DBlockCode::startup() {
    console << "start\n";
    if (isBackmost()) {
        d = module->position.pt[1];
        if (!isFrontmost()) {
            sendHandleableMessage(new SetMessage(Depth, d), module->getInterface(SCLattice::Direction::Front));
        } else { //FrontMost and backmost
            if (module->getInterface(SCLattice::Direction::Right)->isConnected()) {
               sendHandleableMessage(new CheckDMessage(),
                                            module->getInterface(SCLattice::Direction::Right));
                nbWaitingNotifyD++;
            }
            if (module->getInterface(SCLattice::Direction::Left)->isConnected()) {
                sendHandleableMessage(new CheckDMessage(),
                                            module->getInterface(SCLattice::Direction::Left));
               nbWaitingNotifyD++;
            }
        }
    }

}

bool ShapeRecognition3DBlockCode::isBackmost() const {
    return !lattice->cellHasBlock(module->position.offsetY(1));
}

bool ShapeRecognition3DBlockCode::isFrontmost() const {
    return !lattice->cellHasBlock(module->position.offsetY(-1));
}


bool ShapeRecognition3DBlockCode::isTopmost() const {
    return !lattice->cellHasBlock(module->position.offsetZ(1));
}

bool ShapeRecognition3DBlockCode::isBottommost() const {
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
    cerr << "leftD: " << leftD << "; rightD: " << rightD << endl;
    cerr << "Box:" << myBox << " \n";
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
        if(Box::isInBox(box, block.second->position)) {
            block.second->setColor(c);
        }
    }
    c++;
}

void ShapeRecognition3DBlockCode::setMyBox() {
    Cell3DPosition corner(w, d, 0);
    Cell3DPosition base = module->position;
    if(corner.pt[0] < base.pt[0]) {
        int tmp = base.pt[0];
        base.pt[0] = corner.pt[0];
        corner.pt[0] = tmp;

    }
    myBox = Box(base, corner);
    ShapeRecognition3DBlockCode::colorBox(myBox);
}

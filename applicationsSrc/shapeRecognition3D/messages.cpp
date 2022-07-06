//
// Created by jbassil on 05/07/22.
//
#include "messages.hpp"
#include "shapeRecognition3DBlockCode.hpp"

void SetMessage::handle(BaseSimulator::BlockCode *bc) {
    auto *srbc = dynamic_cast<ShapeRecognition3DBlockCode *>(bc);
    switch (type) {
        case Depth: {
            srbc->d = value;
            while (not srbc->waitingCheckD.empty()) {
                P2PNetworkInterface *waitingInt = srbc->waitingCheckD.front();
                srbc->waitingCheckD.pop();
                srbc->sendHandleableMessage(new NotifyMessage(Depth, srbc->d), waitingInt);
            }
            if (!srbc->isFrontmost()) {
                srbc->sendHandleableMessage(new SetMessage(Depth, srbc->d),
                                            srbc->module->getInterface(SCLattice::Direction::Front));
            } else {
                if (srbc->module->getInterface(SCLattice::Direction::Right)->isConnected()) {
                    srbc->sendHandleableMessage(new CheckDMessage(),
                                                srbc->module->getInterface(SCLattice::Direction::Right));
                    srbc->nbWaitingNotifyD++;
                }
                if (srbc->module->getInterface(SCLattice::Direction::Left)->isConnected()) {
                    srbc->sendHandleableMessage(new CheckDMessage(),
                                                srbc->module->getInterface(SCLattice::Direction::Left));
                    srbc->nbWaitingNotifyD++;
                }

            }
        } break;

        case Width: {
            srbc->w = value + 1;
            if (!srbc->isLeftmost()) {
                srbc->sendHandleableMessage(new SetMessage(Width, value + 1),
                                            srbc->module->getInterface(SCLattice::Direction::Left));
            }
        }
            break;

        case Height: {
            srbc->w = value + 1;
            if (!srbc->isBottommost()) {
                srbc->sendHandleableMessage(new SetMessage(Height, value + 1),
                                            srbc->module->getInterface(SCLattice::Direction::Bottom));
            }
        }
            break;

        default:
            break;
    }
}

void CheckDMessage::handle(BaseSimulator::BlockCode *bc) {
    auto *srbc = dynamic_cast<ShapeRecognition3DBlockCode *>(bc);
    P2PNetworkInterface *sender = destinationInterface;
    if (srbc->d == -1) {
        srbc->waitingCheckD.push(sender);
        return;
    }
    if(srbc->isFrontmost()) {
        srbc->sendHandleableMessage(new NotifyMessage(Depth, srbc->d), sender);
    } else {
        srbc->sendHandleableMessage(new NotifyMessage(Depth, -1), sender);
    }

}

void NotifyMessage::handle(BaseSimulator::BlockCode *bc) {
    auto *srbc = dynamic_cast<ShapeRecognition3DBlockCode *>(bc);
    P2PNetworkInterface *sender = destinationInterface;
    switch (type) {
        case Depth: {
            srbc->nbWaitingNotifyD--;
            if (srbc->module->getDirection(sender) == SCLattice::Direction::Left) srbc->leftD = value;
            else if (srbc->module->getDirection(sender) == SCLattice::Direction::Right) srbc->rightD = value;

            if (srbc->nbWaitingNotifyD == 0) {
                if ( srbc->d == srbc->leftD and srbc->d == srbc->rightD) {
                    return;
                }
                if (srbc->d != srbc->rightD and srbc->d != srbc->leftD) {
                    //TODO: must check for height
                    srbc->w = srbc->module->position.pt[0];
                    srbc->setMyBox();
                }
                if (srbc->d == srbc->leftD and srbc->d != srbc->rightD) {
                    srbc->sendHandleableMessage(new FindWMessage(-1),
                                                srbc->module->getInterface(SCLattice::Direction::Left));
                }
                if (srbc->d == srbc->rightD and srbc->d != srbc->leftD) {
                    srbc->sendHandleableMessage(new FindWMessage(-1),
                                                srbc->module->getInterface(SCLattice::Direction::Right));
                }
            }
        }
            break;

        case Width: {

        }
            break;

        case Height: {

        }
            break;

        default:
            break;
    }
}

void FindWMessage::handle(BaseSimulator::BlockCode *bc) {
    auto *srbc = dynamic_cast<ShapeRecognition3DBlockCode *>(bc);
    P2PNetworkInterface *sender = destinationInterface;
    if (value == -1) {
        if (srbc->module->getDirection(sender) == SCLattice::Direction::Left) {
            if (srbc->rightD == srbc->d) {
                srbc->sendHandleableMessage(new FindWMessage(-1),
                                            srbc->module->getInterface(SCLattice::Direction::Right));
            } else {
                srbc->sendHandleableMessage(new FindWMessage(srbc->module->position.pt[0]),
                                            srbc->module->getInterface(SCLattice::Direction::Left));
            }
        } else if (srbc->module->getDirection(sender) == SCLattice::Direction::Right) {
            if (srbc->leftD == srbc->d) {
                srbc->sendHandleableMessage(new FindWMessage(-1),
                                            srbc->module->getInterface(SCLattice::Direction::Left));
            } else {
                srbc->sendHandleableMessage(new FindWMessage(srbc->module->position.pt[0]),
                                            srbc->module->getInterface(SCLattice::Direction::Right));
            }
        } else {
            VS_ASSERT_MSG(false, "Received message from invalid direction!");
        }
    } else {
        if (srbc->module->getDirection(sender) == SCLattice::Direction::Left) {
            if (srbc->rightD != srbc->d) {
                //Todo rectangle set; must search for height
                srbc->w = value;
                srbc->setMyBox();

            } else {
                srbc->sendHandleableMessage(new FindWMessage(value),
                                            srbc->module->getInterface(SCLattice::Direction::Right));
            }
        } else if (srbc->module->getDirection(sender) == SCLattice::Direction::Right) {
            if (srbc->leftD != srbc->d) {
                //TODO rectangle set; must search for height
                srbc->w = value;
                srbc->setMyBox();
            } else {
                srbc->sendHandleableMessage(new FindWMessage(value),
                                            srbc->module->getInterface(SCLattice::Direction::Left));
            }
        }
    }
}
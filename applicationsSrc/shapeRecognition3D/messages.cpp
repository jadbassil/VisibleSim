//
// Created by jbassil on 05/07/22.
//
#include "messages.hpp"
#include "shapeRecognition3DBlockCode.hpp"

void SetMessage::handle(BaseSimulator::BlockCode *bc) {
    auto *srbc = dynamic_cast<ShapeRecognition3DBlockCode *>(bc);

    srbc->d = value;
    while (not srbc->waitingCheckD.empty()) {
        P2PNetworkInterface *waitingInt = srbc->waitingCheckD.front();
        srbc->waitingCheckD.pop();
        srbc->sendHandleableMessage(new NotifyDMessage(srbc->d), waitingInt);
    }
    if (!srbc->isFrontmost()) {
        srbc->sendHandleableMessage(new SetMessage(srbc->d),
                                    srbc->module->getInterface(SCLattice::Direction::Front));
    } else {
        srbc->searchForWidth();
    }
}

void CheckDMessage::handle(BaseSimulator::BlockCode *bc) {
    auto *srbc = dynamic_cast<ShapeRecognition3DBlockCode *>(bc);
    P2PNetworkInterface *sender = destinationInterface;
    if (srbc->d == -1) {
        srbc->waitingCheckD.push(sender);
        return;
    }
//    if(srbc->isFrontmost()) {
//        srbc->sendHandleableMessage(new NotifyDMessage(srbc->d), sender);
//    } else {
//        srbc->sendHandleableMessage(new NotifyDMessage(-1), sender);
//    }
    srbc->sendHandleableMessage(new NotifyDMessage(srbc->d), sender);
}

void NotifyDMessage::handle(BaseSimulator::BlockCode *bc) {
    auto *srbc = dynamic_cast<ShapeRecognition3DBlockCode *>(bc);
    P2PNetworkInterface *sender = destinationInterface;
    srbc->nbWaitingNotifyD--;

    switch(srbc->module->getDirection(sender)) {
        case SCLattice::Direction::Left: srbc->leftD = value; break;
        case SCLattice::Direction::Right: srbc->rightD = value; break;
        case SCLattice::Direction::Top: srbc->topD = value; break;
        case SCLattice::Direction::Bottom: srbc->bottomD = value; break;
    }

    if (srbc->nbWaitingNotifyD == 0) {
        if (srbc->d == srbc->leftD and srbc->d == srbc->rightD) {
            return;
        }
        //leftD != rightD
        if (srbc->d <= srbc->rightD and srbc->d != srbc->leftD) {
            srbc->sendHandleableMessage(new FindWMessage(-1, srbc->d, srbc->module->blockId),
                                        srbc->module->getInterface(SCLattice::Direction::Right));
        } else if (srbc->d != srbc->rightD and srbc->d != srbc->leftD) {
            srbc->w = srbc->module->position.pt[0];
            srbc->myBox.rectangleSet = true;
            srbc->searchForHeight();
        }
//        else if (srbc->d == srbc->leftD and srbc->d != srbc->rightD) {
//            srbc->sendHandleableMessage(new FindWMessage(-1, srbc->d, srbc->module->blockId),
//                                        srbc->module->getInterface(SCLattice::Direction::Left));
//        }

    }

}

void FindWMessage::handle(BaseSimulator::BlockCode *bc) {
    auto *srbc = dynamic_cast<ShapeRecognition3DBlockCode *>(bc);
    P2PNetworkInterface *sender = destinationInterface;
    srbc->console << "received FindW " << value << " from " << sender->getConnectedBlockId() << "\n";
    if (direction == -1) { //direction = GO
        if(srbc->d == -1 or (srbc->leftD == -1 and srbc->module->getInterface(SCLattice::Direction::Left)->isConnected())
            or (srbc->rightD == -1 and srbc->module->getInterface(SCLattice::Direction::Right)->isConnected())) {
            srbc->console << "Reschedule\n";
            srbc->waitingFindWMsgs.push(this->clone());
            getScheduler()->schedule(
                    new InterruptionEvent(getScheduler()->now() + 500, // example delay in us
                                          srbc->module, IT_MODE_FINDW));
            return;
        }
        if (srbc->module->getDirection(sender) == SCLattice::Direction::Left) {
            if (srbc->d >= value and srbc->rightD >= value) {
                srbc->sendHandleableMessage(new FindWMessage(-1, value, senderId),
                                            srbc->module->getInterface(SCLattice::Direction::Right));
            } else {
                srbc->sendHandleableMessage(new FindWMessage(1, srbc->module->position.pt[0], senderId),
                                            srbc->module->getInterface(SCLattice::Direction::Left));
            }
        } else if (srbc->module->getDirection(sender) == SCLattice::Direction::Right) {
            if (srbc->d >= value and srbc->leftD >= value) {
                srbc->sendHandleableMessage(new FindWMessage(-1, value, senderId),
                                            srbc->module->getInterface(SCLattice::Direction::Left));
            } else {
                srbc->sendHandleableMessage(new FindWMessage(1, srbc->module->position.pt[0], senderId),
                                            srbc->module->getInterface(SCLattice::Direction::Right));
            }
        } else {
            VS_ASSERT_MSG(false, "Received message from invalid direction!");
        }
    } else { //direction = Back
        if (srbc->module->blockId == senderId) {
            //Todo rectangle set; must search for height
            srbc->w = value;
            srbc->myBox.rectangleSet = true;
            srbc->searchForHeight();

        } else {
            srbc->sendHandleableMessage(new FindWMessage(1, value, senderId),
                                        srbc->module->getInterface(
                                                BaseSimulator::getWorld()->lattice->getOppositeDirection(
                                                        srbc->module->getDirection(sender))));
        }
//        if (srbc->module->getDirection(sender) == SCLattice::Direction::Left) {
//            if (srbc->module->blockId == senderId) {
//                //Todo rectangle set; must search for height
//                srbc->w = value;
//                srbc->myBox.rectangleSet = true;
//                srbc->searchForHeight();
//
//            } else {
//                srbc->sendHandleableMessage(new FindWMessage(1, value, senderId),
//                                            srbc->module->getInterface(SCLattice::Direction::Right));
//            }
//        } else if (srbc->module->getDirection(sender) == SCLattice::Direction::Right) {
//            if (srbc->module->blockId == senderId) {
//                //TODO rectangle set; must search for height
//                srbc->w = value;
//                srbc->myBox.rectangleSet = true;
//                srbc->searchForHeight();
//            } else {
//                srbc->sendHandleableMessage(new FindWMessage(1, value, senderId),
//                                            srbc->module->getInterface(SCLattice::Direction::Left));
//            }
//        }
    }
}


void CheckWMessage::handle(BaseSimulator::BlockCode *bc) {
    auto *srbc = dynamic_cast<ShapeRecognition3DBlockCode *>(bc);
    P2PNetworkInterface *sender = destinationInterface;
    if (srbc->d == -1 or
        (not srbc->myBox.rectangleSet and srbc->isFrontmost() /*and srbc->rightD == -1 and srbc->leftD == -1*/)) {
        srbc->waitingCheckDW.push(sender);
        srbc->console << "waitingCheckDW.push\n";
        return;
    }
    if(srbc->isFrontmost()) {
        srbc->sendHandleableMessage(new NotifyWMessage(srbc->w), sender);
    } else {
        srbc->sendHandleableMessage(new NotifyWMessage(-1), sender);
    }
}

void NotifyWMessage::handle(BaseSimulator::BlockCode *bc) {
    auto *srbc = dynamic_cast<ShapeRecognition3DBlockCode *>(bc);
    P2PNetworkInterface *sender = destinationInterface;

    srbc->nbWaitingNotifyW--;

    switch(srbc->module->getDirection(sender)) {
        case SCLattice::Direction::Top: srbc->topW = value; break;
        case SCLattice::Direction::Bottom: srbc->bottomW = value; break;
        default:
            VS_ASSERT(false);
    }

    if (srbc->nbWaitingNotifyW == 0) {
        if ( srbc->d == srbc->topD and srbc->d == srbc->bottomD
            and srbc->w == srbc->topW and srbc->w == srbc->bottomW) {
            return;
        }
        if(srbc->bottomW == -1 and srbc->topW == -1) {
            srbc->h =  srbc->module->position[2];
            srbc->setMyBox();
            return;
        }
        if (srbc->d != srbc->topD and srbc->d != srbc->bottomD
            and srbc->w != srbc->topW and srbc->w != srbc->bottomW) {
            srbc->h = srbc->module->position.pt[2];
            srbc->setMyBox();
        } else if (srbc->d == srbc->bottomD and srbc->d != srbc->topD and srbc->w <= srbc->bottomW) {
            srbc->sendHandleableMessage(new FindHMessage(-1),
                                        srbc->module->getInterface(SCLattice::Direction::Bottom));
        } else if (srbc->d == srbc->topD and srbc->d != srbc->bottomD and srbc->w <= srbc->topW) {
            srbc->sendHandleableMessage(new FindHMessage(-1),
                                        srbc->module->getInterface(SCLattice::Direction::Top));
        }
    }
}


void FindHMessage::handle(BaseSimulator::BlockCode *bc) {
    auto *srbc = dynamic_cast<ShapeRecognition3DBlockCode *>(bc);
    P2PNetworkInterface *sender = destinationInterface;

    if (value == -1) {
        if (srbc->module->getDirection(sender) == SCLattice::Direction::Bottom) {
            if (srbc->topD == srbc->d and srbc->topW == srbc->w) {
                srbc->sendHandleableMessage(new FindHMessage(-1),
                                            srbc->module->getInterface(SCLattice::Direction::Top));
            } else {
                srbc->sendHandleableMessage(new FindHMessage(srbc->module->position.pt[2]),
                                            srbc->module->getInterface(SCLattice::Direction::Bottom));
            }
        } else if (srbc->module->getDirection(sender) == SCLattice::Direction::Top) {
            if (srbc->bottomD == srbc->d and srbc->bottomW == srbc->w) {
                srbc->sendHandleableMessage(new FindHMessage(-1),
                                            srbc->module->getInterface(SCLattice::Direction::Bottom));
            } else {
                srbc->sendHandleableMessage(new FindHMessage(srbc->module->position.pt[2]),
                                            srbc->module->getInterface(SCLattice::Direction::Top));
            }
        } else {
            VS_ASSERT_MSG(false, "Received message from invalid direction!");
        }
    } else {

        if (srbc->module->getDirection(sender) == SCLattice::Direction::Bottom) {
            if (srbc->topD != srbc->d or srbc->topW != srbc->w) {
                srbc->h = value;
                srbc->setMyBox();
            } else {
                srbc->sendHandleableMessage(new FindHMessage(value),
                                            srbc->module->getInterface(SCLattice::Direction::Top));
            }
        } else if (srbc->module->getDirection(sender) == SCLattice::Direction::Top) {
            if (srbc->bottomD != srbc->d or srbc->bottomW != srbc->w) {
                srbc->h = value;
                srbc->setMyBox();
            } else {
                srbc->sendHandleableMessage(new FindHMessage(value),
                                            srbc->module->getInterface(SCLattice::Direction::Bottom));
            }
        }
    }
}


//
// Created by jbassil on 29/03/2022.
#include "asyncMessages.hpp"
#include "rePoStBlockCode.hpp"
//

void CrossedSrcMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);

    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        rbc.sendHandleableMessage(dynamic_cast<HandleableMessage *>(this->clone()),
                                  rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }

    if (rbc.isPotentialSource()) {
        if (rbc.parentPosition == Cell3DPosition(-1, -1, -1) or rbc.nbSrcCrossed > nbCrossedSrc + 1) {
            if (rbc.parentPosition != Cell3DPosition(-1, -1, -1)) {
                //send crossedAck(false) to parentPosition;
                rbc.sendHandleableMessage(new CrossedSrcAckMessage(rbc.MMPosition, rbc.parentPosition, false),
                                          rbc.interfaceTo(rbc.MMPosition, rbc.parentPosition), 100, 200);
            }
            rbc.nbSrcCrossed = nbCrossedSrc + 1;
            rbc.console << "nbSrcCrossed: " << rbc.nbSrcCrossed << "\n";
            rbc.parentPosition = fromMMPosition;
            rbc.console << "parentPosition: " << rbc.parentPosition << "\n";
            //send crossedAck(true) to parentPosition
            /*rbc.sendHandleableMessage(new CrossedSrcAckMessage(rbc.MMPosition, fromMMPosition, true),
                                      rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
*/
            for (auto &p: rbc.getAdjacentMMSeeds()) {
                auto *toSeed = dynamic_cast<RePoStBlockCode *>( BaseSimulator::getWorld()->getBlockByPosition(
                        p)->blockCode);
                if (toSeed->isPotentialSource() and toSeed->MMPosition != fromMMPosition) {
                    Cell3DPosition nextMMPosition = toSeed->MMPosition;
                    rbc.nbWaitedAnswersSrcCrossed++;
                    rbc.sendHandleableMessage(new CrossedSrcMessage(rbc.MMPosition, nextMMPosition, rbc.nbSrcCrossed),
                                              rbc.interfaceTo(rbc.MMPosition, nextMMPosition), 100, 200);
                }
            }
            if (rbc.nbWaitedAnswersSrcCrossed == 0) {
                rbc.sendHandleableMessage(new CrossedSrcAckMessage(rbc.MMPosition, rbc.parentPosition, true),
                                          rbc.interfaceTo(rbc.MMPosition, rbc.parentPosition), 100, 200);
                while (not rbc.waitingMessages.empty()) {
                    NetworkInterfaceEnqueueOutgoingEvent *ev = rbc.waitingMessages.front();
                    ev->date = getScheduler()->now();
                    getScheduler()->schedule(ev);
                    rbc.waitingMessages.pop();
                }
            }
        } else {
            rbc.sendHandleableMessage(new CrossedSrcAckMessage(rbc.MMPosition, fromMMPosition, false),
                                      rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
        }
    }
}

void CrossedSrcAckMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);

    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        rbc.sendHandleableMessage(dynamic_cast<HandleableMessage *>(this->clone()),
                                  rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    rbc.nbWaitedAnswersSrcCrossed--;
    if (isChild) {
        rbc.childrenPositions.push_back(fromMMPosition);
        rbc.isSource = false;
        if (not rbc.isDestination) rbc.setMMColor(GREY);

    } else {
        rbc.childrenPositions.erase(std::remove_if(rbc.childrenPositions.begin(), rbc.childrenPositions.end(),
                                                   [&](const Cell3DPosition &p) { return p == fromMMPosition; }),
                                    rbc.childrenPositions.end());
        if (rbc.childrenPositions.empty() and rbc.isPotentialSource()) {
            rbc.isSource = true;
            rbc.setMMColor(RED);
        }
    }
    if (rbc.nbWaitedAnswersSrcCrossed == 0) {
        if (rbc.parentPosition != Cell3DPosition(-1, -1, -1)) {
            rbc.sendHandleableMessage(new CrossedSrcAckMessage(rbc.MMPosition, rbc.parentPosition, true),
                                      rbc.interfaceTo(rbc.MMPosition, rbc.parentPosition), 100, 200);
        }
        while (not rbc.waitingMessages.empty()) {
            NetworkInterfaceEnqueueOutgoingEvent *ev = rbc.waitingMessages.front();
            ev->date = getScheduler()->now();
            getScheduler()->schedule(ev);
            rbc.waitingMessages.pop();
        }
    }
}

void FindSrcMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);

    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);

    if (rbc.module->position != toSeedPosition) {
        rbc.sendHandleableMessage(dynamic_cast<HandleableMessage *>(this->clone()),
                                  rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    //if (rbc.isPotentialSource()) {
    if (rbc.nbWaitedAnswersSrcCrossed > 0 or (rbc.isPotentialSource() and rbc.nbSrcCrossed == -1)) {
        // Finding non disconnecting sources in progress
        getScheduler()->trace("Reschedule", rbc.module->blockId, MAGENTA);
        rbc.waitingMessages.push(new NetworkInterfaceEnqueueOutgoingEvent(0, MessagePtr(this->clone()),
                                                                          destinationInterface));
        return;
    }
    //}

    if (rbc.isDestination) {
        rbc.sendHandleableMessage(new FoundSrcMessage(rbc.MMPosition, fromMMPosition, destination, false, false),
                                  rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
        return;
    }

    if (rbc.mainPathState == BFS and rbc.pathIn.find(destination) == rbc.pathIn.end()) {
        rbc.console << "Overlap\n";
        rbc.sendHandleableMessage(new OverlapMessage(rbc.MMPosition, fromMMPosition, destination),
                                  rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
        return;
    }


    if (rbc.pathIn.find(destination) == rbc.pathIn.end()) {
        rbc.mainPathState = BFS;
        if (rbc.isSource) {
            rbc.mainPathState = ConfPath;
            rbc.console << "lockedSrc: " << rbc.lockedSrc << "\n";
            rbc.pathIn[destination] = fromMMPosition;
            rbc.sendHandleableMessage(
                    new FoundSrcMessage(rbc.MMPosition, rbc.pathIn[destination], destination, true, true),
                    rbc.interfaceTo(rbc.MMPosition, rbc.pathIn[destination]), 100, 200);


            return;
        }
        rbc.pathIn[destination] = fromMMPosition;
        for (auto &p: rbc.getAdjacentMMSeeds()) {
            auto *toSeed = dynamic_cast<RePoStBlockCode *>( BaseSimulator::getWorld()->getBlockByPosition(
                    p)->blockCode);
            if (toSeed->MMPosition != fromMMPosition) {
                rbc.nbWaitedAnswersDestination[destination]++;
                rbc.pathOut[destination].push_back(toSeed->MMPosition);
                rbc.sendHandleableMessage(new FindSrcMessage(rbc.MMPosition, toSeed->MMPosition, destination),
                                          rbc.interfaceTo(rbc.MMPosition, toSeed->MMPosition), 100, 200);
            }
        }
        if (rbc.nbWaitedAnswersDestination[destination] == 0) {
            rbc.sendHandleableMessage(
                    new FoundSrcMessage(rbc.MMPosition, rbc.pathIn[destination], destination, false, true),
                    rbc.interfaceTo(rbc.MMPosition, rbc.pathIn[destination]), 100, 200);

        }
    } else {
        rbc.sendHandleableMessage(new FoundSrcMessage(rbc.MMPosition, fromMMPosition, destination, false, false),
                                  rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
    }

}

void FoundSrcMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);

    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        rbc.sendHandleableMessage(dynamic_cast<HandleableMessage *>(this->clone()),
                                  rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    rbc.nbWaitedAnswersDestination[destination]--;

    if (found and rbc.toSource.find(destination) == rbc.toSource.end()) {
        rbc.console << destination << "\n";
        rbc.toSource[destination] = fromMMPosition;
    }
    bool srcFound = (rbc.toSource.find(destination) != rbc.toSource.end()) or found;

    if (rbc.pathOut.find(destination) != rbc.pathOut.end() and not srcFound) {
        if (auto it = find(rbc.pathOut[destination].begin(), rbc.pathOut[destination].end(), fromMMPosition); it !=
                                                                                                              rbc.pathOut[destination].end()) {
            rbc.console << "erase: " << *it << "\n";
            rbc.pathOut[destination].erase(it);
        }
    }

    if (rbc.nbWaitedAnswersDestination[destination] == 0) {
        bool isRoot = false;
        if (rbc.pathIn.find(destination) != rbc.pathIn.end()) {
            if(srcFound) rbc.mainPathState = ConfPath;
            rbc.sendHandleableMessage(
                    new FoundSrcMessage(rbc.MMPosition, rbc.pathIn[destination], destination, srcFound, true),
                    rbc.interfaceTo(rbc.MMPosition, rbc.pathIn[destination]), 100, 200);

        } else {
            rbc.console << "root\n";

            if (rbc.toSource.find(destination) != rbc.toSource.end()) {
                isRoot = true;
                rbc.mainPathState = Streamline;
                rbc.sendHandleableMessage(
                        new ConfirmSrcMessage(rbc.MMPosition, rbc.toSource[rbc.MMPosition], destination),
                        rbc.interfaceTo(rbc.MMPosition, rbc.toSource[rbc.MMPosition]), 100, 200);
            }
        }

        if (not srcFound) {
            rbc.pathIn.erase(destination);
            rbc.pathOut.erase(destination);
            rbc.nbWaitedAnswersDestination.erase(destination);
        } else {
            for (auto p: rbc.pathOut[destination]) {
                if (p != rbc.toSource[destination]) {
                    rbc.sendHandleableMessage(new CutMessage(rbc.MMPosition, p, destination),
                                              rbc.interfaceTo(rbc.MMPosition, p), 100, 200);
                }
            }
        }
        if(isRoot) {
            rbc.pathOut[destination].clear();
            rbc.pathOut.erase(destination);
        }


    }
}

void ConfirmSrcMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);

    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        rbc.sendHandleableMessage(dynamic_cast<HandleableMessage *>(this->clone()),
                                  rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    rbc.mainPathState = Streamline;
/*    rbc.pathOut[destination].clear();
    rbc.pathOut.erase(destination);*/


    if (rbc.toSource.find(destination) != rbc.toSource.end()) {
        rbc.setMMColor(Colors[(destination.pt[0] + destination.pt[1] + destination.pt[2]) % 9]);
        rbc.sendHandleableMessage(new ConfirmSrcMessage(rbc.MMPosition, rbc.toSource[destination], destination),
                                  rbc.interfaceTo(rbc.MMPosition, rbc.toSource[destination]), 100, 200);
    } else {
        cerr << rbc.MMPosition << "->" << destination << endl;
        rbc.isSource = false;
    }

    for (auto &p: rbc.getAdjacentMMSeeds()) {
        auto *toSeed = dynamic_cast<RePoStBlockCode *>(BaseSimulator::getWorld()->getBlockByPosition(p)->blockCode);

            rbc.sendHandleableMessage(new AvailableAsyncMessage(rbc.MMPosition, toSeed->MMPosition, destination),
                                      rbc.interfaceTo(rbc.MMPosition, toSeed->MMPosition), 100, 200);

    }

}

void CutMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);

    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        rbc.sendHandleableMessage(dynamic_cast<HandleableMessage *>(this->clone()),
                                  rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    if (rbc.pathIn.find(destination) == rbc.pathIn.end()) return;
    if (rbc.pathOut[destination].empty()) {
        rbc.console << "empty: " << "\n";
    } else {
        for (auto &p: rbc.pathOut[destination]) {
            rbc.sendHandleableMessage(new CutMessage(rbc.MMPosition, p, destination),
                                      rbc.interfaceTo(rbc.MMPosition, p), 100, 200);
        }
    }

    rbc.mainPathState = NONE;
    rbc.pathIn.erase(destination);
    rbc.pathOut[destination].clear();
    rbc.pathOut.erase(destination);
    rbc.toSource.erase(destination);
    rbc.nbWaitedAnswersDestination.erase(destination);

    for (auto &p: rbc.getAdjacentMMSeeds()) {
        auto *toSeed = dynamic_cast<RePoStBlockCode *>(BaseSimulator::getWorld()->getBlockByPosition(p)->blockCode);
        rbc.sendHandleableMessage(new AvailableAsyncMessage(rbc.MMPosition, toSeed->MMPosition, destination),
                                  rbc.interfaceTo(rbc.MMPosition, toSeed->MMPosition), 100, 200);
    }
}

void OverlapMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);

    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        rbc.sendHandleableMessage(dynamic_cast<HandleableMessage *>(this->clone()),
                                  rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }


    if (auto it = std::find(rbc.pathOut[destination].begin(), rbc.pathOut[destination].end(), fromMMPosition); it !=
                                                                                                               rbc.pathOut[destination].end()) {
        rbc.nbWaitedAnswersDestination[destination]--;
        //rbc.pathOut[destination].erase(it);
    }
    bool srcFound = (rbc.toSource.find(destination) != rbc.toSource.end());
    if (rbc.nbWaitedAnswersDestination[destination] == 0 and srcFound) {
        rbc.sendHandleableMessage(
                new FoundSrcMessage(rbc.MMPosition, rbc.pathIn[destination], destination, srcFound, true),
                rbc.interfaceTo(rbc.MMPosition, rbc.pathIn[destination]), 100, 200);
        for (auto p: rbc.pathOut[destination]) {
            if (p != rbc.toSource[destination]) {
                rbc.sendHandleableMessage(new CutMessage(rbc.MMPosition, p, destination),
                                          rbc.interfaceTo(rbc.MMPosition, p), 100, 200);
            }
        }
    } else {
        rbc.mainPathState = Overlap;
    }
}


void AvailableAsyncMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);

    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        rbc.sendHandleableMessage(dynamic_cast<HandleableMessage *>(this->clone()),
                                  rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }

    if(rbc.mainPathState == Overlap) {
        rbc.nbWaitedAnswersDestination[rbc.pathIn.begin()->first] = 0;
        for(auto &p: rbc.pathOut[rbc.pathIn.begin()->first]) {
            rbc.nbWaitedAnswersDestination[rbc.pathIn.begin()->first]++;
            rbc.sendHandleableMessage(new FindSrcMessage(rbc.MMPosition, p, rbc.pathIn.begin()->first),
                                      rbc.interfaceTo(rbc.MMPosition, p), 100, 200);
        }
    }
}

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
    if (rbc.isPotentialSource()) {
        if (rbc.nbWaitedAnswersSrcCrossed > 0 or rbc.nbSrcCrossed == -1) {
            // Finding non disconnecting sources in progress
            getScheduler()->trace("Reschedule", rbc.module->blockId, MAGENTA);
            bool test = getScheduler()->schedule(
                    new NetworkInterfaceEnqueueOutgoingEvent(getScheduler()->now() + 1000, MessagePtr(this->clone()),
                                                             destinationInterface));
            if (not test) VS_ASSERT(false);
            return;
        }
    }

    /*if (rbc.pathIn.find(destination) == rbc.pathIn.end()) {
        rbc.mainPathsOld.push_back(destination);
        rbc.pathIn[destination] = fromMMPosition;
        if (rbc.isSource) {
            if(rbc.lockedSrc == 0) {
                rbc.console << "lockedSrc: " << rbc.lockedSrc << "\n";
                rbc.lockedSrc++;
                rbc.sendHandleableMessage(
                        new FoundSrcMessage(rbc.MMPosition, rbc.pathIn[destination], destination, rbc.MMPosition),
                        rbc.interfaceTo(rbc.MMPosition, rbc.pathIn[destination]), 100, 200);
            } else {
                getScheduler()->trace("locked", rbc.module->blockId, RED);
            }

        } else {
            for (auto &p: rbc.getAdjacentMMSeeds()) {
                auto *toSeed = dynamic_cast<RePoStBlockCode *>( BaseSimulator::getWorld()->getBlockByPosition(
                        p)->blockCode);
                if (toSeed->MMPosition != fromMMPosition) {
                    rbc.sendHandleableMessage(new FindSrcMessage(rbc.MMPosition, toSeed->MMPosition, destination),
                                              rbc.interfaceTo(rbc.MMPosition, toSeed->MMPosition), 100, 200);
                }
            }
        }
    }*/
    if (rbc.pathIn.find(destination) == rbc.pathIn.end()) {
        rbc.pathIn[destination] = fromMMPosition;
        if (rbc.isSource) {
            rbc.console << "lockedSrc: " << rbc.lockedSrc << "\n";
            rbc.lockedSrc++;
            rbc.sendHandleableMessage(
                    new FoundSrcMessage(rbc.MMPosition, rbc.pathIn[destination], destination, true, true),
                    rbc.interfaceTo(rbc.MMPosition, rbc.pathIn[destination]), 100, 200);
            return;
        }
        for (auto &p: rbc.getAdjacentMMSeeds()) {
            auto *toSeed = dynamic_cast<RePoStBlockCode *>( BaseSimulator::getWorld()->getBlockByPosition(
                    p)->blockCode);
            if (toSeed->MMPosition != fromMMPosition) {
                rbc.nbWaitedAnswersDestination[destination]++;
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
    if (isChild) {
        rbc.pathOut[destination].push_back(fromMMPosition);
    }

    if (rbc.nbWaitedAnswersDestination[destination] == 0) {
        //TODO: Cut when going branches when going up
        if (rbc.pathIn.find(destination) != rbc.pathIn.end()) {
            bool srcFound = (rbc.toSource.find(destination) != rbc.toSource.end());
            rbc.sendHandleableMessage(
                    new FoundSrcMessage(rbc.MMPosition, rbc.pathIn[destination], destination, srcFound, true),
                    rbc.interfaceTo(rbc.MMPosition, rbc.pathIn[destination]), 100, 200);
        } else {
            cerr << "root\n";
            rbc.sendHandleableMessage(new ConfirmSrcMessage(rbc.MMPosition, rbc.toSource[rbc.MMPosition], destination),
                                      rbc.interfaceTo(rbc.MMPosition, rbc.toSource[rbc.MMPosition]), 100, 200);
            for (auto &p: rbc.pathOut[destination]) {
                if (p != rbc.toSource[destination]) {
                    rbc.sendHandleableMessage(new CutMessage(rbc.MMPosition, p, destination),
                                              rbc.interfaceTo(rbc.MMPosition, p), 100, 200);
                }
            }
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
    if (rbc.toSource.find(destination) != rbc.toSource.end()) {
        rbc.sendHandleableMessage(new ConfirmSrcMessage(rbc.MMPosition, rbc.toSource[destination], destination),
                                  rbc.interfaceTo(rbc.MMPosition, rbc.toSource[destination]), 100, 200);
    } else {
        cerr << rbc.MMPosition << "->" << destination << endl;
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
    if (rbc.pathOut.empty()) {

    } else {
        for (auto &p: rbc.pathOut[destination]) {
            rbc.sendHandleableMessage(new CutMessage(rbc.MMPosition, p, destination),
                                      rbc.interfaceTo(rbc.MMPosition, p), 100, 200);
        }
    }
    rbc.pathIn.erase(destination);
    rbc.pathOut[destination].clear();
    rbc.pathOut.erase(destination);
    rbc.toSource.erase(destination);
    rbc.nbWaitedAnswersDestination.erase(destination);

}

//
// Created by jbassil on 29/03/2022.
#include "asyncMessages.hpp"
#include "transportMessages.hpp"
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

void FindPathMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);

    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);

    if (rbc.module->position != toSeedPosition) {
        P2PNetworkInterface* itf = rbc.interfaceTo(fromMMPosition, toMMPosition);
        if(not itf->isConnected()) {
            Cell3DPosition toPos;
            rbc.module->getNeighborPos(rbc.module->getInterfaceBId(itf), toPos);
            if(rbc.isInMM(toPos))
                getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent(getScheduler()->now()+RePoStBlockCode::getRoundDuration(), MessagePtr(this->clone()),
                                                                                  destinationInterface));

        } else {
            rbc.sendHandleableMessage(dynamic_cast<HandleableMessage *>(this->clone()),
                                      rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        }
        return;
    }
    if (rbc.nbWaitedAnswersSrcCrossed > 0 or (rbc.isPotentialSource() and rbc.nbSrcCrossed == -1)) {
        // Finding non disconnecting sources in progress
        getScheduler()->trace("Reschedule", rbc.module->blockId, MAGENTA);
        rbc.waitingMessages.push(new NetworkInterfaceEnqueueOutgoingEvent(0, MessagePtr(this->clone()),
                                                                          destinationInterface));
        return;
    }

    if(not rbc.waitingMessages.empty()) {
        VS_ASSERT(false);
    }

    if(rbc.fillingState == FULL) return;

    if (rbc.mainPathState == NONE  and
        std::find(rbc.mainPathsOld.begin(), rbc.mainPathsOld.end(), initiator) == rbc.mainPathsOld.end()) {
        rbc.mainPathsOld.push_back(initiator);

        rbc.sendHandleableMessage(new ConfirmEdgeAsyncMessage(rbc.MMPosition, fromMMPosition, initiator),
                                  rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
        rbc.pathDirection = pathDirection;
        if(nbCrossed > 0 and not rbc.isPotentialSource()) {
            nbCrossed = nbCrossed - 1;
        }
        if(rbc.pathDirection == DST_SRC and rbc.isSource) {
            rbc.console << "Source found\n";
            rbc.mainPathState = ConfPath;
            rbc.setMMColor(YELLOW);
            rbc.pathIn = make_pair(initiator, fromMMPosition);
            rbc.sendHandleableMessage(
                    new FoundPathMessage(rbc.MMPosition, fromMMPosition, initiator, true, true),
                    rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
        } else if(rbc.pathDirection == SRC_DST and not rbc.isPotentialSource() and nbCrossed == 0){
                rbc.console << "Filling Destination found\n";
                rbc.mainPathState = Streamline;
                rbc.setMMColor(YELLOW);
                rbc.isDestination = true;
                rbc.destinationOut = rbc.MMPosition;
                rbc.pathIn = make_pair(initiator, fromMMPosition);
                rbc.sendHandleableMessage(
                        new FoundPathMessage(rbc.MMPosition, fromMMPosition, initiator, true, true),
                        rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
        } else {
            rbc.mainPathState = BFS;
            rbc.setMMColor(CYAN);
            rbc.pathIn = make_pair(initiator, fromMMPosition);
            rbc.pathOut.second.clear();
            for(auto &p: rbc.getAdjacentMMSeeds()) {
                auto *toSeed = dynamic_cast<RePoStBlockCode *>( BaseSimulator::getWorld()->getBlockByPosition(
                        p)->blockCode);
                if (toSeed->MMPosition != fromMMPosition)
                    rbc.sendHandleableMessage(
                            new FindPathMessage(rbc.MMPosition, toSeed->MMPosition, initiator, rbc.pathDirection,
                                                nbCrossed),
                            rbc.interfaceTo(rbc.MMPosition, toSeed->MMPosition), 100, 200);
            }
        }
    }
}

void FoundPathMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);

    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        rbc.sendHandleableMessage(dynamic_cast<HandleableMessage *>(this->clone()),
                                  rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    if(rbc.mainPathState == BFS and isIn(rbc.pathOut.second, fromMMPosition)) {
        for(auto &p: rbc.pathOut.second) {
            if(p != fromMMPosition) {
                rbc.sendHandleableMessage(new CutMessage(rbc.MMPosition, p, initiator),
                                          rbc.interfaceTo(rbc.MMPosition, p), 100, 200);
            }
        }
        rbc.pathOut.second.clear();
        rbc.pathOut.first = initiator;
        rbc.pathOut.second.push_back(fromMMPosition);
        if(rbc.pathDirection == DST_SRC and rbc.MMPosition == initiator) {
            rbc.mainPathState = Streamline;
            rbc.setMMColor(MAGENTA);
            rbc.setOperation(rbc.pathOut.second[0], rbc.destinationOut);
            rbc.sendHandleableMessage(new ConfirmPathAsyncMessage(rbc.MMPosition, fromMMPosition, initiator),
                                      rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
        } else if(rbc.pathDirection == SRC_DST and rbc.MMPosition == initiator) {
            rbc.mainPathState = Streamline;
            rbc.setMMColor(MAGENTA);
            rbc.setOperation(rbc.pathIn.second, rbc.pathOut.second[0]);

            rbc.isSource = false;
            auto *coord = dynamic_cast<RePoStBlockCode *>(
                    BaseSimulator::getWorld()
                            ->getBlockByPosition(rbc.coordinatorPosition)
                            ->blockCode);
            Cell3DPosition targetModule =
                    rbc.seedPosition +
                    (*coord->operation->localRules)[0].currentPosition;

            VS_ASSERT(coord->operation);
            // start transporting
            coord->sendHandleableMessage(
                    new CoordinateMessage(coord->operation, targetModule,
                                          coord->module->position, coord->mvt_it),
                    coord->module->getInterface(coord->nearestPositionTo(targetModule)),
                    100, 200);
        } else {
            if(rbc.pathDirection == SRC_DST) {
                rbc.mainPathState = Streamline;
                rbc.setOperation(rbc.pathIn.second, rbc.pathOut.second[0]);

            } else if(rbc.pathDirection == DST_SRC) {
                rbc.mainPathState = ConfPath;
                rbc.setMMColor(YELLOW);
            }

            rbc.sendHandleableMessage(
                    new FoundPathMessage(rbc.MMPosition, rbc.pathIn.second, initiator, true, true),
                    rbc.interfaceTo(rbc.MMPosition, rbc.pathIn.second), 100, 200);
        }
    }
}

void ConfirmPathAsyncMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);

    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        rbc.sendHandleableMessage(dynamic_cast<HandleableMessage *>(this->clone()),
                                  rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    if(rbc.mainPathState == ConfPath and fromMMPosition == rbc.pathIn.second) {
        rbc.mainPathState = Streamline;
        rbc.setMMColor(MAGENTA);
        //rbc.setMMColor(Colors[(initiator.pt[0] + initiator.pt[1] + initiator.pt[2]) % 9]);
        switch (rbc.pathDirection) {
            case DST_SRC: {
                if(not rbc.isSource) {
                    rbc.setOperation( rbc.pathOut.second[0], rbc.pathIn.second);
                    rbc.sendHandleableMessage(new ConfirmPathAsyncMessage(rbc.MMPosition, rbc.pathOut.second[0], initiator),
                                              rbc.interfaceTo(rbc.MMPosition, rbc.pathOut.second[0]), 100, 200);
                } else { //Source
                    rbc.setOperation(rbc.MMPosition, rbc.pathIn.second);
                    rbc.isSource = false;
                    auto *coord = dynamic_cast<RePoStBlockCode *>(
                            BaseSimulator::getWorld()
                                    ->getBlockByPosition(rbc.coordinatorPosition)
                                    ->blockCode);
                    Cell3DPosition targetModule =
                            rbc.seedPosition +
                            (*coord->operation->localRules)[0].currentPosition;

                    VS_ASSERT(coord->operation);
                    // start transporting
                    coord->sendHandleableMessage(
                            new CoordinateMessage(coord->operation, targetModule,
                                                  coord->module->position, coord->mvt_it),
                            coord->module->getInterface(coord->nearestPositionTo(targetModule)),
                            100, 200);
                }
            } break;

            case SRC_DST: {
                if(not rbc.isDestination) {
                    rbc.sendHandleableMessage(new ConfirmPathAsyncMessage(rbc.MMPosition, rbc.pathOut.second[0], initiator),
                                              rbc.interfaceTo(rbc.MMPosition, rbc.pathOut.second[0]), 100, 200);
                } else {
                    rbc.setMMColor(GREEN);
                    VS_ASSERT(false);
                }
            } break;

            default:
                VS_ASSERT(false);
        }

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

    if(rbc.mainPathState != NONE and fromMMPosition == rbc.pathIn.second) {
        for (auto out : rbc.pathOut.second) {
            rbc.sendHandleableMessage(new CutMessage(rbc.MMPosition, out, initiator),
                                      rbc.interfaceTo(rbc.MMPosition, out), 100, 200);
        }
        rbc.mainPathState = NONE;
        rbc.setMMColor(GREY);
        rbc.pathOut.second.clear();
        rbc.pathDirection = NO_DIRECTION;
    }
    for(auto &p: rbc.getAdjacentMMSeeds()) {
        auto *toSeed = dynamic_cast<RePoStBlockCode *>( BaseSimulator::getWorld()->getBlockByPosition(
                p)->blockCode);
        rbc.sendHandleableMessage(new AvailableAsyncMessage(rbc.MMPosition, toSeed->MMPosition, initiator),
                                  rbc.interfaceTo(rbc.MMPosition, toSeed->MMPosition), 100, 200);
    }

}

void ConfirmEdgeAsyncMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);

    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        rbc.sendHandleableMessage(dynamic_cast<HandleableMessage *>(this->clone()),
                                  rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }

    if(rbc.mainPathState == BFS) {
        rbc.pathOut.first = initiator;
        rbc.pathOut.second.push_back(fromMMPosition);
    } else {
        rbc.sendHandleableMessage(new CutMessage(rbc.MMPosition, fromMMPosition, initiator),
                                  rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
    }
}

void AvailableAsyncMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);

    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        P2PNetworkInterface *itf = rbc.interfaceTo(fromMMPosition, toMMPosition);
        if (not itf) {
            return;
        }
        if (not itf->isConnected()) {
            Cell3DPosition toPos;
            rbc.module->getNeighborPos(itf->globalId, toPos);
            if (rbc.isInMM(toPos))
                getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent(
                        getScheduler()->now() + RePoStBlockCode::getRoundDuration(), MessagePtr(this->clone()),
                        destinationInterface));
        } else {
            rbc.sendHandleableMessage(dynamic_cast<HandleableMessage *>(this->clone()),
                                      rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        }
        return;
    }

    if(rbc.mainPathState == BFS) {
        rbc.sendHandleableMessage(
                new FindPathMessage(rbc.MMPosition, fromMMPosition, rbc.mainPathsOld.back(), rbc.pathDirection, rbc.nbSrcCrossed),
                rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
    }
}

void GoTermAsyncMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);

    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        P2PNetworkInterface *itf = rbc.interfaceTo(fromMMPosition, toMMPosition);
        if (not itf) {
            VS_ASSERT(false);
            return;
        }
        if (not itf->isConnected()) {
            if (rbc.MMPosition == initiator) {
                // Initiator MM construction not completed must wait and resend the message.
                Cell3DPosition toPos;
                rbc.module->getNeighborPos(rbc.module->getInterfaceBId(itf), toPos);
                if (rbc.isInMM(toPos))
                    getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent(
                            getScheduler()->now() + RePoStBlockCode::getRoundDuration(),
                            MessagePtr(this->clone()),
                            destinationInterface));
                else {
                    VS_ASSERT(false);
                }
            } else {
                // Receiver Mm construction not completed, hence the construction is incomplete.
                rbc.sendHandleableMessage(
                        new BackTermAsyncMessage(rbc.MMPosition, fromMMPosition, initiator, false),
                        rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
            }

        } else {
            rbc.sendHandleableMessage(dynamic_cast<HandleableMessage *>(this->clone()),
                                      rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        }

        return;
    }

    if (rbc.toDestination.find(initiator) == rbc.toDestination.end()) {
        for (auto &p: rbc.getAdjacentMMSeeds()) {
            // Check if a neighbor MM construction is not complete. Send back not terminated if so.
            auto *toSeed = dynamic_cast<RePoStBlockCode *>(rbc.lattice->getBlock(p)->blockCode);
            if (toSeed->MMPosition != initiator and not toSeed->MMBuildCompleted()) {
                rbc.sendHandleableMessage(
                        new BackTermAsyncMessage(rbc.MMPosition, fromMMPosition, initiator, false),
                        rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
                rbc.terminated = true;
                return;
            }
        }
        rbc.toDestination[initiator] = fromMMPosition;
        rbc.nbWaitedAnswersTermination[initiator] = 0;
        bool term = rbc.mainPathState == NONE and !rbc.isDestination;
        for (auto &p: rbc.getAdjacentMMSeeds()) {
            auto *toSeed = dynamic_cast<RePoStBlockCode *>(rbc.lattice->getBlock(p)->blockCode);
            if (toSeed->MMPosition != fromMMPosition) {
                if (rbc.sendHandleableMessage(new GoTermAsyncMessage(rbc.MMPosition, toSeed->MMPosition, initiator),
                                              rbc.interfaceTo(rbc.MMPosition, toSeed->MMPosition), 100, 200) != -1) {
                    rbc.nbWaitedAnswersTermination[initiator]++;
                }
            }
        }
        if (rbc.nbWaitedAnswersTermination[initiator] == 0) {
            rbc.sendHandleableMessage(
                    new BackTermAsyncMessage(rbc.MMPosition, rbc.toDestination[initiator], initiator, term),
                    rbc.interfaceTo(rbc.MMPosition, rbc.toDestination[initiator]), 100, 200);
            rbc.toDestination.erase(initiator);
            rbc.nbWaitedAnswersTermination.erase(initiator);
            if (rbc.nbWaitedAnswersTermination.empty()) rbc.terminated = true;
        }
    } else {
        rbc.sendHandleableMessage(
                new BackTermAsyncMessage(rbc.MMPosition, fromMMPosition, initiator, true),
                rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
    }
}

void BackTermAsyncMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);

    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        P2PNetworkInterface *itf = rbc.interfaceTo(fromMMPosition, toMMPosition);
        if (not itf) {
            VS_ASSERT(false);
            return;
        }
        if (not itf->isConnected()) {
            //Back term must be received by a complete MM. So, wait and resend the message.
            Cell3DPosition toPos;
            rbc.module->getNeighborPos(rbc.module->getInterfaceBId(itf), toPos);
            if (rbc.isInMM(toPos))
                getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent(
                        getScheduler()->now() + RePoStBlockCode::getRoundDuration(), MessagePtr(this->clone()),
                        destinationInterface));
            else
                VS_ASSERT(false);
        } else {
            rbc.sendHandleableMessage(dynamic_cast<HandleableMessage *>(this->clone()),
                                      rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        }

        return;
    }

    rbc.nbWaitedAnswersTermination[destination]--;
    rbc.console << "terminated1: " << rbc.terminated << "\n";
    rbc.terminated = rbc.terminated and idle;
    rbc.console << "terminated2: " << rbc.terminated << "\n";
    rbc.console << destination << ": " << rbc.nbWaitedAnswersTermination[destination] << "\n";
    VS_ASSERT(rbc.nbWaitedAnswersTermination[destination] >= 0);
    if (rbc.nbWaitedAnswersTermination[destination] == 0) {
        rbc.terminated = rbc.terminated and (rbc.mainPathState == NONE and !rbc.isDestination and rbc.MMBuildCompleted());
        rbc.console << "terminated3: " << rbc.terminated << "\n";
        if (rbc.toDestination.find(destination) != rbc.toDestination.end()) {
            if (rbc.toDestination[destination] != rbc.MMPosition) {
                rbc.sendHandleableMessage(
                        new BackTermAsyncMessage(rbc.MMPosition, rbc.toDestination[destination], destination,
                                                 rbc.terminated),
                        rbc.interfaceTo(rbc.MMPosition, rbc.toDestination[destination]), 100, 200);
                rbc.toDestination.erase(destination);
                rbc.nbWaitedAnswersTermination.erase(destination);
                if (rbc.nbWaitedAnswersTermination.empty()) rbc.terminated = true;
            } else {
                // Building goal shape terminated
                rbc.console << "terminated: " << rbc.terminated << "\n";
                rbc.toDestination.erase(destination);
                rbc.nbWaitedAnswersTermination.erase(destination);
                if (rbc.terminated) {
                    rbc.globallyTerminated = true;
                    //Notify MMs about termination via flooding
                    for (auto &p: rbc.getAdjacentMMSeeds()) {
                        auto *toSeed = dynamic_cast<RePoStBlockCode *>(rbc.lattice->getBlock(p)->blockCode);
                        rbc.sendHandleableMessage(new NotifyTermMessage(rbc.MMPosition, toSeed->MMPosition),
                                                  rbc.interfaceTo(rbc.MMPosition, toSeed->MMPosition), 100, 200);
                    }
                } else {
                    rbc.terminated = true;
                }

            }
        }
    }
}

void NotifyTermMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);

    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        rbc.sendHandleableMessage(dynamic_cast<HandleableMessage *>(this->clone()),
                                  rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    if(rbc.globallyTerminated) return;
    rbc.globallyTerminated = true;
    rbc.setMMColor(LIGHTGREEN);
    for(auto &p: rbc.getAdjacentMMSeeds()) {
        auto* toSeed = dynamic_cast<RePoStBlockCode*>(rbc.lattice->getBlock(p)->blockCode);
        if(toSeed->MMPosition != fromMMPosition) {
            rbc.sendHandleableMessage(new NotifyTermMessage(rbc.MMPosition, toSeed->MMPosition),
                                      rbc.interfaceTo(rbc.MMPosition, toSeed->MMPosition), 100, 200);
        }
    }
    if(rbc.isSource) {
        //Sources exist so must be filled inside sparse MM
        rbc.setMMColor(RED);
        rbc.mainPathState = BFS;
        rbc.mainPathsOld.push_back(rbc.MMPosition);
        rbc.pathDirection = SRC_DST;
        rbc.pathIn = make_pair(rbc.MMPosition, rbc.MMPosition);
        for (auto &p: rbc.getAdjacentMMSeeds()) {
            auto *toSeed = dynamic_cast<RePoStBlockCode *>(rbc.lattice->getBlock(p)->blockCode);
            rbc.sendHandleableMessage(
                    new FindPathMessage(rbc.MMPosition, toSeed->MMPosition, rbc.MMPosition, rbc.pathDirection,
                                        rbc.nbSrcCrossed),
                    rbc.interfaceTo(rbc.MMPosition, toSeed->MMPosition), 100, 200);
        }
    }
}

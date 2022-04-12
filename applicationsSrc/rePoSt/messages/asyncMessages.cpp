//
// Created by jbassil on 29/03/2022.
#include "asyncMessages.hpp"
#include "rePoStBlockCode.hpp"
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

void FindSrcMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);

    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);

    if (rbc.module->position != toSeedPosition) {
        if(not rbc.interfaceTo(fromMMPosition, toMMPosition)->isConnected()) {
            return;
            getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent(getScheduler()->now()+100, MessagePtr(this->clone()),
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
        rbc.waitingMessages.push(new NetworkInterfaceEnqueueOutgoingEvent(0, MessagePtr(this->clone()),
                                                                          destinationInterface));
        /*while (not rbc.waitingMessages.empty()) {
            NetworkInterfaceEnqueueOutgoingEvent *ev = rbc.waitingMessages.front();
            ev->date = getScheduler()->now();
            getScheduler()->schedule(ev);
            rbc.waitingMessages.pop();
        }*/
        return;
    }

    /*if (rbc.isDestination) {
        rbc.sendHandleableMessage(new FoundSrcMessage(rbc.MMPosition, fromMMPosition, destination, false, false),
                                  rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
        return;
    }*/

    if ((rbc.mainPathState == NONE /*or rbc.mainPathState == Streamline*/) and
        std::find(rbc.mainPathsOld.begin(), rbc.mainPathsOld.end(), destination) == rbc.mainPathsOld.end()) {
        rbc.mainPathsOld.push_back(destination);

        rbc.sendHandleableMessage(new ConfirmEdgeAsyncMessage(rbc.MMPosition, fromMMPosition, destination),
                                  rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);

        if(rbc.isSource and rbc.mainPathState == NONE) {
            rbc.mainPathState = ConfPath;
            rbc.pathIn[destination] = fromMMPosition;
            rbc.sendHandleableMessage(
                    new FoundSrcMessage(rbc.MMPosition, fromMMPosition, destination, true, true),
                    rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
        } else {
            rbc.mainPathState = BFS;
            rbc.pathIn[destination] = fromMMPosition;
            rbc.pathOut[destination].clear();
            for(auto &p: rbc.getAdjacentMMSeeds()) {
                auto *toSeed = dynamic_cast<RePoStBlockCode *>( BaseSimulator::getWorld()->getBlockByPosition(
                        p)->blockCode);
                if (toSeed->MMPosition != fromMMPosition)
                    rbc.sendHandleableMessage(new FindSrcMessage(rbc.MMPosition, toSeed->MMPosition, destination),
                                              rbc.interfaceTo(rbc.MMPosition, toSeed->MMPosition), 100, 200);
            }
        }

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

    if(rbc.mainPathState == BFS and isIn(rbc.pathOut[destination], fromMMPosition)) {
        for(auto &p: rbc.pathOut[destination]) {
            if(p != fromMMPosition) {
                rbc.sendHandleableMessage(new CutMessage(rbc.MMPosition, p, destination),
                                          rbc.interfaceTo(rbc.MMPosition, p), 100, 200);
            }
        }
        rbc.pathOut[destination].clear();
        rbc.pathOut[destination].push_back(fromMMPosition);
        if(rbc.MMPosition == destination) {
            rbc.mainPathState = Streamline;
            rbc.setOperation(rbc.pathOut[destination][0], rbc.destinationOut);
            rbc.sendHandleableMessage(new ConfirmSrcMessage(rbc.MMPosition, fromMMPosition, destination),
                                      rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
        } else {
            rbc.mainPathState = ConfPath;
            rbc.sendHandleableMessage(
                    new FoundSrcMessage(rbc.MMPosition, rbc.pathIn[destination], destination, true, true),
                    rbc.interfaceTo(rbc.MMPosition, rbc.pathIn[destination]), 100, 200);
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
    if(rbc.mainPathState == ConfPath and fromMMPosition == rbc.pathIn[destination]) {
        rbc.mainPathState = Streamline;
        rbc.setMMColor(Colors[(destination.pt[0] + destination.pt[1] + destination.pt[2]) % 9]);

        if(not rbc.isSource) {
            rbc.setOperation( rbc.pathOut[destination][0], rbc.pathIn[destination]);
            rbc.sendHandleableMessage(new ConfirmSrcMessage(rbc.MMPosition, rbc.pathOut[destination][0], destination),
                                      rbc.interfaceTo(rbc.MMPosition, rbc.pathOut[destination][0]), 100, 200);
            //if next in streamrline is source remove if from disconnection tree and check if current MM is a source

            /*auto *out = dynamic_cast<RePoStBlockCode *>(BaseSimulator::getWorld()->getBlockByPosition(
                    rbc.getSeedPositionFromMMPosition(rbc.pathOut[destination][0]))->blockCode);
            if (out->isSource) {

                if (auto it= std::find(rbc.childrenPositions.begin(), rbc.childrenPositions.end(),
                                      rbc.pathOut[destination][0]); it != rbc.childrenPositions.end()) {
                    rbc.childrenPositions.erase(it);
                }
                if(rbc.childrenPositions.empty()) {
                    rbc.isSource = true;
                }
                //VS_ASSERT(false);
            }*/
            for(auto &p: rbc.getAdjacentMMSeeds()) {
                auto *toSeed = dynamic_cast<RePoStBlockCode *>( BaseSimulator::getWorld()->getBlockByPosition(
                        p)->blockCode);
                rbc.sendHandleableMessage(new AvailableAsyncMessage(rbc.MMPosition, toSeed->MMPosition, destination),
                                          rbc.interfaceTo(rbc.MMPosition, toSeed->MMPosition), 100, 200);
            }
        } else {
            rbc.setOperation(rbc.MMPosition, rbc.pathIn[destination]);
            rbc.isSource = false;
            auto *coord = dynamic_cast<RePoStBlockCode *>(
                    BaseSimulator::getWorld()
                            ->getBlockByPosition(rbc.coordinatorPosition)
                            ->blockCode);
            Cell3DPosition targetModule =
                    rbc.seedPosition +
                    (*coord->operation->localRules)[0].currentPosition;
            coord->console
                    << "targetModule: " << coord->nearestPositionTo(targetModule)
                    << "\n";

            VS_ASSERT(coord->operation);
            coord->sendHandleableMessage(
                    new CoordinateMessage(coord->operation, targetModule,
                                          coord->module->position, coord->mvt_it),
                    coord->module->getInterface(coord->nearestPositionTo(targetModule)),
                    100, 200);
        }

    }

 /*   rbc.sendHandleableMessage(new AvailableAsyncMessage(rbc.MMPosition, rbc.pathIn[destination], destination),
                             rbc.interfaceTo(rbc.MMPosition, rbc.pathIn[destination]), 100, 200);
    for (auto out : rbc.pathOut[destination]) {
        rbc.sendHandleableMessage(new AvailableAsyncMessage(rbc.MMPosition, out, destination),
                                  rbc.interfaceTo(rbc.MMPosition, out), 100, 200);
    }*/

}

void CutMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);

    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        rbc.sendHandleableMessage(dynamic_cast<HandleableMessage *>(this->clone()),
                                  rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }

    if(rbc.mainPathState != NONE and fromMMPosition == rbc.pathIn[destination]) {
        for (auto out : rbc.pathOut[destination]) {
            rbc.sendHandleableMessage(new CutMessage(rbc.MMPosition, out, destination),
                                      rbc.interfaceTo(rbc.MMPosition, out), 100, 200);
        }
        rbc.mainPathState = NONE;
        rbc.pathIn.erase(destination);
        rbc.pathOut[destination].clear();
        rbc.pathOut.erase(destination);
    }
    for(auto &p: rbc.getAdjacentMMSeeds()) {
        auto *toSeed = dynamic_cast<RePoStBlockCode *>( BaseSimulator::getWorld()->getBlockByPosition(
                p)->blockCode);
        rbc.sendHandleableMessage(new AvailableAsyncMessage(rbc.MMPosition, toSeed->MMPosition, destination),
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
        rbc.pathOut[destination].push_back(fromMMPosition);
    } else {
        rbc.sendHandleableMessage(new CutMessage(rbc.MMPosition, fromMMPosition, destination),
                                  rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
    }
}

void AvailableAsyncMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);

    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        if(not rbc.interfaceTo(fromMMPosition, toMMPosition)->isConnected()) {
            return;
            getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent(getScheduler()->now()+1000, MessagePtr(this->clone()),
                                                                              destinationInterface));
        } else {
            rbc.sendHandleableMessage(dynamic_cast<HandleableMessage *>(this->clone()),
                                      rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        }

        return;
    }

    if(rbc.mainPathState == BFS) {
        rbc.sendHandleableMessage(
                new FindSrcMessage(rbc.MMPosition, fromMMPosition, rbc.mainPathsOld.back()),
                rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
    }
}

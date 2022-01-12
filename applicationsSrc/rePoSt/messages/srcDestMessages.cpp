#include "srcDestMessages.hpp"

#include "maxFlowMessages.hpp"
#include "rePoStBlockCode.hpp"

void GoMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *static_cast<RePoStBlockCode *>(bc);

    rbc.console << "received: " << this->getName() << "\n";
    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        if (not rbc.interfaceTo(fromMMPosition, toMMPosition)) VS_ASSERT(false);
        rbc.sendHandleableMessage(new GoMessage(fromMMPosition, toMMPosition, distance),
                                  rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }

    if (rbc.parentPosition == Cell3DPosition(-1, -1, -1) and
            rbc.module->blockId != RePoStBlockCode::GC->blockId) {
        rbc.parentPosition = fromMMPosition;
        // distance = data->distance + 1;
        rbc.isPotentialSource() ? rbc.distanceSrc = distance + 1 : rbc.distanceSrc = distance;
        rbc.console << "distance1: " << rbc.distanceSrc << "\n";
        rbc.nbWaitedAnswers = 0;
        for (auto p : rbc.getAdjacentMMSeeds()) {
            Cell3DPosition toMMPosition1 =
                    dynamic_cast<RePoStBlockCode *>(
                            BaseSimulator::getWorld()->getBlockByPosition(p)->blockCode)
                            ->MMPosition;
            if (toMMPosition1 == fromMMPosition) continue;
            rbc.sendHandleableMessage(new GoMessage(rbc.MMPosition, toMMPosition1, rbc.distanceSrc),
                                      rbc.interfaceTo(rbc.MMPosition, toMMPosition1), 100, 200);

            rbc.nbWaitedAnswers++;
        }
        if (rbc.nbWaitedAnswers == 0) {
            rbc.sendHandleableMessage(new BackMessage(rbc.MMPosition, rbc.parentPosition, true),
                                      rbc.interfaceTo(rbc.MMPosition, rbc.parentPosition), 100,
                                      200);
        }
    } else if ((not rbc.isPotentialSource() and distance < rbc.distanceSrc) or
               (rbc.isPotentialSource() and distance + 1 < rbc.distanceSrc)) {
        rbc.sendHandleableMessage(new BackMessage(rbc.MMPosition, rbc.parentPosition, false),
                                  rbc.interfaceTo(rbc.MMPosition, rbc.parentPosition), 100, 200);

        rbc.parentPosition = fromMMPosition;
        rbc.childrenPositions.clear();
        rbc.isPotentialSource() ? rbc.distanceSrc = distance + 1 : rbc.distanceSrc = distance;
        rbc.console << "distance2 : " << rbc.distanceSrc << "\n";

        rbc.nbWaitedAnswers = 0;
        for (auto &p : rbc.getAdjacentMMSeeds()) {
            Cell3DPosition toMMPosition1 =
                dynamic_cast<RePoStBlockCode *>(
                    BaseSimulator::getWorld()->getBlockByPosition(p)->blockCode)
                    ->MMPosition;
            if (toMMPosition1 == fromMMPosition) continue;
            if (not rbc.interfaceTo(rbc.MMPosition, toMMPosition1)->isConnected()) VS_ASSERT(false);
            rbc.sendHandleableMessage(new GoMessage(rbc.MMPosition, toMMPosition1, rbc.distanceSrc),
                                      rbc.interfaceTo(rbc.MMPosition, toMMPosition1), 100, 200);
            rbc.nbWaitedAnswers++;
        }
        if (rbc.nbWaitedAnswers == 0) {
            Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(rbc.parentPosition);
            rbc.sendHandleableMessage(new BackMessage(rbc.MMPosition, rbc.parentPosition, true),
                                      rbc.interfaceTo(rbc.MMPosition, rbc.parentPosition), 100,
                                      200);
        }
    } else {
        Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(fromMMPosition);
        rbc.sendHandleableMessage(new BackMessage(rbc.MMPosition, fromMMPosition, false),
                                  rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
    }
    rbc.console << "parent: " << rbc.parentPosition << "\n";
}

void BackMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *static_cast<RePoStBlockCode *>(bc);

    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        rbc.sendHandleableMessage(static_cast<HandleableMessage *>(this->clone()),
                                  rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    rbc.nbWaitedAnswers--;
    rbc.console << "nbWaitedAnswers: " << rbc.nbWaitedAnswers << "\n";
    vector<Cell3DPosition>::iterator it =
        find(rbc.childrenPositions.begin(), rbc.childrenPositions.end(), fromMMPosition);
    if (isChild and it == rbc.childrenPositions.end()) {
        rbc.childrenPositions.push_back(fromMMPosition);
        rbc.isSource = false;
    } else if (not isChild) {
        if (it != rbc.childrenPositions.end()) {
            rbc.childrenPositions.erase(it);
        }
    }
    if (rbc.nbWaitedAnswers == 0) {
        if (rbc.parentPosition == Cell3DPosition(-1, -1, -1) and
            rbc.module->position == rbc.GC->position) {
            cerr << rbc.module->blockId << ": Coordination Tree is Built\n";
            reconfigurationStep = MAXFLOW;
            destinations.clear();
            // Start MaxFlow

            // Choosing destinations
            cerr << "targetMap size: " <<  RePoStBlockCode::targetMap.size() << endl;
            cerr << "initialMap size: " << RePoStBlockCode::initialMap.size() << endl;

            if (RePoStBlockCode::initialMap.size() == RePoStBlockCode::targetMap.size()) {
                for (auto block : BaseSimulator::getWorld()->buildingBlocksMap) {
                    RePoStBlockCode *MMblock =
                        static_cast<RePoStBlockCode *>(block.second->blockCode);
                    //Start MaxFlow
                    if (MMblock->isSource and MMblock->seedPosition == MMblock->module->position) {
                        cerr << MMblock->MMPosition << ": is source\n";
                        MMblock->state = ACTIVE;
                        MMblock->mainPathState = BFS;
                        MMblock->mainPathIn = MMblock->MMPosition;
                        MMblock->mainPathsOld.push_back(MMblock->MMPosition);

                        // Start MaxFlow
                        for (const auto& p : MMblock->getAdjacentMMSeeds()) {
                            cerr << "MMPosition"
                                 << ": " << p << endl;
                            RePoStBlockCode *toSeed = static_cast<RePoStBlockCode *>(
                                BaseSimulator::getWorld()->getBlockByPosition(p)->blockCode);
                            MMblock->deficit++;
                            MMblock->sendHandleableMessage(
                                new BFSMessage(MMblock->MMPosition, toSeed->MMPosition,
                                               MMblock->MMPosition),
                                MMblock->interfaceTo(MMblock->MMPosition, toSeed->MMPosition), 100,
                                200);
                        }
                    }
                }
                rbc.start_wave();
                rbc.switchModulesColors();
            } else if (RePoStBlockCode::targetMap.size() < RePoStBlockCode::initialMap.size()) {
                // choose one source and find non blocking destinations to be filled
                cerr << "Nb of potential sources: " << RePoStBlockCode::NbOfPotentialSources << endl;
                for (auto MMPos : RePoStBlockCode::initialMap) {
                    Cell3DPosition MMPos1 = Cell3DPosition(MMPos[0], MMPos[1], MMPos[2]);
                    if(not rbc.lattice->cellHasBlock((rbc.getSeedPositionFromMMPosition(MMPos1)))) continue;
                    RePoStBlockCode *MMBlock = dynamic_cast<RePoStBlockCode *>(
                        rbc.lattice->getBlock(rbc.getSeedPositionFromMMPosition(MMPos1))
                            ->blockCode);
                    if (MMBlock->isSource) {
                        MMBlock->nbWaitedAnswers = 0;
                        for (auto p : MMBlock->getAdjacentMMSeeds()) {
                            Cell3DPosition toMMPosition =
                                static_cast<RePoStBlockCode *>(
                                    BaseSimulator::getWorld()->getBlockByPosition(p)->blockCode)
                                    ->MMPosition;
                            MMBlock->console << toMMPosition << "\n";
                            MMBlock->sendHandleableMessage(
                                new GoDstMessage(MMBlock->MMPosition, toMMPosition,
                                              MMBlock->distanceDst),
                                MMBlock->interfaceTo(MMBlock->MMPosition, toMMPosition), 100, 200);
                            MMBlock->nbWaitedAnswers++;
                        }
                        return;
                    }
                }
            }

        } else {
            rbc.sendHandleableMessage(new BackMessage(rbc.MMPosition, rbc.parentPosition, true),
                                      rbc.interfaceTo(rbc.MMPosition, rbc.parentPosition), 100,
                                      200);
        }
    }
}

void GoDstMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *static_cast<RePoStBlockCode *>(bc);

    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        if (not rbc.interfaceTo(fromMMPosition, toMMPosition)) VS_ASSERT(false);
        rbc.sendHandleableMessage(new GoDstMessage(fromMMPosition, toMMPosition, distance),
                                  rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    rbc.console << "received: " << this->getName() << "\n";
    rbc.console << "distanceDst: " << rbc.distanceDst << "\n";
    //rbc.nbWaitedAnswers = 0;
    if (rbc.fillingState == FULL or distance >= RePoStBlockCode::NbOfPotentialSources) {
        // ignore Full MM while building the tree
        rbc.sendHandleableMessage(new BackDstMessage(rbc.MMPosition, fromMMPosition, false),
                                  rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
        return;
    }
    if (rbc.parentPositionDst == Cell3DPosition(-1, -1, -1)) {
        rbc.parentPositionDst = fromMMPosition;
        // distance = data->distance + 1;
        rbc.isPotentialDestination() ? rbc.distanceDst = distance + 1 : rbc.distanceDst = distance;
        rbc.console << "distance1: " << rbc.distanceDst << "\n";
        rbc.nbWaitedAnswers = 0;
        for (auto p: rbc.getAdjacentMMSeeds()) {
            Cell3DPosition toMMPosition1 =
                    static_cast<RePoStBlockCode *>(
                            BaseSimulator::getWorld()->getBlockByPosition(p)->blockCode)
                            ->MMPosition;
            if (toMMPosition1 == fromMMPosition) continue;
            rbc.sendHandleableMessage(new GoDstMessage(rbc.MMPosition, toMMPosition1, rbc.distanceDst),
                                      rbc.interfaceTo(rbc.MMPosition, toMMPosition1), 100, 200);

            rbc.nbWaitedAnswers++;
        }
        rbc.console << "nbWaitedAnswers: " << rbc.nbWaitedAnswers << "\n";

        if (rbc.nbWaitedAnswers == 0) {
            rbc.sendHandleableMessage(new BackDstMessage(rbc.MMPosition, rbc.parentPositionDst, true),
                                      rbc.interfaceTo(rbc.MMPosition, rbc.parentPositionDst), 100,
                                      200);
        }
    } else if ((not rbc.isPotentialDestination() and distance < rbc.distanceDst) or
               (rbc.isPotentialDestination() and distance + 1 < rbc.distanceDst)) {

        rbc.sendHandleableMessage(new BackDstMessage(rbc.MMPosition, rbc.parentPositionDst, false),
                                  rbc.interfaceTo(rbc.MMPosition, rbc.parentPositionDst), 100, 200);

        rbc.parentPositionDst = fromMMPosition;
        rbc.childrenPositionsDst.clear();
        rbc.isPotentialDestination() ? rbc.distanceDst = distance + 1 : rbc.distanceDst = distance;
        rbc.console << "distance2 : " << rbc.distanceDst << "\n";

        rbc.nbWaitedAnswers = 0;
        for (auto p: rbc.getAdjacentMMSeeds()) {
            Cell3DPosition toMMPosition1 =
                    static_cast<RePoStBlockCode *>(
                            BaseSimulator::getWorld()->getBlockByPosition(p)->blockCode)
                            ->MMPosition;
            if (toMMPosition1 == fromMMPosition) continue;
            rbc.sendHandleableMessage(new GoDstMessage(rbc.MMPosition, toMMPosition1, rbc.distanceDst),
                                      rbc.interfaceTo(rbc.MMPosition, toMMPosition1), 100, 200);
            rbc.nbWaitedAnswers++;
        }
        if (rbc.nbWaitedAnswers == 0) {
            rbc.sendHandleableMessage(new BackDstMessage(rbc.MMPosition, rbc.parentPositionDst, true),
                                      rbc.interfaceTo(rbc.MMPosition, rbc.parentPositionDst), 100,
                                      200);
        }
    } else {
        rbc.sendHandleableMessage(new BackDstMessage(rbc.MMPosition, fromMMPosition, false),
                                  rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
    }
}

void BackDstMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *static_cast<RePoStBlockCode *>(bc);

    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        rbc.sendHandleableMessage(static_cast<HandleableMessage *>(this->clone()),
                                  rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }

    rbc.nbWaitedAnswers--;
    rbc.console << "nbWaitedAnswers: " << rbc.nbWaitedAnswers << "\n";
    auto it =
        find(rbc.childrenPositionsDst.begin(), rbc.childrenPositionsDst.end(), fromMMPosition);
    if (isChild and it == rbc.childrenPositionsDst.end()) {
        rbc.childrenPositionsDst.push_back(fromMMPosition);
        if(rbc.isPotentialDestination())
            rbc.isDestination = false;
    } else if (not isChild) {
        if (it != rbc.childrenPositionsDst.end()) {
            rbc.childrenPositionsDst.erase(it);
        }
    }
    if (rbc.nbWaitedAnswers == 0) {
        if (rbc.parentPositionDst == Cell3DPosition(-1, -1, -1)) {
           // Start MaxFlow
            for (auto block : BaseSimulator::getWorld()->buildingBlocksMap) {
                auto *MMblock =
                    dynamic_cast<RePoStBlockCode *>(block.second->blockCode);
                //Start MaxFlow
                if (MMblock->isSource and MMblock->seedPosition == MMblock->module->position) {
                    cerr << MMblock->MMPosition << ": is source\n";
                    MMblock->state = ACTIVE;
                    MMblock->mainPathState = BFS;
                    MMblock->mainPathIn = MMblock->MMPosition;
                    MMblock->mainPathsOld.push_back(MMblock->MMPosition);

                    // Start MaxFlow
                    for (auto p : MMblock->getAdjacentMMSeeds()) {
                        cerr << "MMPosition"
                                << ": " << p << endl;
                        RePoStBlockCode *toSeed = static_cast<RePoStBlockCode *>(
                            BaseSimulator::getWorld()->getBlockByPosition(p)->blockCode);
                        MMblock->deficit++;
                        MMblock->sendHandleableMessage(
                            new BFSMessage(MMblock->MMPosition, toSeed->MMPosition,
                                            MMblock->MMPosition),
                            MMblock->interfaceTo(MMblock->MMPosition, toSeed->MMPosition), 100,
                            200);
                    }
                }
            }
            static_cast<RePoStBlockCode*>(RePoStBlockCode::GC->blockCode)->start_wave();
            rbc.switchModulesColors();
 
        } else {
            rbc.sendHandleableMessage(new BackDstMessage(rbc.MMPosition, rbc.parentPositionDst, true),
                                      rbc.interfaceTo(rbc.MMPosition, rbc.parentPositionDst), 100,
                                      200);
        }
    }
}
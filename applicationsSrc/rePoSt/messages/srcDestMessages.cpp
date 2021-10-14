#include "srcDestMessages.hpp"

#include "maxFlowMessages.hpp"
#include "rePoStBlockCode.hpp"

void GoMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode *rbc = static_cast<RePoStBlockCode *>(bc);
    rbc->console << "received: " << this->getName() << "\n";
    Cell3DPosition toSeedPosition = rbc->getSeedPositionFromMMPosition(toMMPosition);
    if (rbc->module->position != toSeedPosition) {
        if (not rbc->interfaceTo(fromMMPosition, toMMPosition)) VS_ASSERT(false);
        rbc->sendHandleableMessage(new GoMessage(fromMMPosition, toMMPosition, distance),
                                   rbc->interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }

    if (rbc->parentPosition == Cell3DPosition(-1, -1, -1) and
        rbc->module->blockId != RePoStBlockCode::GC->blockId) {
        rbc->parentPosition = fromMMPosition;
        // distance = data->distance + 1;
        rbc->isPotentialSource() ? rbc->distance = distance + 1 : rbc->distance = distance;
        rbc->console << "distance1: " << rbc->distance << "\n";
        rbc->nbWaitedAnswers = 0;
        for (auto p : rbc->getAdjacentMMSeeds()) {
            Cell3DPosition toMMPosition1 =
                static_cast<RePoStBlockCode *>(
                    BaseSimulator::getWorld()->getBlockByPosition(p)->blockCode)
                    ->MMPosition;
            if (toMMPosition1 == fromMMPosition) continue;
            rbc->sendHandleableMessage(new GoMessage(rbc->MMPosition, toMMPosition1, rbc->distance),
                                       rbc->interfaceTo(rbc->MMPosition, toMMPosition1), 100, 200);

            rbc->nbWaitedAnswers++;
        }
        if (rbc->nbWaitedAnswers == 0) {
            Cell3DPosition toSeedPosition = rbc->getSeedPositionFromMMPosition(rbc->parentPosition);
            rbc->sendHandleableMessage(new BackMessage(rbc->MMPosition, rbc->parentPosition, true),
                                       rbc->interfaceTo(rbc->MMPosition, rbc->parentPosition), 100,
                                       200);
        }
    } else if ((not rbc->isPotentialSource() and distance < rbc->distance) or
               (rbc->isPotentialSource() and distance + 1 < rbc->distance)) {
        Cell3DPosition toSeedPosition = rbc->getSeedPositionFromMMPosition(rbc->parentPosition);
        rbc->sendHandleableMessage(new BackMessage(rbc->MMPosition, rbc->parentPosition, false),
                                   rbc->interfaceTo(rbc->MMPosition, rbc->parentPosition), 100,
                                   200);

        rbc->parentPosition = fromMMPosition;
        rbc->childrenPositions.clear();
        rbc->isPotentialSource() ? rbc->distance = distance + 1 : rbc->distance = distance;
        rbc->console << "distance2 : " << rbc->distance << "\n";

        rbc->nbWaitedAnswers = 0;
        for (auto p : rbc->getAdjacentMMSeeds()) {
            Cell3DPosition toMMPosition1 =
                static_cast<RePoStBlockCode *>(
                    BaseSimulator::getWorld()->getBlockByPosition(p)->blockCode)
                    ->MMPosition;
            if (toMMPosition1 == fromMMPosition) continue;
            rbc->sendHandleableMessage(new GoMessage(rbc->MMPosition, toMMPosition1, rbc->distance),
                                       rbc->interfaceTo(rbc->MMPosition, toMMPosition1), 100, 200);
            rbc->nbWaitedAnswers++;
        }
        if (rbc->nbWaitedAnswers == 0) {
            Cell3DPosition toSeedPosition = rbc->getSeedPositionFromMMPosition(rbc->parentPosition);
            rbc->sendHandleableMessage(new BackMessage(rbc->MMPosition, rbc->parentPosition, true),
                                       rbc->interfaceTo(rbc->MMPosition, rbc->parentPosition), 100,
                                       200);
        }
    } else {
        Cell3DPosition toSeedPosition = rbc->getSeedPositionFromMMPosition(fromMMPosition);
        rbc->sendHandleableMessage(new BackMessage(rbc->MMPosition, fromMMPosition, false),
                                   rbc->interfaceTo(rbc->MMPosition, fromMMPosition), 100, 200);
    }
    rbc->console << "parent: " << rbc->parentPosition << "\n";
}

void BackMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode *rbc = static_cast<RePoStBlockCode *>(bc);
    Cell3DPosition toSeedPosition = rbc->getSeedPositionFromMMPosition(toMMPosition);
    if (rbc->module->position != toSeedPosition) {
        rbc->sendHandleableMessage(static_cast<HandleableMessage *>(this->clone()),
                                   rbc->interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    rbc->nbWaitedAnswers--;
    rbc->console << "nbWaitedAnswers: " << rbc->nbWaitedAnswers << "\n";
    vector<Cell3DPosition>::iterator it =
        find(rbc->childrenPositions.begin(), rbc->childrenPositions.end(), fromMMPosition);
    if (isChild and it == rbc->childrenPositions.end()) {
        rbc->childrenPositions.push_back(fromMMPosition);
    } else if (not isChild) {
        if (it != rbc->childrenPositions.end()) {
            rbc->childrenPositions.erase(it);
        }
    }
    if (rbc->nbWaitedAnswers == 0) {
        if (rbc->parentPosition == Cell3DPosition(-1, -1, -1) and
            rbc->module->position == RePoStBlockCode::GC->position) {
            cerr << rbc->module->blockId << ": Coordination Tree is Built\n";
            reconfigurationStep = MAXFLOW;
            destinations.clear();
            for (auto block : BaseSimulator::getWorld()->buildingBlocksMap) {
                RePoStBlockCode *MMblock = static_cast<RePoStBlockCode *>(block.second->blockCode);
                if (MMblock->isPotentialSource() and
                    MMblock->seedPosition == MMblock->module->position and
                    MMblock->childrenPositions.empty()) {
                    MMblock->isSource = true;
                    cerr << MMblock->MMPosition << ": is source\n";
                    // VS_ASSERT(mainPathState == NONE);
                    MMblock->state = ACTIVE;
                    MMblock->mainPathState = BFS;
                    MMblock->mainPathIn = MMblock->MMPosition;
                    MMblock->mainPathsOld.push_back(MMblock->MMPosition);
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
                if (MMblock->isPotentialDestination() and
                    MMblock->seedPosition == MMblock->module->position) {
                    if (find(destinations.begin(), destinations.end(), MMblock->destinationOut) ==
                        destinations.end()) {
                        cerr << MMblock->MMPosition << ": is destination\n";
                        MMblock->isDestination = true;
                        destinations.push_back(MMblock->destinationOut);
                    }
                }
            }
            rbc->start_wave();
            rbc->switchModulesColors();
        } else {
            Cell3DPosition toSeedPosition = rbc->getSeedPositionFromMMPosition(rbc->parentPosition);
            rbc->sendHandleableMessage(new BackMessage(rbc->MMPosition, rbc->parentPosition, true),
                                       rbc->interfaceTo(rbc->MMPosition, rbc->parentPosition), 100,
                                       200);
        }
    }
}

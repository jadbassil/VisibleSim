#include "maxFlowMessages.hpp"
#include "transportMessages.hpp"
#include "rePoStBlockCode.hpp"

/* -------------------------------------------------------------------------- */
/*                       Termination Detection Messages                       */
/* -------------------------------------------------------------------------- */
void GoTermMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode& rbc = *static_cast<RePoStBlockCode *>(bc);

    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        rbc.sendHandleableMessage(static_cast<GoTermMessage *>(this->clone()),
                                   rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    rbc.nbWaitedAnswers = 0;
    for (auto child : rbc.childrenPositions) {
        Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(child);
        rbc.sendHandleableMessage(new GoTermMessage(rbc.MMPosition, child),
                                   rbc.interfaceTo(rbc.MMPosition, child), 100, 200);
        rbc.nbWaitedAnswers++;
    }
    if (rbc.nbWaitedAnswers == 0) {
        if (rbc.state == PASSIVE and rbc.deficit == 0) {
            rbc.b = rbc.cont_passive;
            Cell3DPosition parentSeedPosition =
                rbc.getSeedPositionFromMMPosition(rbc.parentPosition);
            rbc.sendHandleableMessage(
                new BackTermMessage(rbc.MMPosition, rbc.parentPosition, rbc.b),
                rbc.interfaceTo(rbc.MMPosition, rbc.parentPosition), 100, 200);
            rbc.cont_passive = true;
        } else {
            getScheduler()->schedule(new InterruptionEvent(getScheduler()->now() + 500, rbc.module,
                                                           IT_MODE_TERMINATION));
        }
    }
}

void BackTermMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *static_cast<RePoStBlockCode *>(bc);
    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        rbc.sendHandleableMessage(static_cast<BackTermMessage *>(this->clone()),
                                   rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    rbc.res = rbc.res and ans;
    rbc.nbWaitedAnswers--;
    rbc.console << "Rec. BackTerm " << rbc.nbWaitedAnswers << "\n";
    if (rbc.nbWaitedAnswers == 0) {
        if (rbc.state == PASSIVE and rbc.deficit == 0) {
            rbc.b = rbc.cont_passive;
            rbc.cont_passive = true;
            if (rbc.parentPosition != Cell3DPosition(-1, -1, -1)) {
                Cell3DPosition parentSeedPosition =
                    rbc.getSeedPositionFromMMPosition(rbc.parentPosition);
                rbc.sendHandleableMessage(
                    new BackTermMessage(rbc.MMPosition, rbc.parentPosition, rbc.res and rbc.b),
                    rbc.interfaceTo(rbc.MMPosition, rbc.parentPosition), 100, 200);

            } else {
                rbc.console << "res: " << rbc.res << "; b: " << rbc.b << "\n";
                if (rbc.res and rbc.b) {
                    //Start transportation phase
                    cerr << "MaxFlow Terminated!!" << endl;
                    cerr << "Starting Modules Transportation" << endl;
                    reconfigurationStep = TRANSPORT;
                    bool end = true;
                    for (auto id_block : BaseSimulator::getWorld()->buildingBlocksMap) {
                        auto *block =
                            dynamic_cast<RePoStBlockCode *>(id_block.second->blockCode);
                        if (block->isSource and block->mainPathState == Streamline and
                            block->module->position == block->seedPosition) {
                            end = false;
                            auto *coord = dynamic_cast<RePoStBlockCode *>(
                                BaseSimulator::getWorld()
                                    ->getBlockByPosition(block->coordinatorPosition)
                                    ->blockCode);
                            Cell3DPosition targetModule =
                                block->seedPosition +
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

                    if (end) {
                        reconfigurationStep = DONE;
                        cout << "Nb of iterations: " << RePoStBlockCode::nbOfIterations - 1 << "\n";
                    }
                } else {
                    rbc.start_wave();
                }
            }
        } else {
            getScheduler()->schedule(new InterruptionEvent(getScheduler()->now() + 500, rbc.module,
                                                           IT_MODE_TERMINATION));
        }
    }
    rbc.res = true;
}

void AckMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *static_cast<RePoStBlockCode *>(bc);
    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        rbc.sendHandleableMessage(static_cast<AckMessage *>(this->clone()),
                                   rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    } else {
        rbc.console << "Rec: Ack from: " << fromMMPosition << "\n";
        rbc.deficit--;
    }
}
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                              MaxFlow Messages                              */
/* -------------------------------------------------------------------------- */
void BFSMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *static_cast<RePoStBlockCode *>(bc);
    if (rbc.module->position != rbc.getSeedPositionFromMMPosition(toMMPosition)) {
        // forward the message to target MML
        rbc.sendHandleableMessage(static_cast<BFSMessage *>(this->clone()),
                                   rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
/*    if(fromMMPosition == toMMPosition.offsetY(1)) {
        rbc.state = PASSIVE;
        return;
    }*/
    vector<Cell3DPosition> pathsOld;
    pathsOld += rbc.mainPathsOld;
    pathsOld += rbc.aug1PathsOld;
    pathsOld += rbc.aug2PathsOld;
    rbc.console << "pathsOld: ";
    for (auto pOld : pathsOld) rbc.console << pOld << "; ";
    rbc.console << "\n";

    if(rbc.fillingState == FULL) {
        rbc.state = PASSIVE;
        return;
    }

    if (rbc.mainPathState == NONE and !isIn(pathsOld, sourceMM)) {
        rbc.mainPathsOld.push_back(sourceMM);
        Cell3DPosition fromSeedPosition = rbc.getSeedPositionFromMMPosition(fromMMPosition);
        rbc.deficit++;
        rbc.sendHandleableMessage(new ConfirmEdgeMessage(rbc.MMPosition, fromMMPosition),
                                   rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
        if (rbc.isDestination) {
            rbc.mainPathState = ConfPath;
            rbc.mainPathIn = fromMMPosition;
            rbc.mainPathOut.clear();
            // mainPathOut.push_back(MMPosition);
            rbc.console << "fromSeedPosition: " << fromSeedPosition << "\n";
            rbc.deficit++;
            rbc.sendHandleableMessage(new ConfirmPathMessage(rbc.MMPosition, fromMMPosition),
                                       rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
        } else {
            rbc.mainPathState = BFS;
            rbc.mainPathIn = fromMMPosition;
            rbc.mainPathOut.clear();
            for (auto p : rbc.getAdjacentMMSeeds()) {
                rbc.console << "Send Around\n";
                RePoStBlockCode *toSeed = static_cast<RePoStBlockCode *>(
                    BaseSimulator::getWorld()->getBlockByPosition(p)->blockCode);
                rbc.deficit++;
                rbc.sendHandleableMessage(
                    new BFSMessage(rbc.MMPosition, toSeed->MMPosition, sourceMM),
                    rbc.interfaceTo(rbc.MMPosition, toSeed->MMPosition), 100, 200);
            }
        }
    } else if (rbc.mainPathState == Streamline and rbc.aug1PathState == NONE and
               fromMMPosition != rbc.mainPathIn and fromMMPosition != rbc.mainPathOut and
               !isIn(pathsOld, sourceMM)) {
        rbc.aug1PathsOld.push_back(sourceMM);
        Cell3DPosition fromSeedPosition = rbc.getSeedPositionFromMMPosition(fromMMPosition);
        rbc.deficit++;
        rbc.sendHandleableMessage(new ConfirmEdgeMessage(rbc.MMPosition, fromMMPosition),
                                   rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
        rbc.aug1PathState = BFS;
        rbc.aug1PathIn = fromMMPosition;
        rbc.aug1PathOut.clear();
        Cell3DPosition mainPathInSeedPosition = rbc.getSeedPositionFromMMPosition(rbc.mainPathIn);
        if (rbc.mainPathIn != rbc.MMPosition) {
            rbc.deficit++;
            rbc.sendHandleableMessage(new BFSMessage(rbc.MMPosition, rbc.mainPathIn, sourceMM),
                                       rbc.interfaceTo(rbc.MMPosition, rbc.mainPathIn), 100,
                                       200);
        }

    } else if (rbc.mainPathState == Streamline and rbc.aug2PathState == NONE and
               fromMMPosition == rbc.mainPathOut) {
        rbc.aug2PathsOld.push_back(sourceMM);
        Cell3DPosition fromSeedPosition = rbc.getSeedPositionFromMMPosition(fromMMPosition);
        rbc.deficit++;
        rbc.sendHandleableMessage(new ConfirmEdgeMessage(rbc.MMPosition, fromMMPosition),
                                   rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
        rbc.aug2PathState = BFS;
        rbc.aug2PathIn = fromMMPosition;
        rbc.aug2PathOut.clear();
        for (auto p : rbc.getAdjacentMMSeeds()) {
            rbc.console << "Send Around\n";
            Cell3DPosition seedMMPosition =
                static_cast<RePoStBlockCode *>(
                    BaseSimulator::getWorld()->getBlockByPosition(p)->blockCode)
                    ->MMPosition;
            if (seedMMPosition == fromMMPosition) continue;
            rbc.deficit++;
            rbc.sendHandleableMessage(new BFSMessage(rbc.MMPosition, seedMMPosition, sourceMM),
                                       rbc.interfaceTo(rbc.MMPosition, seedMMPosition), 100, 200);
        }
    } else {
        rbc.console << rbc.MMPosition << ": path can no longer be augmented"
                     << "\n";
    }
    rbc.state = PASSIVE;
}

void ConfirmEdgeMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *static_cast<RePoStBlockCode *>(bc);
    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        rbc.sendHandleableMessage(static_cast<ConfirmEdgeMessage *>(this->clone()),
                                   rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    if (rbc.mainPathState == BFS) {
        rbc.mainPathOut.push_back(fromMMPosition);
    } else if (rbc.aug1PathState == BFS) {
        rbc.aug1PathOut.push_back(fromMMPosition);
    } else if (rbc.aug2PathState == BFS) {
        rbc.aug2PathOut.push_back(fromMMPosition);
    } else {
        rbc.deficit++;
        Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(fromMMPosition);
        rbc.sendHandleableMessage(new CutOffMessage(rbc.MMPosition, fromMMPosition),
                                   rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
    }
    rbc.state = PASSIVE;
}

void ConfirmPathMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *static_cast<RePoStBlockCode *>(bc);
    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        rbc.sendHandleableMessage(static_cast<ConfirmPathMessage *>(this->clone()),
                                   rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    if (rbc.mainPathState == BFS and isIn(rbc.mainPathOut, fromMMPosition)) {
        for (auto out : rbc.mainPathOut) {
            if (out != fromMMPosition and not isIn(rbc.aug1PathOut, out) and
                not isIn(rbc.aug2PathOut, out)) {
                rbc.deficit++;
                Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(out);
                rbc.sendHandleableMessage(new CutOffMessage(rbc.MMPosition, out),
                                           rbc.interfaceTo(rbc.MMPosition, out), 100, 200);
            }
        }
        rbc.mainPathOut.clear();
        rbc.mainPathOut.push_back(fromMMPosition);
        if (rbc.isSource) {
            cerr << "Streamline from source: " << rbc.MMPosition << " is established" << endl;
            rbc.mainPathState = Streamline;
            Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(fromMMPosition);
            rbc.deficit++;
            rbc.sendHandleableMessage(
                new ConfirmStreamlineMessage(rbc.MMPosition, fromMMPosition),
                rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
            rbc.setOperation(rbc.mainPathIn, rbc.mainPathOut.front());
        } else {
            rbc.mainPathState = ConfPath;
            rbc.deficit++;
            rbc.sendHandleableMessage(new ConfirmPathMessage(rbc.MMPosition, rbc.mainPathIn),
                                       rbc.interfaceTo(rbc.MMPosition, rbc.mainPathIn), 100,
                                       200);
        }
    } else if (rbc.aug1PathState == BFS and fromMMPosition == rbc.mainPathIn) {
        rbc.aug1PathOut.clear();
        rbc.aug1PathOut.push_back(fromMMPosition);
        rbc.aug1PathState = ConfPath;
        rbc.deficit++;
        rbc.sendHandleableMessage(new ConfirmPathMessage(rbc.MMPosition, rbc.aug1PathIn),
                                   rbc.interfaceTo(rbc.MMPosition, rbc.aug1PathIn), 100, 200);
    } else if (rbc.aug2PathState == BFS and
               (rbc.aug1PathState != ConfPath or fromMMPosition != rbc.mainPathIn)) {
        for (auto out : rbc.aug2PathOut) {
            if (out != fromMMPosition) {
                rbc.deficit++;
                rbc.sendHandleableMessage(new CutOffMessage(rbc.MMPosition, out),
                                           rbc.interfaceTo(rbc.MMPosition, out), 100, 200);
            }
        }
        rbc.aug2PathOut.clear();
        rbc.aug2PathOut.push_back(fromMMPosition);
        rbc.aug2PathState = ConfPath;
        rbc.deficit++;
        rbc.sendHandleableMessage(new ConfirmPathMessage(rbc.MMPosition, rbc.aug2PathIn),
                                   rbc.interfaceTo(rbc.MMPosition, rbc.aug2PathIn), 100, 200);
    }
    rbc.state = PASSIVE;
}

void ConfirmStreamlineMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *static_cast<RePoStBlockCode *>(bc);
    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        rbc.deficit++;
        rbc.sendHandleableMessage(static_cast<ConfirmStreamlineMessage *>(this->clone()),
                                   rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    if (rbc.mainPathState == ConfPath and fromMMPosition == rbc.mainPathIn) {
        rbc.mainPathState = Streamline;
        if (not rbc.isDestination) {
            rbc.deficit++;
            rbc.sendHandleableMessage(
                new ConfirmStreamlineMessage(rbc.MMPosition, rbc.mainPathOut.front()),
                rbc.interfaceTo(rbc.MMPosition, rbc.mainPathOut.front()), 100, 200);
        } else {
            RePoStBlockCode::NbOfStreamlines++;
            cerr << "NbOfStreamlines: " << RePoStBlockCode::NbOfStreamlines << endl << endl;
        }
        rbc.deficit++;
        rbc.sendHandleableMessage(new AvailableMessage(rbc.MMPosition, rbc.mainPathIn),
                                   rbc.interfaceTo(rbc.MMPosition, rbc.mainPathIn), 100, 200);
        for (auto out : rbc.mainPathOut) {
            rbc.console << "Send Available MainPathOut\n";
            rbc.deficit++;
            rbc.sendHandleableMessage(new AvailableMessage(rbc.MMPosition, out),
                                       rbc.interfaceTo(rbc.MMPosition, out), 100, 200);
        }
    } else if (rbc.aug1PathState == ConfPath and fromMMPosition == rbc.aug1PathIn) {
        rbc.mainPathIn = rbc.aug1PathIn;
        VS_ASSERT(not rbc.aug1PathOut.empty());
        rbc.deficit++;
        rbc.sendHandleableMessage(
            new ConfirmStreamlineMessage(rbc.MMPosition, rbc.aug1PathOut.front()),
            rbc.interfaceTo(rbc.MMPosition, rbc.aug1PathOut.front()), 100, 200);
        rbc.deficit++;
        rbc.sendHandleableMessage(new AvailableMessage(rbc.MMPosition, rbc.aug1PathIn),
                                   rbc.interfaceTo(rbc.MMPosition, rbc.aug1PathIn), 100, 200);
        for (auto out : rbc.aug1PathOut) {
            rbc.console << "Send Around Available aug1PathOut\n";
            rbc.deficit++;
            rbc.sendHandleableMessage(new AvailableMessage(rbc.MMPosition, out),
                                       rbc.interfaceTo(rbc.MMPosition, out), 100, 200);
        }
        rbc.aug1PathState = NONE;
        rbc.aug1PathOut.clear();
        rbc.aug1PathIn.set(-1, -1, -1);
    } else if (rbc.aug2PathState == ConfPath and fromMMPosition == rbc.aug2PathIn) {
        VS_ASSERT(not rbc.aug2PathOut.empty());
        rbc.deficit++;
        rbc.sendHandleableMessage(
            new ConfirmStreamlineMessage(rbc.MMPosition, rbc.aug2PathOut.front()),
            rbc.interfaceTo(rbc.MMPosition, rbc.aug2PathOut.front()), 100, 200);
        rbc.deficit++;
        rbc.sendHandleableMessage(new AvailableMessage(rbc.MMPosition, rbc.aug2PathIn),
                                   rbc.interfaceTo(rbc.MMPosition, rbc.aug2PathIn), 100, 200);
        for (auto out : rbc.aug2PathOut) {
            rbc.console << "Send Around Available aug2PathOut\n";
            rbc.deficit++;
            rbc.sendHandleableMessage(new AvailableMessage(rbc.MMPosition, out),
                                       rbc.interfaceTo(rbc.MMPosition, out), 100, 200);
        }
        if (rbc.aug2PathOut.front() == rbc.mainPathIn) {
            rbc.mainPathState = NONE;
            rbc.mainPathIn.set(-1, -1, -1);
            rbc.mainPathOut.clear();
            rbc.aug1PathState = NONE;
            rbc.aug1PathOut.clear();
            rbc.aug1PathIn.set(-1, -1, -1);
        } else {
            rbc.mainPathOut = rbc.aug2PathOut;
        }
        rbc.aug2PathState = NONE;
        rbc.aug2PathOut.clear();
        rbc.aug2PathIn.set(-1, -1, -1);
    } else {
        VS_ASSERT(false);
    }
    if (not rbc.isDestination)
        rbc.setOperation(rbc.mainPathIn, rbc.mainPathOut.front());
    else
        rbc.setOperation(rbc.mainPathIn, rbc.destinationOut);
    rbc.state = PASSIVE;
}

void AvailableMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *static_cast<RePoStBlockCode *>(bc);
    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        rbc.sendHandleableMessage(static_cast<AvailableMessage *>(this->clone()),
                                   rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    if (rbc.mainPathState == BFS) {
        if (!rbc.mainPathsOld.empty()) {
            rbc.deficit++;
            rbc.sendHandleableMessage(
                new BFSMessage(rbc.MMPosition, fromMMPosition, rbc.mainPathsOld.back()),
                rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
        }
    } else if (rbc.aug1PathState == BFS and fromMMPosition == rbc.mainPathIn) {
        if (!rbc.aug1PathsOld.empty()) {
            rbc.deficit++;
            rbc.sendHandleableMessage(
                new BFSMessage(rbc.MMPosition, fromMMPosition, rbc.aug1PathsOld.back()),
                rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
        }
    } else if (rbc.aug2PathState == BFS and fromMMPosition == rbc.mainPathOut) {
        if (!rbc.aug2PathsOld.empty()) {
            rbc.deficit++;
            rbc.sendHandleableMessage(
                new BFSMessage(rbc.MMPosition, fromMMPosition, rbc.aug2PathsOld.back()),
                rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
        }
    } else {
        rbc.console << rbc.MMPosition << ": path can no longer be augmented"
                     << "\n";
    }
    rbc.state = PASSIVE;
}

void CutOffMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *static_cast<RePoStBlockCode *>(bc);
    bool isMainPathRemoved = false;
    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        rbc.sendHandleableMessage(static_cast<CutOffMessage *>(this->clone()),
                                   rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    //       console << "mainPathState: " << mainPathState << "\n";
    //     console << "aug1PathState: " << aug1PathState << "\n";
    //     console << "aug2PathState: " << aug2PathState << "\n";
    //      console << "mainPathIn: " << mainPathIn << "\n";
    //     console << "mainPathOut: ";
    //     for(auto out: mainPathOut) console << out << " | ";
    //     console << "\n";
    //     console << "aug1PathIn: " << aug1PathIn << "\n";
    //     console << "aug1PathOut: ";
    //     for(auto out: aug1PathOut) console << out << " | ";
    //     console << "\n";
    //     console << "aug2PathIn: " << aug2PathIn << "\n";
    //     console << "aug2PathOut: ";
    //     for(auto out: aug2PathOut) console << out << " | ";
    //     console << "\n";
    if (rbc.mainPathState != NONE and fromMMPosition == rbc.mainPathIn) {
        for (auto out : rbc.mainPathOut) {
            //            VS_ASSERT(mainPathOut.size() == 1);
            rbc.console << out;
            rbc.deficit++;
            rbc.sendHandleableMessage(new CutOffMessage(rbc.MMPosition, out),
                                       rbc.interfaceTo(rbc.MMPosition, out), 100, 200);
        }
        rbc.mainPathState = NONE;
        rbc.mainPathOut.clear();
        rbc.mainPathIn.set(-1, -1, -1);
        isMainPathRemoved = true;
    }
    rbc.console << "isMainPathRemoved: " << isMainPathRemoved << "\n";
    if (rbc.aug1PathState != NONE and (fromMMPosition == rbc.aug1PathIn or isMainPathRemoved)) {
        if (rbc.aug1PathOut != rbc.aug2PathOut) {
            for (auto out : rbc.aug1PathOut) {
                if (isIn(rbc.aug2PathOut, out)) continue;

                rbc.deficit++;
                rbc.sendHandleableMessage(new CutOffMessage(rbc.MMPosition, out),
                                           rbc.interfaceTo(rbc.MMPosition, out), 100, 200);
            }
        }

        rbc.aug1PathState = NONE;
        rbc.aug1PathOut.clear();
        rbc.aug1PathIn.set(-1, -1, -1);
    }
    if (rbc.aug2PathState != NONE and (fromMMPosition == rbc.aug2PathIn or isMainPathRemoved)) {
        for (auto out : rbc.aug2PathOut) {
            if (isIn(rbc.aug1PathOut, out)) continue;
            rbc.deficit++;
            rbc.sendHandleableMessage(new CutOffMessage(rbc.MMPosition, out),
                                       rbc.interfaceTo(rbc.MMPosition, out), 100, 200);
        }
        rbc.aug2PathState = NONE;
        rbc.aug2PathOut.clear();
        rbc.aug2PathIn.set(-1, -1, -1);
    }
    for (auto p : rbc.getAdjacentMMSeeds()) {
        Cell3DPosition seedMMPosition =
            static_cast<RePoStBlockCode *>(
                BaseSimulator::getWorld()->getBlockByPosition(p)->blockCode)
                ->MMPosition;
        rbc.console << "Send Around Available: " << seedMMPosition << "\n";
        rbc.deficit++;
        rbc.sendHandleableMessage(new AvailableMessage(rbc.MMPosition, seedMMPosition),
                                   rbc.interfaceTo(rbc.MMPosition, seedMMPosition), 100, 200);
    }
    // console << "mainPathState: " << mainPathState << "\n";
    // console << "aug1PathState: " << aug1PathState << "\n";
    // console << "aug2PathState: " << aug2PathState << "\n";
    rbc.state = PASSIVE;
}

/* -------------------------------------------------------------------------- */
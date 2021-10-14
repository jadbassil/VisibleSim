#include "transportMessages.hpp"
#include "rePoStBlockCode.hpp"

void CoordinateMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode *rbc = static_cast<RePoStBlockCode *>(bc);
    P2PNetworkInterface *sender = this->destinationInterface;
    // console << "Received Coordinate Msg from: " << sender->getConnectedBlockId()
    // << " " << coordinateData->coordinatorPosition   << " " << coordinateData->position<< "\n";

    if (rbc->module->position == position) {
        rbc->console << "Can start moving\n";
        VS_ASSERT(operation);
        rbc->mvt_it = it;
        rbc->coordinatorPosition = coordinatorPosition;
        rbc->operation = operation;
        bool bridgeStop = false;
        if (rbc->operation->isTransfer() or
            (rbc->operation->isDismantle() and
             static_cast<Dismantle_Operation *>(rbc->operation)->filled)) {
            // Special logic to avoid unsupported motions of bridging modules
            bridgeStop =
                static_cast<Transfer_Operation *>(rbc->operation)->handleBridgeMovements(rbc);
        }
        if (bridgeStop) return;
        rbc->probeGreenLight();
    } else {
        if (rbc->module->getInterface(rbc->nearestPositionTo(position, sender))->isConnected()) {
            rbc->sendHandleableMessage(
                static_cast<CoordinateMessage *>(this->clone()),
                rbc->module->getInterface(rbc->nearestPositionTo(position, sender)), 100, 200);
        } else {
            rbc->sendHandleableMessage(static_cast<CoordinateMessage *>(this->clone()), sender, 100,
                                       200);
        }
    }
}

void CoordinateBackMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode *rbc = static_cast<RePoStBlockCode *>(bc);
    P2PNetworkInterface *sender = this->destinationInterface;
    RePoStBlockCode *senderMM = static_cast<RePoStBlockCode *>(
        BaseSimulator::getWorld()->getBlockById(sender->getConnectedBlockId())->blockCode);
    if (senderMM->sendingCoordinateBack) {
        senderMM->sendingCoordinateBack = false;
    }
    if (rbc->module->position == coordinatorPosition) {
        rbc->console << "mvt_it: " << rbc->mvt_it << "\n";
        rbc->console << "steps: " << steps << "\n";
     
        // Add the number of steps to get to the next module's movements
        for (int i = 0; i < steps; i++) {
            rbc->mvt_it++;
        }

        if (rbc->mvt_it >= rbc->operation->localRules->size()) {
            // operation ended
            rbc->isCoordinator = false;
            rbc->console << "Movement Done\n";
            return;
        }
        if ((*rbc->operation->localRules)[rbc->mvt_it].currentPosition + rbc->seedPosition ==
            rbc->module->position) {
            // The coordinator must start its motions (In Dismantle operations)
            // LocalMovement lmvt = (*rbc->operation->localRules)[rbc->mvt_it];
            rbc->probeGreenLight();
            return;
        }

        if ((*rbc->operation->localRules)[rbc->mvt_it - 1].state == MOVING and
            rbc->operation->isTransfer()) {
            rbc->operation->setMvtItToNextModule(rbc->module->blockCode);
        }
        rbc->console << rbc->mvt_it << ": Movement ended must switch to next one "
                     << (*rbc->operation->localRules)[rbc->mvt_it].currentPosition << "\n";
        Cell3DPosition targetModule =
            rbc->seedPosition + (*rbc->operation->localRules)[rbc->mvt_it].currentPosition;

        if (rbc->lattice->cellsAreAdjacent(rbc->module->position, targetModule) and
            not rbc->module->getInterface(targetModule)->isConnected()) {
            // Received coordinateBack and no module is connected
            // Must wait until the module arrives to the starting position
            rbc->awaitingCoordinator = true;
            getScheduler()->schedule(
                new InterruptionEvent(getScheduler()->now() + rbc->getRoundDuration(), rbc->module,
                                      IT_MODE_TRANSFERBACK));
            return;
        }
        VS_ASSERT(rbc->operation);
        rbc->sendHandleableMessage(
            new CoordinateMessage(rbc->operation, targetModule, rbc->module->position, rbc->mvt_it),
            rbc->module->getInterface(rbc->nearestPositionTo(targetModule)), 100, 200);
    } else {  // Forward the message to the coordinator
        P2PNetworkInterface *sender = this->destinationInterface;
        rbc->sendHandleableMessage(
            static_cast<CoordinateBackMessage *>(this->clone()),
            rbc->module->getInterface(rbc->nearestPositionTo(coordinatorPosition, sender)), 100,
            200);
    }
}

void PLSMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode *rbc = static_cast<RePoStBlockCode *>(bc);
    P2PNetworkInterface *sender = this->destinationInterface;
    if (rbc->movingState != MOVING) {
        bool nextToSender = rbc->isAdjacentToPosition(srcPos);
        bool nextToTarget = rbc->isAdjacentToPosition(targetPos);
        bool targetNextToSrc = false;

        if (rbc->module->getState() == BuildingBlock::State::ACTUATING) {
            getScheduler()->trace("light turned orange1", rbc->module->blockId, ORANGE);
            if (rbc->moduleAwaitingGo) rbc->setGreenLight(true);
            rbc->moduleAwaitingGo = true;
            rbc->awaitingModulePos = srcPos;
            rbc->awaitingModuleProbeItf = sender;
            rbc->setGreenLight(false);
            // module->setColor(DARKORANGE);
            return;
        }
        Catoms3DBlock *targetLightNeighbor =
            rbc->findTargetLightAmongNeighbors(targetPos, srcPos, sender);

        if (targetLightNeighbor) {  // Check if targetLightNeighbor has already received this msg
            for (const auto cell :
                 rbc->lattice->getActiveNeighborCells(targetLightNeighbor->position)) {
                if (rbc->module->getInterface(cell) == sender or cell == rbc->module->position)
                    continue;
                if (rbc->lattice->cellsAreAdjacent(cell, srcPos)) {
                    targetNextToSrc = true;
                }
            }
            if (!targetNextToSrc) {
                if (rbc->lattice->cellsAreAdjacent(targetLightNeighbor->position, srcPos)) {
                    targetNextToSrc = true;
                }
            }
            RePoStBlockCode *targetLightNeighborMM =
                static_cast<RePoStBlockCode *>(targetLightNeighbor->blockCode);
            if (targetLightNeighborMM->isCoordinator and
                targetLightNeighborMM->operation->getDirection() == Direction::UP) {
                targetNextToSrc = false;
            }
            rbc->console << "targetNextToSrc: " << targetNextToSrc << "\n";
        }
        if (targetLightNeighbor and targetLightNeighbor->position != srcPos and
            !targetNextToSrc) {  // neighbor is target light
            rbc->console << "Neighbor is target light: " << targetLightNeighbor->position << "\n";
            rbc->console << "target pos: " << targetPos << "\n";
            P2PNetworkInterface *tlitf = rbc->module->getInterface(targetLightNeighbor->position);

            VS_ASSERT(tlitf and tlitf->isConnected());

            // sendMessage("PLS Msg", new MessageOf<PLS>(PLS_MSG_ID, PLS(srcPos, targetPos)), tlitf,
            // 100, 200);
            rbc->sendHandleableMessage(static_cast<PLSMessage *>(this->clone()), tlitf, 100, 200);
        } else if ((not targetLightNeighbor and nextToTarget) or targetNextToSrc) {
            bool mustAvoidBlocking = false;
            if (targetPos - rbc->seedPosition == Cell3DPosition(1, 0, 2) and
                rbc->lattice->cellHasBlock(rbc->seedPosition + Cell3DPosition(1, 1, 2))) {
                // Special test to avoid blocking when a BF meta-module is filling back
                RePoStBlockCode *x = static_cast<RePoStBlockCode *>(
                    BaseSimulator::getWorld()
                        ->getBlockByPosition(rbc->seedPosition + Cell3DPosition(1, 1, 2))
                        ->blockCode);
                if (x->movingState == MOVING) {
                    mustAvoidBlocking = true;
                }
            }
            rbc->console << "mustAvoidBlocking: " << mustAvoidBlocking << "\n\n";
            if (rbc->greenLightIsOn or
                (nextToSender and rbc->module->getState() != BuildingBlock::State::ACTUATING and
                 not mustAvoidBlocking)) {
                P2PNetworkInterface *itf =
                    nextToSender ? rbc->module->getInterface(srcPos) : sender;
                VS_ASSERT(itf and itf->isConnected());
                rbc->console << "test\n";
                rbc->console << srcPos << targetPos << "\n";
                rbc->sendHandleableMessage(new GLOMessage(targetPos, srcPos), itf, 100, 200);
                // rbc->sendMessage("GLO Msg", new MessageOf<PLS>(GLO_MSG_ID, PLS(targetPos,
                // srcPos)),itf, 100, 0);
            } else {
                getScheduler()->trace("light turned orange2", rbc->module->blockId, ORANGE);
                if (rbc->moduleAwaitingGo) rbc->setGreenLight(true);
                rbc->moduleAwaitingGo = true;
                rbc->awaitingModulePos = srcPos;
                rbc->awaitingModuleProbeItf = sender;
                // module->setColor(DARKORANGE);
            }
        } else {  // not neighborNextToTarget and not nextToSender
            rbc->module->setColor(BLACK);
            VS_ASSERT_MSG(false, "error: not neighborNextToTarget and not nextToSender");
        }
    } else {
        rbc->module->setColor(BLACK);
        VS_ASSERT_MSG(false, "Module is moving. Should not receive this message");
    }
}

void GLOMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode *rbc = static_cast<RePoStBlockCode *>(bc);
    P2PNetworkInterface *sender = this->destinationInterface;

    if (!rbc->rotating) {  // module is pivot
        bool nextToDest = rbc->isAdjacentToPosition(targetPos);
        P2PNetworkInterface *itf;
        Cell3DPosition nnCell = Cell3DPosition(0, 0, 0);
        if (not nextToDest) {
            for (const auto &nCell : rbc->lattice->getActiveNeighborCells(rbc->module->position)) {
                if (rbc->lattice->cellsAreAdjacent(nCell, targetPos)) {
                    nnCell = nCell;
                    continue;
                }
            }
        }
        if (nextToDest) {
            itf = rbc->module->getInterface(targetPos);

        } else if (nnCell != Cell3DPosition(0, 0, 0)) {
            itf = rbc->module->getInterface(nnCell);
        } else {
            itf = rbc->module->getInterface(rbc->nearestPositionTo(targetPos));
        }
        rbc->console << "ERROR: " << targetPos << "\n";
        // VS_ASSERT(itf and itf->isConnected());

        rbc->setGreenLight(false);
        if (itf->isConnected())
            rbc->sendHandleableMessage(new GLOMessage(srcPos, targetPos), itf, 100, 200);
        // sendMessage("GLO Msg", new MessageOf<PLS>(GLO_MSG_ID, PLS(srcPos, targetPos)), itf, 100,
        // 0);
    } else if (rbc->module->position == targetPos) {
        VS_ASSERT(rbc->module->position == targetPos);
        Cell3DPosition targetPosition =
            (*rbc->operation->localRules)[rbc->mvt_it].nextPosition + rbc->seedPosition;

        if (rbc->module->canRotateToPosition(
                targetPosition) /**and not lattice->cellIsBlocked(targetPosition)**/) {
            if ((rbc->relativePos() == Cell3DPosition(-1, 1, 2) /**or
                              /ùùrelativePos() == Cell3DPosition(-1, -1, 2)**/) or
                rbc->relativePos() == Cell3DPosition(1, 0, 1) or
                rbc->relativePos() == Cell3DPosition(1, -1, 1) or
                rbc->relativePos() == Cell3DPosition(2, 0, 2) or rbc->relativePos() == Cell3DPosition(1,2,2) or rbc->relativePos() == Cell3DPosition(1,0,2)) {
                // if(relativePos() == Cell3DPosition(1,2,2)) VS_ASSERT(false);
                getScheduler()->schedule(new Catoms3DRotationStartEvent(
                    getScheduler()->now(), rbc->module, targetPosition, RotationLinkType::OctaFace,
                    false));
            } else {
                getScheduler()->schedule(
                    new Catoms3DRotationStartEvent(getScheduler()->now(), rbc->module,
                                                   targetPosition, RotationLinkType::Any, false));
            }

        } else {
            // retry rotating to next position
            //            VS_ASSERT_MSG(false, "Can not rotate to next position");
            rbc->probeGreenLight();
        }
    }
}

void FTRMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode *rbc = static_cast<RePoStBlockCode *>(bc);
    // Cell3DPosition finalPos = *msg->getData();
    if (not rbc->greenLightIsOn) {
        rbc->setGreenLight(true);
        // for (auto n : lattice->getActiveNeighborCells(module->position)) {
        //     if (n == finalPos) continue;
        //     if (lattice->cellsAreAdjacent(n, finalPos) and module->getInterface(n) != sender) {
        //         sendMessage("FTR message", new MessageOf<Cell3DPosition>(FTR_MSG_ID, finalPos),
        //                     module->getInterface(n), 100, 200);
        //     }
        // }
    }
}
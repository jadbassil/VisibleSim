#include "transportMessages.hpp"

#include "rePoStBlockCode.hpp"

void CoordinateMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *static_cast<RePoStBlockCode *>(bc);
    P2PNetworkInterface *sender = this->destinationInterface;
    // console << "Received Coordinate Msg from: " << sender->getConnectedBlockId()
    // << " " << coordinateData->coordinatorPosition   << " " << coordinateData->position<< "\n";
    if(rbc.isAdjacentToPosition(position)) {
        if(not rbc.module->getInterface(position)->isConnected()) {
            VS_ASSERT(false);
            return;
        }
    }

    if (rbc.module->position == position) {
        rbc.console << "Can start moving\n";
        VS_ASSERT(operation);
        rbc.mvt_it = it;
        rbc.coordinatorPosition = coordinatorPosition;
        rbc.operation = operation;
        bool bridgeStop = false;
        if (rbc.operation->isTransfer() or
            (rbc.operation->isDismantle() and
             static_cast<Dismantle_Operation *>(rbc.operation)->filled)) {
            // Special logic to avoid unsupported motions of bridging modules
            bridgeStop =
                static_cast<Transfer_Operation *>(rbc.operation)->handleBridgeMovements(&rbc);
        }
        if (bridgeStop) return;
        rbc.probeGreenLight();
    } else {
        if (rbc.interfaceTo(position, sender)->isConnected()) {
            rbc.sendHandleableMessage(
                static_cast<CoordinateMessage *>(this->clone()),
                rbc.interfaceTo(position, sender), 100, 200);
        } else {
            rbc.sendHandleableMessage(static_cast<CoordinateMessage *>(this->clone()), sender, 100,
                                      200);
        }
    }
}

void CoordinateBackMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *static_cast<RePoStBlockCode *>(bc);
    P2PNetworkInterface *sender = this->destinationInterface;
    RePoStBlockCode *senderMM = static_cast<RePoStBlockCode *>(
        BaseSimulator::getWorld()->getBlockById(sender->getConnectedBlockId())->blockCode);
    if (senderMM->sendingCoordinateBack) {
        senderMM->sendingCoordinateBack = false;
    }
    if (rbc.module->position == coordinatorPosition) {
        rbc.console << "mvt_it: " << rbc.mvt_it << "\n";
        rbc.console << "steps: " << steps << "\n";

        // Add the number of steps to get to the next module's movements
        for (int i = 0; i < steps; i++) {
            rbc.mvt_it++;
        }

    

        if (rbc.mvt_it >= rbc.operation->localRules->size()) {
            // operation ended
            rbc.isCoordinator = false;
            rbc.console << "Movement Done\n";
            return;
        }
        if ((*rbc.operation->localRules)[rbc.mvt_it].currentPosition + rbc.seedPosition ==
            rbc.module->position) {
            // The coordinator must start its motions (In Dismantle operations)
            rbc.probeGreenLight();
            return;
        }

        if(rbc.operation->isFill() and rbc.operation->getDirection() == Direction::LEFT and rbc.transferCount >= 8 and steps == 0) {
            return;
        }

        if ((*rbc.operation->localRules)[rbc.mvt_it - 1].state == MOVING and
            rbc.operation->isTransfer()) {
            rbc.operation->setMvtItToNextModule(rbc.module->blockCode);
        }
        
        rbc.console << rbc.mvt_it << ": Movement ended must switch to next one "
                     << (*rbc.operation->localRules)[rbc.mvt_it].currentPosition << "\n";
        Cell3DPosition targetModule =
            rbc.seedPosition + (*rbc.operation->localRules)[rbc.mvt_it].currentPosition;
        if(not rbc.lattice->cellHasBlock(targetModule) and rbc.operation->isTransfer() and 
            rbc.operation->getDirection() == Direction::LEFT) {
            //check if target module(the module connected to the next module is connected)
            rbc.mvt_it++;
            targetModule =
            rbc.seedPosition + (*rbc.operation->localRules)[rbc.mvt_it].currentPosition;
        }
        if (rbc.lattice->cellsAreAdjacent(rbc.module->position, targetModule) and
            not rbc.module->getInterface(targetModule)->isConnected()) {
            // Received coordinateBack and no module is connected
            // Must wait until the module arrives to the starting position
            rbc.awaitingCoordinator = true;
            getScheduler()->schedule(
                new InterruptionEvent(getScheduler()->now() + rbc.getRoundDuration(), rbc.module,
                                      IT_MODE_TRANSFERBACK));
            return;
        }
        VS_ASSERT(rbc.operation);
        
        rbc.sendHandleableMessage(
            new CoordinateMessage(rbc.operation, targetModule, rbc.module->position, rbc.mvt_it),
            rbc.interfaceTo(targetModule), 100, 200);
    } else {  // Forward the message to the coordinator
        P2PNetworkInterface *sender = this->destinationInterface;
        rbc.sendHandleableMessage(
            static_cast<CoordinateBackMessage *>(this->clone()),
            rbc.interfaceTo(coordinatorPosition, sender), 100,
            200);
    }
}

void PLSMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *static_cast<RePoStBlockCode *>(bc);
    P2PNetworkInterface *sender = this->destinationInterface;
  
    if(rbc.module->position != destPos) {
        rbc.sendHandleableMessage(static_cast<PLSMessage*>(this->clone()),
            rbc.interfaceTo(destPos, sender), 100, 200);
        return;
    }

    bool nextToSender = rbc.isAdjacentToPosition(srcPos);
    rbc.console << "ACTUATING: " << (rbc.module->getState() == BuildingBlock::State::ACTUATING) << "\n";
    // if (rbc.module->getState() == BuildingBlock::State::ACTUATING) {
    //     getScheduler()->trace("light turned orange1", rbc.module->blockId, ORANGE);
    //     if(not rbc.awaitingSources.empty()) rbc.setGreenLight(true);
    //     rbc.awaitingSources.insert(srcPos);
      
    //     return;
    // }

    if (rbc.isCoordinator and rbc.operation->isTransfer()) {
        if (rbc.operation->getDirection() == Direction::BACK and
            srcPos.pt[2] > rbc.module->position.pt[2] and
            rbc.module->getInterface(rbc.module->position.offsetY(-1))->isConnected()) {
            //Avoid blocking when transferring back and coming from above.
            getScheduler()->trace("light turned orange2\n", rbc.module->blockId, ORANGE);
            if (not rbc.awaitingSources.empty()) rbc.setGreenLight(true);
            rbc.awaitingSources.insert(srcPos);
            rbc.module->setColor(DARKORANGE);
            return;
        }
    }

    if (rbc.operation) {
        if (/*rbc.operation->isTransfer() and*/ rbc.operation->getDirection() == Direction::BACK and
            rbc.getPreviousOpDir() == Direction::RIGHT and
            rbc.operation->getMMShape() == FRONTBACK) {
            if (rbc.relativePos() == Cell3DPosition(1, 1, 2) and
                (srcPos == rbc.seedPosition + Cell3DPosition(-2, 0, 2) or srcPos == rbc.seedPosition + Cell3DPosition(-2,1,2))) {
                rbc.console << "prevTransferCount: " << rbc.prevTransferCount << "\n";
                if (rbc.transferCount < 4 or rbc.prevTransferCount == rbc.transferCount) {
                    getScheduler()->trace("light turned orange2\n", rbc.module->blockId, ORANGE);
                    if (not rbc.awaitingSources.empty()) rbc.setGreenLight(true);
                    rbc.awaitingSources.insert(srcPos);
                    rbc.module->setColor(DARKORANGE);
                    rbc.prevTransferCount = rbc.transferCount + 3;
                    return;
                }
            }
            if (rbc.isCoordinator and (srcPos == rbc.seedPosition + Cell3DPosition(-1, 0, 2) and
                                       rbc.transferCount == 8) or
                (rbc.mvt_it == 0 and srcPos == rbc.seedPosition + Cell3DPosition(-2, 0, 2))) {
                getScheduler()->trace("light turned orange2\n", rbc.module->blockId, ORANGE);
                if (not rbc.awaitingSources.empty()) rbc.setGreenLight(true);
                rbc.awaitingSources.insert(srcPos);
                rbc.module->setColor(DARKORANGE);
                return;
            }
        }
        if (rbc.isCoordinator and rbc.operation->isFill() and Direction::LEFT and
            rbc.operation->isZeven() and rbc.transferCount > 8 and
            rbc.operation->getPrevOpDirection() == Direction::LEFT and
            (rbc.module->getInterface(rbc.module->position.offsetY(1))->isConnected() or
             rbc.module->getInterface(rbc.module->position.offsetY(-1))->isConnected())) {
            getScheduler()->trace("light turned orange2\n", rbc.module->blockId, ORANGE);
            if (not rbc.awaitingSources.empty()) rbc.setGreenLight(true);
            rbc.awaitingSources.insert(srcPos);
            rbc.module->setColor(DARKORANGE);
            return;
        }

        if (rbc.isCoordinator and rbc.operation->getPrevOpDirection() == Direction::BACK and
            rbc.module->getInterface(rbc.module->position.offsetY(-1))->isConnected()) {
            getScheduler()->trace("light turned orange2\n", rbc.module->blockId, ORANGE);
            if (not rbc.awaitingSources.empty()) rbc.setGreenLight(true);
            rbc.awaitingSources.insert(srcPos);
            rbc.module->setColor(DARKORANGE);
            return;
        }
    }

        // if (rbc.relativePos() == Cell3DPosition(1, 1, 2) and
        //     srcPos == rbc.seedPosition + Cell3DPosition(-2, 0, 2)) {
        //     if (rbc.operation->isTransfer() and rbc.operation->getDirection() == Direction::BACK and
        //         rbc.getPreviousOpDir() == Direction::RIGHT and
        //         rbc.operation->getMMShape() == FRONTBACK) {
        //         if (rbc.transferCount < 4) {
        //             getScheduler()->trace("light turned orange2\n", rbc.module->blockId, ORANGE);
        //             if (not rbc.awaitingSources.empty()) rbc.setGreenLight(true);
        //             rbc.awaitingSources.insert(srcPos);
        //             rbc.module->setColor(DARKORANGE);
        //             return;
        //         }
        //     }
        // }

        if ((rbc.greenLightIsOn or nextToSender) and
            rbc.module->getState() != BuildingBlock::State::ACTUATING) {
            // send GLO to source
            rbc.sendHandleableMessage(new GLOMessage(srcPos), rbc.interfaceTo(srcPos), 100, 200);
        } else if (srcPos - rbc.seedPosition == Cell3DPosition(0, 0, 1) and
                   rbc.lattice->cellHasBlock(rbc.module->position + Cell3DPosition(1, 0, 1))) {
            // avoid deadlock when coming from back and transfering left.
            // VS_ASSERT(false);
            rbc.sendHandleableMessage(new GLOMessage(srcPos), rbc.interfaceTo(srcPos), 100, 200);
        } else if (rbc.relativePos() == Cell3DPosition(2, 1, 2) and
                   rbc.lattice->cellHasBlock(rbc.module->position + Cell3DPosition(0, -1, 1))) {
            // avoid deadlock when FB transfering back and left MM is being dismantled
            rbc.sendHandleableMessage(new GLOMessage(srcPos), rbc.interfaceTo(srcPos), 100, 200);
        } else {
            getScheduler()->trace("light turned orange2\n", rbc.module->blockId, ORANGE);
            if (not rbc.awaitingSources.empty()) rbc.setGreenLight(true);
            rbc.awaitingSources.insert(srcPos);
            rbc.module->setColor(DARKORANGE);
        }
    }

void GLOMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *static_cast<RePoStBlockCode *>(bc);
        if(rbc.isAdjacentToPosition(srcPos) and not rbc.module->getInterface(srcPos)->isConnected()) {
        return;
    } 
    if(rbc.module->position != srcPos) {
        rbc.sendHandleableMessage(static_cast<PLSMessage*>(this->clone()),
            rbc.interfaceTo(srcPos), 100, 200);
        return;
    }


    rbc.nbWaitedAnswers--;
    if(rbc.nbWaitedAnswers == 0) {
        Cell3DPosition targetPosition =
            (*rbc.operation->localRules)[rbc.mvt_it].nextPosition + rbc.seedPosition;

        if (rbc.module->canRotateToPosition(
                targetPosition) /**and not lattice->cellIsBlocked(targetPosition)**/) {
            if ((rbc.relativePos() == Cell3DPosition(-1, 1, 2) /**or
                              relativePos() == Cell3DPosition(-1, -1, 2)**/) or
                rbc.relativePos() == Cell3DPosition(1, 0, 1) or
                rbc.relativePos() == Cell3DPosition(1, -1, 1) or
                rbc.relativePos() == Cell3DPosition(2, 0, 2) or
                rbc.relativePos() == Cell3DPosition(1,2,2) or
                rbc.relativePos() == Cell3DPosition(1,0,2)) {
                // if(relativePos() == Cell3DPosition(1,2,2)) VS_ASSERT(false);
                getScheduler()->schedule(new Catoms3DRotationStartEvent(
                    getScheduler()->now(), rbc.module, targetPosition, RotationLinkType::OctaFace,
                    false));
            } else {
                getScheduler()->schedule(
                    new Catoms3DRotationStartEvent(getScheduler()->now(), rbc.module,
                                                   targetPosition, RotationLinkType::Any, false));
            }

        } else {
            // retry rotating to next position
            //            VS_ASSERT_MSG(false, "Can not rotate to next position");
            rbc.probeGreenLight();
        }
    }
}

void FTRMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *static_cast<RePoStBlockCode *>(bc);
    // Cell3DPosition finalPos = *msg->getData();
    bool haveMovingNeighbors = false;
    // if (rbc.isCoordinator) {
        
    //     for (auto &cell : rbc.lattice->getActiveNeighborCells(rbc.module->position)) {
    //         if (not static_cast<RePoStBlockCode *>(rbc.lattice->getBlock(cell)->blockCode)->isInMM(cell) 
    //             and cell != sourceInterface->hostBlock->position
    //             and static_cast<RePoStBlockCode *>(rbc.lattice->getBlock(cell)->blockCode)->rotating) {
    //             haveMovingNeighbors = true;
    //         }
    //     }
    // }

    if (rbc.movingState == WAITING and (rbc.relativePos() == Cell3DPosition(-2, 0, 1) or
                                         rbc.relativePos() == Cell3DPosition(-2, -1, 1))) {
        /**
        * Transfer left
        * Avoid blocking when passing the last bridge module. Must stay RED until the latest arrived
        *   module starts its next operation 
        * //TODO: might need to add the last bridge module when direction is right
        */
        return;
    }
    Cell3DPosition senderPos;
    rbc.module->getNeighborPos(rbc.module->getInterfaceId(destinationInterface), senderPos);
    if (rbc.isCoordinator and rbc.getPreviousOpDir() == Direction::RIGHT and rbc.operation->getMMShape() == FRONTBACK
        and senderPos == rbc.module->position.offsetY(-1)) {
        return;
    }
    if (rbc.isCoordinator and rbc.getPreviousOpDir() == Direction::DOWN and
        rbc.operation->getDirection() == Direction::RIGHT  and
        (senderPos == rbc.module->position.offsetY(1) or senderPos == rbc.module->position.offsetY(-1 ))) {
        return;
    }
    if(rbc.isCoordinator and rbc.getPreviousOpDir() == Direction::BACK and rbc.operation->getDirection() == Direction::DOWN){
        return;
    }

    if (not rbc.greenLightIsOn and not haveMovingNeighbors) {
        rbc.setGreenLight(true);

    }
   
}
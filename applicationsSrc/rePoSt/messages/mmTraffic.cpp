//
// Created by jbassil on 17/10/22.
//

#include "mmTraffic.hpp"
#include "rePoStBlockCode.hpp"
#include "transportMessages.hpp"

void MMPLSMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);

    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        P2PNetworkInterface *itf = rbc.interfaceTo(fromMMPosition, toMMPosition);
        if (not itf->isConnected()) {
            Cell3DPosition toPos;
            rbc.module->getNeighborPos(rbc.module->getInterfaceBId(itf), toPos);
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
    if(rbc.MMgreenLightOn) {
        if(rbc.lattice->cellHasBlock(rbc.coordinatorPosition)) {
            auto *coordinator = dynamic_cast<RePoStBlockCode*>(rbc.lattice->getBlock(rbc.coordinatorPosition)->blockCode);
        }

        rbc.pathIn = make_pair(rbc.MMPosition, fromMMPosition);

        if(rbc.isPotentialDestination() ) {
        /*    rbc.destinationOut = rbc.pathOut.second[0];*/
            MaxFlow::graph[rbc.MMPosition][rbc.destinationOut]--;
            if( MaxFlow::graph[rbc.MMPosition][rbc.destinationOut] == 0) {
                MaxFlow::graph[rbc.MMPosition].erase( rbc.destinationOut);
            }
            rbc.setOperation(rbc.pathIn.second, rbc.destinationOut);
            //rbc.isDestination = false;
        } else {
            rbc.isDestination = false;
            rbc.pathOut.second.clear();
            rbc.pathOut.first = rbc.MMPosition;
            rbc.pathOut.second.push_back(MaxFlow::graph[rbc.MMPosition].begin()->first);
            MaxFlow::graph[rbc.MMPosition].begin()->second--;
            if(MaxFlow::graph[rbc.MMPosition].begin()->second == 0) {
                MaxFlow::graph[rbc.MMPosition].erase(MaxFlow::graph[rbc.MMPosition].begin());
            }
            rbc.setOperation(rbc.pathIn.second, rbc.pathOut.second[0]);
        }

        rbc.MMgreenLightOn = false;
        rbc.setMMColor(RED);
        rbc.sendHandleableMessage(new MMGLOMessage(rbc.MMPosition, fromMMPosition),
                                  rbc.interfaceTo(rbc.MMPosition, fromMMPosition), 100, 200);
    } else {
        rbc.setMMColor(ORANGE);
        rbc.awaitingMM.push(fromMMPosition);
    }
}

void MMGLOMessage::handle(BaseSimulator::BlockCode *bc) {
    RePoStBlockCode &rbc = *dynamic_cast<RePoStBlockCode *>(bc);

    Cell3DPosition toSeedPosition = rbc.getSeedPositionFromMMPosition(toMMPosition);
    if (rbc.module->position != toSeedPosition) {
        rbc.sendHandleableMessage(dynamic_cast<HandleableMessage *>(this->clone()),
                                  rbc.interfaceTo(fromMMPosition, toMMPosition), 100, 200);
        return;
    }
    auto* coordinator =
            dynamic_cast<RePoStBlockCode*>(rbc.lattice->getBlock(rbc.coordinatorPosition)->blockCode);
    rbc.console << "coordinator: " << rbc.coordinatorPosition << "\n";
    rbc.waitingMMGLO = false;
    if(coordinator->operation->isDismantle()) {
        Cell3DPosition targetModule =
                rbc.seedPosition +
                (*coordinator->operation->localRules)[0].currentPosition;

        VS_ASSERT(coordinator->operation);

        coordinator->sendHandleableMessage(
                new CoordinateMessage(coordinator->operation, targetModule,
                                      coordinator->module->position, coordinator->mvt_it),
                coordinator->module->getInterface(coordinator->nearestPositionTo(targetModule)),
                100, 200);
    } else {
        coordinator->console << "handle add neighbor " << coordinator->addedPos << "\n";
        coordinator->operation->handleAddNeighborEvent(coordinator, coordinator->addedPos);
    }


}

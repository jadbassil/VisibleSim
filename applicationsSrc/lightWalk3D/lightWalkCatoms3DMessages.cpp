#include <iostream>
#include <sstream>

#include "events/events.h"
#include "utils/utils.h"
#include "robots/catoms3D/catoms3DRotationEvents.h"

#include "lightWalkCatoms3DBlockCode.hpp"
#include "lightWalkCatoms3DMessages.hpp"


void ProbePivotLightStateMessage::handle(BaseSimulator::BlockCode* bc) {
    LightWalkCatoms3DBlockCode& mabc = *static_cast<LightWalkCatoms3DBlockCode*>(bc);
    const long long probeKey = mabc.makePositionMessageKey(srcPos, targetPos, mobileId);
    if (!mabc.seenProbeMessages.insert(probeKey).second) {
        return;
    }

    if (mabc.catom->position[2] == mabc.ZLINE) {
        if (mabc.catom->position[1] != line) {
            return;
        }

        bool nextToSender = mabc.isAdjacentToPosition(srcPos);
        bool nextToTarget = mabc.isAdjacentToPosition(targetPos);
        Catoms3DBlock* targetLightNeighbor = mabc.findTargetLightAmongNeighbors(targetPos);

        if (targetLightNeighbor
            and targetLightNeighbor->position != srcPos) {
            mabc.sendMessage(this->clone(),
                             mabc.catom->getInterface(targetLightNeighbor->position),
                             MSG_DELAY_MC, 0);
        } else if (not targetLightNeighbor and nextToTarget) {
            if (mabc.greenLightIsOn or (mabc.isLineTip() and nextToSender)) {
                P2PNetworkInterface* itf = nextToSender ?
                    mabc.catom->getInterface(srcPos) :
                    mabc.catom->getInterface(mabc.catom->position + Cell3DPosition(-1, 0, 0));
                VS_ASSERT(itf);
                mabc.greenLightIsOn = false;
                mabc.catom->setColor(RED);
                mabc.sendMessage(new GreenLightIsOnMessage(mabc.catom->position, srcPos),
                                 itf, MSG_DELAY_MC, 0);
            } else {

                mabc.moduleAwaitingGo = true;
                mabc.awaitingModulePos = srcPos;
                mabc.catom->setColor(ORANGE);
            }
        } else {
            return;
        }
    } else {
        return;
    }
}

void GreenLightIsOnMessage::handle(BaseSimulator::BlockCode* bc) {
    LightWalkCatoms3DBlockCode& mabc = *static_cast<LightWalkCatoms3DBlockCode*>(bc);

    if (mabc.catom->position != dstPos) {
        bool nextToDest = mabc.isAdjacentToPosition(dstPos);

        P2PNetworkInterface* itf = nextToDest ?
            mabc.catom->getInterface(dstPos) :
            mabc.catom->getInterface(mabc.catom->position + Cell3DPosition(-1, 0, 0));

        VS_ASSERT(itf);
        mabc.sendMessage(this->clone(), itf, MSG_DELAY_MC, 0);
    } else {
        Catoms3DBlock *pivot = mabc.findMotionPivotForTarget(mabc.targetPos);
        if (!pivot) {
            const Cell3DPosition noPivot(-1, -1, -1);
            mabc.sendReleaseToPivot(srcPos, mabc.activeReservationToken, false);
            if (mabc.pendingMotionPivotPos != noPivot
                and mabc.pendingMotionPivotPos != srcPos) {
                mabc.sendReleaseToPivot(mabc.pendingMotionPivotPos,
                                        mabc.activeReservationToken, false);
            }
            mabc.pendingMotionPivotPos = noPivot;

            const int retryEvent = mabc.catom->position[2] == mabc.ZLINE ? 2 : 1;
            mabc.scheduler->schedule(new InterruptionEvent<int>(
                mabc.scheduler->now() + MSG_DELAY_MC, mabc.catom, retryEvent));
            return;
        }

        mabc.reservedPivotPos = srcPos;
        mabc.reservedMotionPivotPos = pivot->position;
        mabc.pendingMotionPivotPos = Cell3DPosition(-1, -1, -1);
        LightWalkCatoms3DBlockCode::recordMotionStarted(mabc.catom->position,
                                                        mabc.targetPos);
        mabc.rotating = true;
        mabc.scheduler->schedule(
            new Catoms3DRotationStartEvent(getScheduler()->now() + MSG_DELAY_MC,
                                           mabc.catom, pivot, mabc.targetPos));
    }
}

void ReservationRequestMessage::handle(BaseSimulator::BlockCode* bc) {
    LightWalkCatoms3DBlockCode& mabc = *static_cast<LightWalkCatoms3DBlockCode*>(bc);
    const long long requestKey = mabc.makeTokenMessageKey(mobileId, token, 10);
    if (!mabc.seenReservationRequestMessages.insert(requestKey).second) {
        return;
    }

    if (mabc.catom->position != pivotPos) {
        const int senderId = sourceInterface ?
            static_cast<int>(sourceInterface->hostBlock->blockId) : -1;
        for (int i = 0; i < mabc.catom->getNbInterfaces(); ++i) {
            P2PNetworkInterface *p2p = mabc.catom->getInterface(i);
            if (!p2p or !p2p->isConnected()) continue;
            if (senderId != -1 and
                static_cast<int>(p2p->getConnectedBlockId()) == senderId) {
                continue;
            }

            mabc.sendMessage(this->clone(), p2p, MSG_DELAY_MC, 0);
        }
        return;
    }

    bool granted = false;
    const long long reservationKey = mabc.makeTokenMessageKey(mobileId, token, 20);
    auto reservedTarget = mabc.reservedTargetCells.find(targetPos);
    const bool targetCellAvailable =
        reservedTarget == mabc.reservedTargetCells.end()
        or reservedTarget->second == reservationKey;
    const bool sameReservation =
        mabc.reservedForMobileId == mobileId and mabc.reservedForToken == token;
    const bool olderSpacingWaiter =
        !sameReservation
        and mabc.hasOlderConflictingSpacingWaiter(mobileId, mobilePos, targetPos);
    const bool spacingAvailable =
        sameReservation
        or (!olderSpacingWaiter
            and mabc.movementSpacingIsAvailable(mobileId, token, mobilePos, targetPos));
    const bool spacingDenied = targetCellAvailable and !spacingAvailable;
    if (targetCellAvailable
        and spacingAvailable
        and (sameReservation or (mabc.reservedForToken == 0 and mabc.greenLightIsOn))) {
        granted = true;
        mabc.reservedForMobileId = mobileId;
        mabc.reservedForToken = token;
        mabc.reservedTargetPos = targetPos;
        mabc.reservedTargetCells[targetPos] = reservationKey;
        mabc.reserveMovementSpacing(mobileId, token, mobilePos, targetPos);
        mabc.greenLightIsOn = false;
        mabc.catom->setColor(RED);
    } else if (spacingDenied) {
        mabc.noteMovementSpacingDenied(mobileId, token, mobilePos, targetPos);
    } else {
        mabc.clearMovementSpacingWait(mobileId);
    }

    if (!granted) {
        LightWalkCatoms3DBlockCode::recordReservationDenied(targetCellAvailable,
                                                            spacingDenied);
    }

    P2PNetworkInterface *mobileItf = mabc.catom->getInterface(mobilePos);
    if (mobileItf and mobileItf->isConnected()) {
        mabc.sendMessage(new ReservationReplyMessage(mobileId, token, line,
                                                     pivotPos, targetPos, granted,
                                                     spacingDenied),
                         mobileItf, MSG_DELAY_MC, 0);
    } else {
        for (int i = 0; i < mabc.catom->getNbInterfaces(); ++i) {
            P2PNetworkInterface *p2p = mabc.catom->getInterface(i);
            if (!p2p or !p2p->isConnected()) continue;
            mabc.sendMessage(new ReservationReplyMessage(mobileId, token, line,
                                                         pivotPos, targetPos, granted,
                                                         spacingDenied),
                             p2p, MSG_DELAY_MC, 0);
        }
    }
}

void ReservationReplyMessage::handle(BaseSimulator::BlockCode* bc) {
    LightWalkCatoms3DBlockCode& mabc = *static_cast<LightWalkCatoms3DBlockCode*>(bc);
    const long long replyKey = mabc.makeTokenMessageKey(mobileId, token, 11);
    if (!mabc.seenReservationReplyMessages.insert(replyKey).second) {
        return;
    }

    if (static_cast<int>(mabc.catom->blockId) != mobileId) {
        const int senderId = sourceInterface ?
            static_cast<int>(sourceInterface->hostBlock->blockId) : -1;
        for (int i = 0; i < mabc.catom->getNbInterfaces(); ++i) {
            P2PNetworkInterface *p2p = mabc.catom->getInterface(i);
            if (!p2p or !p2p->isConnected()) continue;
            if (senderId != -1 and
                static_cast<int>(p2p->getConnectedBlockId()) == senderId) {
                continue;
            }

            mabc.sendMessage(this->clone(), p2p, MSG_DELAY_MC, 0);
        }
        return;
    }

    mabc.handleReservationReply(mobileId, token, pivotPos, targetPos, granted,
                                spacingDenied);
}

void LaunchLineMessage::handle(BaseSimulator::BlockCode* bc) {
    LightWalkCatoms3DBlockCode& mabc = *static_cast<LightWalkCatoms3DBlockCode*>(bc);
    const long long key = mabc.makeLineLaunchKey(targetLine, launchId);
    if (!mabc.seenLineLaunchMessages.insert(key).second) {
        return;
    }

    if (mabc.lineNumber == targetLine) {
        mabc.lineStarted = true;
        if (mabc.isLineHead()) {
            mabc.startLineLocomotion();
        }
    }

    const int senderId = sourceInterface ?
        static_cast<int>(sourceInterface->hostBlock->blockId) : -1;
    mabc.sendLineLaunch(targetLine, launchId, senderId);
}

void ReleasePivotLightMessage::handle(BaseSimulator::BlockCode* bc) {
    LightWalkCatoms3DBlockCode& mabc = *static_cast<LightWalkCatoms3DBlockCode*>(bc);
    const long long releaseKey = mabc.makeTokenMessageKey(mobileId, token, 12);
    if (!mabc.seenReleaseMessages.insert(releaseKey).second) {
        return;
    }

    if (mabc.catom->position != pivotPos) {
        const int senderId = sourceInterface ?
            static_cast<int>(sourceInterface->hostBlock->blockId) : -1;
        for (int i = 0; i < mabc.catom->getNbInterfaces(); ++i) {
            P2PNetworkInterface *p2p = mabc.catom->getInterface(i);
            if (!p2p or !p2p->isConnected()) continue;
            if (senderId != -1 and
                static_cast<int>(p2p->getConnectedBlockId()) == senderId) {
                continue;
            }

            mabc.sendMessage(this->clone(), p2p, MSG_DELAY_MC, 0);
        }
        return;
    }

    mabc.releaseMovementSpacing(mobileId, token);

    if (mabc.reservedForMobileId == mobileId and mabc.reservedForToken == token) {
        const long long reservationKey = mabc.makeTokenMessageKey(mobileId, token, 20);
        auto reservedTarget = mabc.reservedTargetCells.find(mabc.reservedTargetPos);
        if (reservedTarget != mabc.reservedTargetCells.end()
            and reservedTarget->second == reservationKey) {
            mabc.reservedTargetCells.erase(reservedTarget);
        }
        mabc.reservedTargetPos = Cell3DPosition(-1, -1, -1);
        mabc.reservedForMobileId = -1;
        mabc.reservedForToken = 0;
        mabc.greenLightIsOn = true;
        mabc.moduleAwaitingGo = false;
        mabc.catom->setColor(GREEN);
    } else if (mabc.reservedForToken == 0) {
        mabc.greenLightIsOn = true;
        mabc.moduleAwaitingGo = false;
        mabc.catom->setColor(GREEN);
    }

    P2PNetworkInterface *mobileItf = mabc.catom->getInterface(mobilePos);
    if (mobileItf and mobileItf->isConnected()) {
        mabc.sendMessage(new ReleasePivotAckMessage(mobileId, token, line,
                                                    completedMotion),
                         mobileItf, MSG_DELAY_MC, 0);
    } else {
        for (int i = 0; i < mabc.catom->getNbInterfaces(); ++i) {
            P2PNetworkInterface *p2p = mabc.catom->getInterface(i);
            if (!p2p or !p2p->isConnected()) continue;
            mabc.sendMessage(new ReleasePivotAckMessage(mobileId, token, line,
                                                        completedMotion),
                             p2p, MSG_DELAY_MC, 0);
        }
    }
}

void ReleasePivotAckMessage::handle(BaseSimulator::BlockCode* bc) {
    LightWalkCatoms3DBlockCode& mabc = *static_cast<LightWalkCatoms3DBlockCode*>(bc);
    const long long ackKey = mabc.makeTokenMessageKey(mobileId, token, 13);
    if (!mabc.seenReservationAckMessages.insert(ackKey).second) {
        return;
    }

    if (static_cast<int>(mabc.catom->blockId) != mobileId) {
        const int senderId = sourceInterface ?
            static_cast<int>(sourceInterface->hostBlock->blockId) : -1;
        for (int i = 0; i < mabc.catom->getNbInterfaces(); ++i) {
            P2PNetworkInterface *p2p = mabc.catom->getInterface(i);
            if (!p2p or !p2p->isConnected()) continue;
            if (senderId != -1 and
                static_cast<int>(p2p->getConnectedBlockId()) == senderId) {
                continue;
            }

            mabc.sendMessage(this->clone(), p2p, MSG_DELAY_MC, 0);
        }
        return;
    }

    mabc.handleReleaseAck(mobileId, token, completedMotion);
}

void DpValueMessage::handle(BaseSimulator::BlockCode* bc) {
    LightWalkCatoms3DBlockCode& mabc = *static_cast<LightWalkCatoms3DBlockCode*>(bc);
    mabc.handleDpValueMessage(originId, sequence, cell, dpValue);
}

void FlattenLayerLaunchMessage::handle(BaseSimulator::BlockCode* bc) {
    LightWalkCatoms3DBlockCode& mabc = *static_cast<LightWalkCatoms3DBlockCode*>(bc);

    const long long key = mabc.makeFlattenKey(targetLayer, launchId);
    if (!mabc.seenFlattenLaunchMessages.insert(key).second)
        return;

    const int senderId = sourceInterface ?
        static_cast<int>(sourceInterface->hostBlock->blockId) : -1;
    for (int i = 0; i < mabc.catom->getNbInterfaces(); ++i) {
        P2PNetworkInterface *p2p = mabc.catom->getInterface(i);
        if (!p2p or !p2p->isConnected()) continue;
        if (senderId != -1 and
            static_cast<int>(p2p->getConnectedBlockId()) == senderId) continue;
        mabc.sendMessage(this->clone(), p2p, MSG_DELAY_MC, 0);
    }

    if (static_cast<int>(mabc.catom->position[2]) != targetLayer)
        return;
    if (mabc.flatteningLocomotionStarted)
        return;

    if (mabc.isFlatteningLeftHead()) {
        mabc.startFlatteningLocomotion();
    }
}

void FlattenLayerDoneMessage::handle(BaseSimulator::BlockCode* bc) {
    LightWalkCatoms3DBlockCode& mabc = *static_cast<LightWalkCatoms3DBlockCode*>(bc);

    const long long key = mabc.makeFlattenKey(completedLayer, launchId);
    if (!mabc.seenFlattenDoneMessages.insert(key).second)
        return;

    const int senderId = sourceInterface ?
        static_cast<int>(sourceInterface->hostBlock->blockId) : -1;
    for (int i = 0; i < mabc.catom->getNbInterfaces(); ++i) {
        P2PNetworkInterface *p2p = mabc.catom->getInterface(i);
        if (!p2p or !p2p->isConnected()) continue;
        if (senderId != -1 and
            static_cast<int>(p2p->getConnectedBlockId()) == senderId) continue;
        mabc.sendMessage(this->clone(), p2p, MSG_DELAY_MC, 0);
    }

    // Once the flattening phase has finished, never re-run the completion
    // block: doing so re-resets every module's line-launch guards and
    // restarts line locomotion, causing LaunchLine messages to be re-emitted
    // forever.
    if (LightWalkCatoms3DBlockCode::flatteningPhaseComplete)
        return;

    // Count each layer exactly once across the whole network. The done flood
    // reaches every module and each one runs this handler once (per-module
    // dedup), so a raw increment counted module receptions rather than layers
    // and blew past the threshold on the first layer.
    if (!LightWalkCatoms3DBlockCode::flatteningDoneLayers.insert(completedLayer).second)
        return;
    LightWalkCatoms3DBlockCode::flatteningCompletedLayers =
        static_cast<int>(LightWalkCatoms3DBlockCode::flatteningDoneLayers.size());

    if (LightWalkCatoms3DBlockCode::flatteningCompletedLayers >=
        LightWalkCatoms3DBlockCode::flatteningTotalLayers)
    {

        LightWalkCatoms3DBlockCode::flatteningPhaseActive   = false;
        LightWalkCatoms3DBlockCode::flatteningPhaseComplete = true;

        auto &blocks = BaseSimulator::getWorld()->getMap();

        for (const auto &entry : blocks) {
            auto *bb = static_cast<Catoms3D::Catoms3DBlock *>(entry.second);
            if (!bb) continue;
            auto *bc2 = dynamic_cast<LightWalkCatoms3DBlockCode *>(bb->blockCode);
            if (!bc2 or !bc2->initialized) continue;

            bc2->lineNumber              = static_cast<int>(bb->position[1]);
            bc2->relocated               = false;
            bc2->lineStarted             = false;
            bc2->initialLineHead         = bc2->isLineHead();
            bc2->nextLineLaunchSent      = true;
            bc2->sameLineSuccessorLaunchSent = false;
            bc2->releasedPivotCount      = 0;
        }

        for (const auto &entry : blocks) {
            auto *bb = static_cast<Catoms3D::Catoms3DBlock *>(entry.second);
            if (!bb) continue;
            auto *bc2 = dynamic_cast<LightWalkCatoms3DBlockCode *>(bb->blockCode);
            if (!bc2 or !bc2->initialized) continue;

            if (bc2->isLineHead()) {

                bc2->initialLineHead = true;
                bc2->lineStarted     = true;
                bc2->startLineLocomotion();
            } else {
                bb->setColor(GREEN);
            }
        }
    } else {

        const int nextLayer = completedLayer + 1;
        mabc.broadcastFlattenLaunch(nextLayer,
                                    static_cast<int>(mabc.catom->blockId));
    }
}

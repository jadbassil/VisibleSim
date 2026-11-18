/*
 * blinkyBlocksBlockCode.h
 *
 *  Created on: 23 mars 2013
 *      Author: dom
 */

#ifndef BLINKYBLOCKSBLOCKCODE_H_
#define BLINKYBLOCKSBLOCKCODE_H_

#include "../../base/blockCode.h"
#include "../../comm/network.h"
#include "../../events/events.h"
#include "blinkyBlocksBlock.h"

namespace BlinkyBlocks {

class BlinkyBlocksBlock;

class BlinkyBlocksBlockCode : public BaseSimulator::BlockCode {
   public:
    BlinkyBlocksBlockCode(BlinkyBlocksBlock* host);

    virtual ~BlinkyBlocksBlockCode() override;
    static Time getMessageTime(int size);

    int sendMessage(const char* msgString, Message* msg, P2PNetworkInterface* dest, int nbBytes);

    int sendMessage(Message* msg, P2PNetworkInterface* dest, int nbBytes);
    int sendMessage(const char* msgString, Message* msg,
                                       P2PNetworkInterface* dest, int t, int dt);
    int sendMessage(HandleableMessage* msg, P2PNetworkInterface* dest, int nbBytes);
    virtual void processLocalEvent(EventPtr pev) override {
        // Do not remove, used by sample blockcode
        BlockCode::processLocalEvent(pev);
    }
};

}  // namespace BlinkyBlocks

#endif /* BLINKYBLOCKSBLOCKCODE_H_ */

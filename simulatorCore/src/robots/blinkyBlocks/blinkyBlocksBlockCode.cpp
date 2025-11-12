/*
 * blinkyBlocksBlockCode.cpp
 *
 *  Created on: 23 mars 2013
 *      Author: dom
 */

#include "blinkyBlocksBlockCode.h"

#include <iostream>

#include "../../utils/trace.h"

using namespace std;

namespace BlinkyBlocks {

BlinkyBlocksBlockCode::BlinkyBlocksBlockCode(BlinkyBlocksBlock* host) : BlockCode(host) {
#ifdef DEBUG_OBJECT_LIFECYCLE
    OUTPUT << "BlinkyBlocksBlockCode constructor" << endl;
#endif
}

Time BlinkyBlocksBlockCode::getMessageTime(int size) {
    //    double time = 9.443 * exp(-12) * pow(size, 6) - 6.733 * exp(-9) * pow(size, 5) + 1.808 *
    //    exp(-6) * pow(size, 4) -
    //                 0.0002245 * pow(size, 3) + 0.01261 * pow(size, 2) - 0.262 * size + 1.857;
    double time = 0.08935 * (double)size + 1.516;
    return (Time)round(time * 1000);
}

int BlinkyBlocksBlockCode::sendMessage(Message* msg, P2PNetworkInterface* dest, int nbBytes) {
    return BlockCode::sendMessage(msg, dest, BlinkyBlocksBlockCode::getMessageTime(nbBytes), 0);
}

int BlinkyBlocksBlockCode::sendMessage(const char* msgString, Message* msg,
                                       P2PNetworkInterface* dest, int t, int dt) {
    // cout << "send Message: " << nbBytes << " " <<  t << endl;
    return BlockCode::sendMessage(msgString, msg, dest, t, dt);
}

int BlinkyBlocksBlockCode::sendMessage(const char* msgString, Message* msg,
                                       P2PNetworkInterface* dest, int nbBytes) {
    Time t = BlinkyBlocksBlockCode::getMessageTime(nbBytes);
    // cout << "send Message: " << nbBytes << " " <<  t << endl;
    return BlockCode::sendMessage(msgString, msg, dest, t, 0);
}

int BlinkyBlocksBlockCode::sendMessage(HandleableMessage* msg, P2PNetworkInterface* dest,
                                       int nbBytes) {
    // cout << "send Message: " << nbBytes << endl;
    return BlockCode::sendMessage(msg, dest, BlinkyBlocksBlockCode::getMessageTime(nbBytes), 0);
}

BlinkyBlocksBlockCode::~BlinkyBlocksBlockCode() {
#ifdef DEBUG_OBJECT_LIFECYCLE
    OUTPUT << "BlinkyBlocksBlockCode destructor" << endl;
#endif
}

}  // namespace BlinkyBlocks

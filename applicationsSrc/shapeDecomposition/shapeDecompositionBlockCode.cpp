#include "shapeDecompositionBlockCode.hpp"

#include <bitset>
#include <climits>

#include "robots/blinkyBlocks/blinkyBlocksSimulator.h"

using namespace BlinkyBlocks;

array<SCLattice::myDirection, 4> ShapeDecompositionBlockCode::DIRECTIONS = {
    SCLattice::myDirection::Front, SCLattice::myDirection::Right, SCLattice::myDirection::Back,
    SCLattice::myDirection::Left};

array<string, 4> ShapeDecompositionBlockCode::DIRECTIONS_NAMES = {"Front", "Right", "Back", "Left"};

vector<Corner> ShapeDecompositionBlockCode::corners;

int ShapeDecompositionBlockCode::nbCorners = 0;

ShapeDecompositionBlockCode::ShapeDecompositionBlockCode(BlinkyBlocksBlock *host)
    : BlinkyBlocksBlockCode(host) {
    // @warning Do not remove block below, as a blockcode with a NULL host might be created
    //  for command line parsing
    if (not host) return;

    // Registers a callback (handleSampleMessage) to the message of type SAMPLE_MSG_ID
    addMessageEventFunc2(BORDER_MSG_ID,
                         std::bind(&ShapeDecompositionBlockCode::handleBorderMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
 
    addMessageEventFunc2(GET_BORDER_MSG_ID,
                         std::bind(&ShapeDecompositionBlockCode::handleGetBorderMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));
    addMessageEventFunc2(BRIDGE_MSG_ID,
                         std::bind(&ShapeDecompositionBlockCode::handleBridgeMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));

    // Set the module pointer
    module = static_cast<BlinkyBlocksBlock *>(hostBlock);
}

void ShapeDecompositionBlockCode::startup() {
    if (module->blockId == 1) {
        string configFile = getSimulator()->getCmdLine().getConfigFile();
        configFile = configFile.substr(0, configFile.find_last_of("."));
        string outputFileName = "results_" + configFile + ".json";
        // if outputFileName exists, delete it
        cout << "outputFileName: " << outputFileName << endl;
        remove(outputFileName.c_str());
        
    }
    console << "start\n";
    if ((isFrontmost() && isLeftmost()) ||
        (isFrontmost() && lattice->cellHasBlock(module->position + Cell3DPosition(1, -1, 0)) &&
         lattice->cellHasBlock(module->position + Cell3DPosition(1, 0, 0)))) {
        isCorner = true;
        //module->setColor(YELLOW);
    }
    if (isBridgeEnd()) {
        console << "isBridgeEnd\n";
        isBridge = true;
    }
    onBorder = false;

    if (isCorner && !onBorder) {
        //module->setColor(GREEN);
        onBorder = true;
        traceIsActive = true;
        myCorner.position = module->position;

        auto sendBorderMessage = [&](SCLattice::myDirection dir, Direction nextDir) {
            nextDirections.push_back(nextDir);
            myCorner.nextDir = nextDir;
            // if (isBridge) {
            //     P2PNetworkInterface *bridgeItf =
            //         getInterfaceInDirection(DIRECTIONS[(static_cast<short>(nextDir) + 1) % 4]);
            //     if (bridgeItf && bridgeItf->isConnected()) {
            //         bridgeEnds[1] = bridgeItf;
            //         sendMessage("Bridge", new Message(BRIDGE_MSG_ID), bridgeItf, 1);
            //     }
            // }
            console << "Sending message to " << DIRECTIONS_NAMES[int(nextDir)] << "\n";
            border.directions.push_back(nextDir);
            border.initiatorId = module->blockId;
            sendMessage("Border",
                        new MessageOf<BorderMessageData>(
                            BORDER_MSG_ID,
                            BorderMessageData(myCorner, nextDir, module->blockId, border,
                                              vector<Direction>(), vector<Corner>({myCorner}))),
                        module->getInterface(dir), 1);
        };

        if (module->hasNeighbor(SCLattice::myDirection::Front)) {
            sendBorderMessage(SCLattice::myDirection::Front, Direction::Front);
        } else if (module->hasNeighbor(SCLattice::myDirection::Right)) {
            sendBorderMessage(SCLattice::myDirection::Right, Direction::Right);
        } else if (module->hasNeighbor(SCLattice::myDirection::Back)) {
            sendBorderMessage(SCLattice::myDirection::Back, Direction::Back);
        } else if (module->hasNeighbor(SCLattice::myDirection::Left)) {
            sendBorderMessage(SCLattice::myDirection::Left, Direction::Left);
        }
    }
}

void ShapeDecompositionBlockCode::handleBorderMessage(std::shared_ptr<Message> _msg,
                                                      P2PNetworkInterface *sender) {
    MessageOf<BorderMessageData> *msg = static_cast<MessageOf<BorderMessageData> *>(_msg.get());
    BorderMessageData data = *msg->getData();
    Direction dir = data.dir;
    Border receivedBorder = data.border;
    vector<Direction> receivedDirections = data.directions;
    vector<Corner> receivedCorners = data.corners;
    short initiatorId = data.initiatorId;

    console << "Received message from " << DIRECTIONS_NAMES[int(dir)]
            << " initiator: " << initiatorId << " sender: " << sender->getConnectedBlockId()
            << "\n";
    if (initiatorId == module->blockId) {
        if (receivedCorners.size() >= 3) {
            console << "On Bresenham line: " << receivedCorners[receivedCorners.size() - 1].position
                    << " " << receivedCorners[receivedCorners.size() - 2].position << " "
                    << receivedCorners[receivedCorners.size() - 3].position << " "
                    << (receivedCorners.size() > 3
                            ? receivedCorners[receivedCorners.size() - 4].position
                            : Cell3DPosition())
                    << " " << module->position << "\n";

            if (receivedCorners.size() > 3 &&
                isOnLine(receivedCorners[receivedCorners.size() - 4].position,
                         receivedCorners[receivedCorners.size() - 2].position, module->position) &&
                isOnBorder() and
                (module->position.dist_euclid(
                     receivedCorners[receivedCorners.size() - 1].position) <= 1 ||
                 receivedCorners[receivedCorners.size() - 1].position.dist_euclid(
                     receivedCorners[receivedCorners.size() - 2].position) <= 1)) {
                console << "On line 1\n";
                receivedCorners.erase(receivedCorners.end() - 3, receivedCorners.end());
            } else if (isOnLine(receivedCorners[receivedCorners.size() - 2].position,
                                receivedCorners[receivedCorners.size() - 3].position,
                                module->position) and
                       (module->position.dist_euclid(
                            receivedCorners[receivedCorners.size() - 1].position) <= 1 ||
                        receivedCorners[receivedCorners.size() - 1].position.dist_euclid(
                            receivedCorners[receivedCorners.size() - 2].position) <= 1)) {
                receivedCorners.erase(receivedCorners.end() - 2, receivedCorners.end());
                console << "On line 2\n";
            }
            receivedCorners.push_back(myCorner);
        }
        //module->setColor(RED);
        cout << "border " << module->blockId << " terminated\n";
        
        // print receivedDirections
        cout << "Decompressed directions" << endl;
        // Print the received directions
        for (auto &d : receivedDirections) {
            std::cout << (int)d << " ";
        }
        cout << endl << endl;
        // Compress the directions
        vector<int> compressedDirections = RLEcompress(receivedDirections);
        cout << "RLE compression\n";
        cout << module->blockId << " " << receivedDirections.size() << " "
             << compressedDirections.size() << endl;
        // Print the compressed directions
        for (auto &d : compressedDirections) {
            std::cout << d << " ";
        }
        cout << endl << endl << "LZW compression\n";
        compressedDirections = LZWcompress(receivedDirections).first;
        cout << module->blockId << " " << receivedDirections.size() << " "
             << compressedDirections.size() << endl;
        // Print the compressed directions
        for (auto &d : compressedDirections) {
            std::cout << d << " ";
        }
        cout << endl << endl;
        printResults(receivedDirections, receivedCorners);
        // Decompress the directions
        // cout << "Decompressed directions" << endl;
        // vector<Direction> decompressedDirections = LZWdecompress(compressedDirections);
        // cout << module->blockId << " " << decompressedDirections.size() << endl;
        // for(auto &d : decompressedDirections) {
        //     std::cout << DIRECTIONS_NAMES[(int)d] << " ";
        // }
        cout << endl << endl;
        isInitiator = true;
        module->setColor(BLUE);

        
            nbWaitedBridges[module->blockId] = 0;
            bridgesIn[module->blockId] = nullptr;
            // initiate bridge search
            for (auto &itf: module->getP2PNetworkInterfaces()) {
                if(itf->isConnected()) {
                    sendMessage("bridge msg", new MessageOf<BridgeMessageData>(BRIDGE_MSG_ID, BridgeMessageData(0, module->blockId, false)), itf, 1);
                    nbWaitedBridges[module->blockId]++;
                }
            }
        
        return;
    }
    if (borderInitiators.find(dir) != borderInitiators.end()) {
        if (borderInitiators[dir] < initiatorId) {
            console << "Already received message from " << DIRECTIONS_NAMES[int(dir)] << "\n";
            return;
        }
    }
    borderInitiators[dir] = initiatorId;
    prevDirections.push_back(dir);
    prevCorner = data.prevCorner;
    receivedDirections.push_back(dir);
    // module->setColor(RED);
    onBorder = true;

    short j = (short(dir) + 3) % 4;
    for (int i = 0; i < 4; i++) {
        console << "j=" << j << "\n";
        P2PNetworkInterface *nextItf = getInterfaceInDirection(DIRECTIONS[j]);
        Corner *newCorner = NULL;
        if (nextItf and nextItf->isConnected()) {
            if (j != static_cast<short>(dir)) {
                // module->setColor(GREEN);
                newCorner = new Corner(module->position, dir, static_cast<Direction>(j));
                if (find(corners.begin(), corners.end(), *newCorner) == corners.end())
                    corners.push_back(*newCorner);
                prevCorner = *newCorner;
                console << "size: " << receivedCorners.size() << "\n";
                if (receivedCorners.size() >= 3) {
                    console << "On Bresenham line: "
                            << receivedCorners[receivedCorners.size() - 1].position << " "
                            << receivedCorners[receivedCorners.size() - 2].position << " "
                            << receivedCorners[receivedCorners.size() - 3].position << " "
                            << (receivedCorners.size() > 3
                                    ? receivedCorners[receivedCorners.size() - 4].position
                                    : Cell3DPosition())
                            << " " << module->position << "\n";
                    // get altitude to the hypothenus of the triangle formed by the last 3 corners
                    if (receivedCorners.size() > 3 &&
                        isOnLine(receivedCorners[receivedCorners.size() - 4].position,
                                 receivedCorners[receivedCorners.size() - 2].position,
                                 module->position) &&
                        isOnBorder() and
                        (module->position.dist_euclid(
                             receivedCorners[receivedCorners.size() - 1].position) <= 1 ||
                         receivedCorners[receivedCorners.size() - 1].position.dist_euclid(
                             receivedCorners[receivedCorners.size() - 2].position) <= 1)) {
                        console << "On line 1\n";
                        receivedCorners.erase(receivedCorners.end() - 3, receivedCorners.end());
                    } else if (isOnLine(receivedCorners[receivedCorners.size() - 2].position,
                                        receivedCorners[receivedCorners.size() - 3].position,
                                        module->position) and
                               (module->position.dist_euclid(
                                    receivedCorners[receivedCorners.size() - 1].position) <= 1 ||
                                receivedCorners[receivedCorners.size() - 1].position.dist_euclid(
                                    receivedCorners[receivedCorners.size() - 2].position) <= 1)) {
                        receivedCorners.erase(receivedCorners.end() - 2, receivedCorners.end());
                        console << "On line 2\n";
                    }
                    receivedCorners.push_back(*newCorner);
                } else {
                    receivedCorners.push_back(*newCorner);
                }

                // receivedCorners.push_back(*newCorner);
            }
            initiator_prevNext[initiatorId] =
                make_pair(DIRECTIONS[static_cast<short>(dir)], DIRECTIONS[j]);
            prevInBorder = DIRECTIONS[static_cast<short>(dir)];

            nextDirections.push_back(static_cast<Direction>(j));
            if (nextDirections.size() > prevDirections.size()) {
                nextDirections.erase(nextDirections.begin());
            }
            receivedBorder.directions.push_back(static_cast<Direction>(j));
            if (newCorner) {
                sendMessage(
                    "Border1",
                    new MessageOf<BorderMessageData>(
                        BORDER_MSG_ID,
                        BorderMessageData(*newCorner, static_cast<Direction>(j), initiatorId,
                                          border, receivedDirections, receivedCorners)),
                    nextItf, receivedCorners.size() * 2 + 1);
            } else {
                sendMessage(
                    "Border2",
                    new MessageOf<BorderMessageData>(
                        BORDER_MSG_ID,
                        BorderMessageData(data.prevCorner, static_cast<Direction>(j), initiatorId,
                                          border, receivedDirections, receivedCorners)),
                    nextItf, receivedCorners.size() * 2 + 1);
            }
            return;
        }
        j = (j + 1) % 4;
    }
}

void ShapeDecompositionBlockCode::handleBridgeMessage(std::shared_ptr<Message> _msg,
                                                      P2PNetworkInterface *sender) {
    // Handle bridge message
    MessageOf<BridgeMessageData> *msg = static_cast<MessageOf<BridgeMessageData> *>(_msg.get());
    BridgeMessageData data = *msg->getData();
    int distance = data.distance + 1;
    bool selected = data.selected;
    int initiatorId = data.initiatorId;
    if (distance > 0) {
        if(nbWaitedBridges.find(initiatorId) == nbWaitedBridges.end()) {
            nbWaitedBridges[initiatorId] = 0;
        }
        if(bridgesDistance.find(initiatorId) == bridgesDistance.end() || distance < bridgesDistance[initiatorId]) {
            bridgesDistance[initiatorId] = distance;
            bridgesIn[initiatorId] = sender;
            
            console << "Forwarding bridge message from " << sender->getConnectedBlockId() << "\n";
            for(auto &itf: module->getP2PNetworkInterfaces()) {
                if(itf != sender && itf->isConnected()) {
                    sendMessage("Bridge msg", new MessageOf<BridgeMessageData>(BRIDGE_MSG_ID, BridgeMessageData(distance, initiatorId, false)), itf, 1);
                    nbWaitedBridges[initiatorId]++;
                }
            }
            if(nbWaitedBridges[initiatorId] == 0) {
                sendMessage("Bridge back msg", new MessageOf<BridgeMessageData>(BRIDGE_MSG_ID, BridgeMessageData(-1, initiatorId, true)), sender, 1);
            }
        } else {
            sendMessage("Bridge back msg", new MessageOf<BridgeMessageData>(BRIDGE_MSG_ID, BridgeMessageData(-1, initiatorId, false)), sender, 1);
        }
    } else if (distance <= 0) {
        nbWaitedBridges[initiatorId]--;
        console << "Received bridge back message from " << sender->getConnectedBlockId()  << "\n";
        if(selected) {
            bridgeOut[initiatorId] = sender;
        }
        if(nbWaitedBridges[initiatorId] == 0) {
            if(module->blockId == initiatorId) {
                cout << "Bridge search terminated for initiator " << initiatorId << "\n";
            } else {
                sendMessage("Bridge back msg", new MessageOf<BridgeMessageData>(BRIDGE_MSG_ID, BridgeMessageData(-1, initiatorId, true)), bridgesIn[initiatorId], 1);
            }

        }
    
    }
}

void ShapeDecompositionBlockCode::handleGetBorderMessage(std::shared_ptr<Message> _msg,
                                                         P2PNetworkInterface *sender) {}

// RLE compression on a vector of directions
vector<int> ShapeDecompositionBlockCode::RLEcompress(vector<Direction> &directions) {
    vector<int> compressed;
    int count = 1;
    for (int i = 1; i < directions.size(); i++) {
        if (directions[i] == directions[i - 1]) {
            count++;
        } else {
            compressed.push_back(count);
            compressed.push_back(static_cast<int>(directions[i - 1]));
            count = 1;
        }
    }
    compressed.push_back(count);
    compressed.push_back(static_cast<int>(directions[directions.size() - 1]));
    return compressed;
}

// LZW compression on a vector of directions
pair<vector<int>, int> ShapeDecompositionBlockCode::LZWcompress(vector<Direction> &directions) {
    vector<int> compressed;
    int totalBits = 0;
    map<vector<Direction>, int> dictionary;
    for (int i = 0; i < 4; i++) {
        vector<Direction> v;
        v.push_back((Direction)i);
        dictionary[v] = i;
    }
    vector<Direction> w;
    w.push_back(directions[0]);
    for (int i = 1; i < directions.size(); i++) {
        vector<Direction> wc = w;
        wc.push_back(directions[i]);
        if (dictionary.find(wc) != dictionary.end()) {
            w = wc;
        } else {
            compressed.push_back(dictionary[w]);
            vector<Direction> wc1;
            wc1.push_back(directions[i]);
            dictionary[wc] = dictionary.size();
            w = wc1;
        }
    }
    int bitsPerWord = 2;  // Initial words can be encoded on 2 bits each
    for (int i = 0; i < compressed.size(); i++) {
        // Adjust bitsPerWord to the minimum required for the current word
        while (compressed[i] < (1 << (bitsPerWord - 1)) && bitsPerWord > 2) {
            bitsPerWord--;
        }
        totalBits += bitsPerWord;
        // Increase bitsPerWord if the current word requires more bits
        while (compressed[i] >= (1 << bitsPerWord)) {
            bitsPerWord++;
        }
    }
    cout << "Total bits: " << totalBits << endl;
    return make_pair(compressed, totalBits);
}

pair<vector<int>, int> ShapeDecompositionBlockCode::LZWcompressCorners(vector<Corner> &corners) {
    vector<int> compressed;
    int totalBits = 0;
    vector<int> v;
    map<vector<int>, int> dictionary;
    for (auto &pos : corners) {

        v.push_back((int) pos.position.pt[0]);
        v.push_back((int) pos.position.pt[1]);
        v.push_back((int) pos.position.pt[2]);
    }
    for (int i = 0; i <= 9; i++) {
        dictionary[{i}] = i;
    }
    vector<int> w;
    w.push_back(v[0]);
    for (int i = 1; i < compressed.size(); i++) {
        vector<int> wc = w;
        wc.push_back(compressed[i]);
        if (dictionary.find(wc) != dictionary.end()) {
            w = wc;
        } else {
            compressed.push_back(dictionary[w]);
            vector<int> wc1;
            wc1.push_back(compressed[i]);
            dictionary[wc] = dictionary.size();
            w = wc1;
        }
    }
    int bitsPerWord = 4;  // Initial words can be encoded on 4 bits each
    for (int i = 0; i < compressed.size(); i++) {
        // Adjust bitsPerWord to the minimum required for the current word
        while (compressed[i] < (1 << (bitsPerWord - 1)) && bitsPerWord > 4) {
            bitsPerWord--;
        }
        totalBits += bitsPerWord;
        // Increase bitsPerWord if the current word requires more bits
        while (compressed[i] >= (1 << bitsPerWord)) {
            bitsPerWord++;
        }
    }
    cout << "Total bits: " << totalBits << endl;
    return make_pair(compressed, totalBits);
}

// LZW decompression on a vector of directions
vector<Direction> ShapeDecompositionBlockCode::LZWdecompress(vector<int> &compressed) {
    vector<Direction> directions;
    map<int, vector<Direction>> dictionary;
    for (int i = 0; i < 4; i++) {
        vector<Direction> v;
        v.push_back((Direction)i);
        dictionary[i] = v;
    }
    int old = compressed[0];
    vector<Direction> s = dictionary[old];
    vector<Direction> c;
    c.push_back(s[0]);
    for (int i = 1; i < compressed.size(); i++) {
        int n = compressed[i];
        if (dictionary.find(n) == dictionary.end()) {
            s = dictionary[old];
            s.push_back(s[0]);
        } else {
            s = dictionary[n];
        }
        for (int j = 0; j < s.size(); j++) {
            c.push_back(s[j]);
        }
        vector<Direction> entry = dictionary[old];
        entry.push_back(s[0]);
        dictionary[dictionary.size()] = entry;
        old = n;
    }
}

// LZW compress bitset
pair<vector<int>, int> ShapeDecompositionBlockCode::LZWcompressBitset(
    vector<bitset<8>> &directions) {
    vector<int> compressed;
    map<vector<bitset<8>>, int, VectorBitsetComparator> dictionary;
    for (int i = 0; i < 256; i++) {
        vector<bitset<8>> v;
        v.push_back(bitset<8>(i));
        dictionary[v] = i;
    }
    vector<bitset<8>> w;
    w.push_back(directions[0]);
    for (int i = 1; i < directions.size(); i++) {
        vector<bitset<8>> wc = w;
        wc.push_back(directions[i]);
        if (dictionary.find(wc) != dictionary.end()) {
            w = wc;
        } else {
            compressed.push_back(dictionary[w]);
            vector<bitset<8>> wc1;
            wc1.push_back(directions[i]);
            dictionary[wc] = dictionary.size();
            w = wc1;
        }
    }
    compressed.push_back(dictionary[w]);
    int totalBits = 0;
    int bitsPerWord = 8;  // Initial words can be encoded on 2 bits each
    for (int i = 0; i < compressed.size(); i++) {
        // Adjust bitsPerWord to the minimum required for the current word
        while (compressed[i] < (1 << (bitsPerWord - 1)) && bitsPerWord > 2) {
            bitsPerWord--;
        }
        totalBits += bitsPerWord;
        // Increase bitsPerWord if the current word requires more bits
        while (compressed[i] >= (1 << bitsPerWord)) {
            bitsPerWord++;
        }
    }
    cout << "Total bits: " << totalBits << endl;
    return make_pair(compressed, totalBits);
}


short ShapeDecompositionBlockCode::getSenderDirection(P2PNetworkInterface *sender) {
    Cell3DPosition senderPos =
        BaseSimulator::getWorld()->getBlockById(sender->getConnectedBlockId())->position;
    Cell3DPosition diff = senderPos - module->position;
    int dir;
    console << "diff: " << diff << "\n";
    if (diff == Cell3DPosition(0, 1, 0)) {
        dir = 2;
    } else if (diff == Cell3DPosition(1, 0, 0)) {
        dir = 1;
    } else if (diff == Cell3DPosition(0, -1, 0)) {
        dir = 0;
    } else if (diff == Cell3DPosition(-1, 0, 0)) {
        dir = 3;
    }
    return dir;
}

P2PNetworkInterface *ShapeDecompositionBlockCode::getInterfaceInDirection(
    SCLattice::myDirection dir) {
    switch (dir) {
        case SCLattice::myDirection::Back:
            return module->getInterfaceToNeighborPos(module->position + Cell3DPosition(0, 1, 0));
        case SCLattice::myDirection::Right:
            return module->getInterfaceToNeighborPos(module->position + Cell3DPosition(1, 0, 0));
        case SCLattice::myDirection::Front:
            return module->getInterfaceToNeighborPos(module->position + Cell3DPosition(0, -1, 0));
        case SCLattice::myDirection::Left:
            return module->getInterfaceToNeighborPos(module->position + Cell3DPosition(-1, 0, 0));
        default:
            return NULL;
    }
}

void ShapeDecompositionBlockCode::onMotionEnd() {
    console << " has reached its destination" << "\n";

    // do stuff
    // ...
}

void ShapeDecompositionBlockCode::processLocalEvent(EventPtr pev) {
    std::shared_ptr<Message> message;
    stringstream info;

    // Do not remove line below
    BlinkyBlocksBlockCode::processLocalEvent(pev);

    switch (pev->eventType) {
        case EVENT_ADD_NEIGHBOR: {
            // Do something when a neighbor is added to an interface of the module
            break;
        }

        case EVENT_REMOVE_NEIGHBOR: {
            // Do something when a neighbor is removed from an interface of the module
            break;
        }
    }
}

/// ADVANCED BLOCKCODE FUNCTIONS BELOW

void ShapeDecompositionBlockCode::onBlockSelected() {
    // Debug stuff:
    for (auto &i_p : initiator_prevNext) {
        console << "initiator: " << i_p.first << " prev: " << i_p.second.first
                << " next: " << i_p.second.second << "\n";
    }

    // print nextDirections and prevDirections
    cout << "nextDirections: ";
    for (auto &dir : nextDirections) {
        cout << DIRECTIONS_NAMES[static_cast<short>(dir)] << " ";
    }
    cout << "\n";
    cout << "prevDirections: ";
    for (auto &dir : prevDirections) {
        cout << DIRECTIONS_NAMES[static_cast<short>(dir)] << " ";
    }
    cout << endl;

    cout << "prevCorner: " << prevCorner.position << " prev: " << (int)prevCorner.prevDir
         << " next: " << (int)prevCorner.nextDir << endl;

    // print borderInitiators
    for (auto &init : borderInitiators) {
        cout << "borderInitiators: " << DIRECTIONS_NAMES[static_cast<short>(init.first)] << " "
             << init.second << endl;
    }

    Cell3DPosition start(9, 2, 0);
    Cell3DPosition end(2, 5, 0);
    vector<Cell3DPosition> points = bresenhamLine(start, module->position);
    cout << "Bresenham line: ";
    for (auto &p : points) {
        cout << p << " ";
    }
    cout << endl;

    cout << "nbCorners: " << nbCorners << endl;

    // print corners
    // for (auto &corner : corners) {
    //     cout << "corner: " << corner.position << " prev: " <<
    //     DIRECTIONS_NAMES[corner.prevDir]
    //          << " next: " << DIRECTIONS_NAMES[corner.nextDir] << endl;
    // }

    // cout << "corners size: " << corners.size() << endl;
    // cout.flush();
    // vector<vector<Corner>> borders = getBorders();
    // cout << borders.size() << " borders\n";
    // print borders

    // for (auto &border : borders) {
    //     cout << "border " << border.size() << ": ";
    //     for (auto &corner : border) {
    //         cout << corner.position << " ";
    //     }
    //     cout << endl;
    // }
    // // print borders in bits
    // vector<bitset<8>> bitsetBorder;
    // for (auto &border : borders) {
    //     bitsetBorder.clear();
    //     for (auto &corner : border) {
    //         bitsetBorder.push_back(bitset<8>(corner.position.pt[0]));
    //         bitsetBorder.push_back(bitset<8>(corner.position.pt[1]));
    //         cout << bitset<8>(corner.position.pt[0]) << " " << bitset<8>(corner.position.pt[1])
    //              << " ";
    //     }
    //     cout << endl;
    //     vector<int> compressedBitsetBorder = LZWcompressBitset(bitsetBorder).first;
    //     // print compressed bitset
    //     cout << "compressed bitset: " << compressedBitsetBorder.size() << " : ";
    //     for (auto &b : compressedBitsetBorder) {
    //         cout << b << " ";
    //     }
    //     cout << endl;
    // }
    // cout << endl;
    // // print bridgeEnds
    // cout << "bridgeEnds: ";
    // for (auto &end : bridgeEnds) {
    //     if (end) {
    //         cout << end->getConnectedBlockId() << " ";
    //     } else {
    //         cout << "NULL ";
    //     }
    // }
    // cout << endl;
    // // test trace
    // if (module->blockId == 1)
    //     sendMessage("Trace", new MessageOf<int>(TRACE_MSG_ID, module->blockId),
    //                 getInterfaceInDirection(DIRECTIONS[(short)nextDirections[0]]), 1);
}

vector<vector<Corner>> ShapeDecompositionBlockCode::getBorders() {
    vector<vector<Corner>> borders;
    map<Direction, vector<Corner>> cornersByDirection;
    for (auto &corner : corners) {
        if (cornersByDirection.find(corner.prevDir) == cornersByDirection.end()) {
            cornersByDirection[corner.prevDir] = vector<Corner>();
        }
        cornersByDirection[corner.prevDir].push_back(corner);
    }
    while (!cornersByDirection.empty()) {
        vector<Corner> border;
        Corner minCorner = cornersByDirection.begin()->second[0];

        for (auto c : cornersByDirection) {
            for (auto &corner : c.second) {
                if (corner.position < minCorner.position) {
                    minCorner = corner;
                }
            }
        }
        cout << "minCorner: " << minCorner.position << endl;
        Corner next = minCorner;
        border.push_back(next);
        bool borderFound = false;
        while (!borderFound) {
            Direction nextDir = next.nextDir;
            short ligneDir = -1;
            if (nextDir == Direction::Right || nextDir == Direction::Left) {
                ligneDir = 1;
            } else if (nextDir == Direction::Front || nextDir == Direction::Back) {
                ligneDir = 0;
            }

            short minDist = SHRT_MAX;
            Corner curr = Corner(next.position, next.prevDir, next.nextDir);
            for (auto &corner : cornersByDirection[nextDir]) {
                if (corner.position.pt[ligneDir] == curr.position.pt[ligneDir] and
                    curr.position != corner.position) {
                    if (abs(corner.position.pt[(ligneDir + 1) % 2] -
                            curr.position.pt[(ligneDir + 1) % 2]) < minDist) {
                        next = corner;
                        // cout << "next: " << next.position << " " <<
                        // DIRECTIONS_NAMES[next.nextDir]
                        //      << endl;
                        minDist = abs(corner.position.pt[(ligneDir + 1) % 2] -
                                      curr.position.pt[(ligneDir + 1) % 2]);
                    }
                }
            }
            auto it =
                find(cornersByDirection[nextDir].begin(), cornersByDirection[nextDir].end(), next);
            if (it != cornersByDirection[nextDir].end()) {
                cornersByDirection[nextDir].erase(it);
                if (cornersByDirection[nextDir].empty()) {
                    cornersByDirection.erase(nextDir);
                }
            }

            // if (border.size() > 2) {
            //     Corner prev = border[border.size() - 1];
            //     Corner prevPrev = border[border.size() - 2];

            //     bool added = false;
            //     if (next.prevDir != next.nextDir) {
            //         // check if next is an external corner
            //         if (abs(next.position.pt[0] - prev.position.pt[0]) != 1 or
            //             abs(next.position.pt[1] - prev.position.pt[1]) != 1) {
            //             if (next.prevDir == Direction::Left and next.nextDir == Direction::Right)
            //             {
            //                 Cell3DPosition add =
            //                     Cell3DPosition(next.position.pt[0] + 1, next.position.pt[1], 0);
            //                 if (add != prev.position) {
            //                     border.push_back(Corner(add, prev.nextDir, next.prevDir));
            //                     lattice->getBlock(add)->setColor(GREEN);
            //                 }
            //                 prev = border[border.size() - 1];
            //                 prevPrev = border[border.size() - 2];
            //                 added = true;
            //             }
            //         }
            //     }
            //     if ((abs(next.position.pt[0] - prevPrev.position.pt[0]) == 1 and
            //          abs(next.position.pt[1] - prevPrev.position.pt[1]) == 1)) {
            //         border.erase(border.end() - 1);
            //         prev = border[border.size() - 1];
            //         prevPrev = border[border.size() - 2];
            //     }
            //     if (areColinear(prevPrev.position, prev.position, next.position) and
            //         abs(next.position.pt[0] - prev.position.pt[0]) == 1 and
            //         abs(next.position.pt[1] - prev.position.pt[1]) == 1) {
            //         border.erase(border.end() - 1);
            //     }
            //     border.push_back(next);

            // } else {
            border.push_back(next);
            // }
            // corners.erase(find(corners.begin(), corners.end(), next));
            if (next.position == border[0].position) {
                borders.push_back(border);
                borderFound = true;
            }
        }
    }
    return borders;
}

bool ShapeDecompositionBlockCode::areColinear(Cell3DPosition &p1, Cell3DPosition &p2,
                                              Cell3DPosition &p3) {
    return (p1[0] * (p2[1] - p3[1]) + p2[0] * (p3[1] - p1[1]) + p3[0] * (p1[1] - p2[1])) == 0;
}

bool ShapeDecompositionBlockCode::isIn(Cell3DPosition &pos, vector<vector<Corner>> &borders) {
    // Ray casting algorithm
    int intersections = 0;
    for (auto &border : borders) {
        for (int i = 0; i < border.size(); i++) {
            Cell3DPosition p1 = border[i].position;
            Cell3DPosition p2 = border[(i + 1) % border.size()].position;
            if (p1[1] == p2[1]) {
                if (p1[1] == pos[1] && ((p1[0] <= pos[0] && pos[0] <= p2[0]) ||
                                        (p2[0] <= pos[0] && pos[0] <= p1[0]))) {
                    return true;
                }
            } else {
                if ((p1[1] <= pos[1] && pos[1] < p2[1]) || (p2[1] <= pos[1] && pos[1] < p1[1])) {
                    double vt = (double)(pos[1] - p1[1]) / (p2[1] - p1[1]);
                    if (pos[0] < p1[0] + vt * (p2[0] - p1[0])) {
                        intersections++;
                    }
                }
            }
        }
    }
    return (intersections % 2) != 0;
}

// LZW implementation for compressing the borders
vector<int> ShapeDecompositionBlockCode::compress(vector<vector<Corner>> &borders) {
    vector<int> compressed;
    // map<vector<Corner>, int> dictionary;
    // for (int i = 0; i < borders.size(); i++) {
    //     dictionary[borders[i]] = i;
    // }
    // vector<Corner> w;
    // for (auto &k : borders[0]) {
    //     w.push_back(k);
    // }
    // for (int i = 1; i < borders.size(); i++) {
    //     vector<Corner> wk;
    //     for (auto &k : borders[i]) {
    //         wk.push_back(k);
    //     }
    //     if (dictionary.find(w) != dictionary.end()) {
    //         w = wk;
    //     } else {
    //         compressed.push_back(dictionary[w]);
    //         dictionary[w] = dictionary.size();
    //         w = wk;
    //     }
    // }
    // compressed.push_back(dictionary[w]);
    return compressed;
}

// LZW implementation for decompressing the borders
vector<vector<Corner>> ShapeDecompositionBlockCode::decompress(vector<int> &compressed) {
    vector<vector<Corner>> decompressed;
    // map<int, vector<Corner>> dictionary;
    // for (int i = 0; i < compressed.size(); i++) {
    //     vector<Corner> v;
    //     dictionary[i] = v;
    // }
    // int k = 0;
    // vector<Corner> w;
    // for (auto &k : dictionary[compressed[0]]) {
    //     w.push_back(k);
    // }
    // for (int i = 1; i < compressed.size(); i++) {
    //     vector<Corner> wk;
    //     if (dictionary.find(compressed[i]) != dictionary.end()) {
    //         wk = dictionary[compressed[i]];
    //     } else {
    //         wk = w;
    //         wk.push_back(w[0]);
    //     }
    //     decompressed.push_back(wk);
    //     if (dictionary.find(k) != dictionary.end()) {
    //         dictionary[k] = w;
    //     } else {
    //         dictionary[k] = w;
    //     }
    //     k++;
    //     w = wk;
    //}
    return decompressed;
}

bool ShapeDecompositionBlockCode::isOnBorder() const {
    return  (module->getNbNeighbors() < 4) ||
            (module->getInterface(SCLattice::Direction::Top)->isConnected() && module->getInterface(SCLattice::Direction::Bottom)->isConnected() && module->getNbNeighbors() < 6);
   
}

void ShapeDecompositionBlockCode::onAssertTriggered() {
    console << " has triggered an assert" << "\n";

    // Print debugging some info if needed below
    // ...
}

bool ShapeDecompositionBlockCode::parseUserCommandLineArgument(int &argc, char **argv[]) {
    /* Reading the command line */

    if ((argc > 0) && ((*argv)[0][0] == '-')) {
        switch ((*argv)[0][1]) {
            // Single character example: -b
            case 'b': {
                cout << "-b option provided" << endl;
                return true;
            } break;

            // Composite argument example: --foo 13
            case '-': {
                string varg = string((*argv)[0] + 2);  // argv[0] without "--"
                if (varg == string("foo")) {           //
                    int fooArg;
                    try {
                        fooArg = stoi((*argv)[1]);
                        argc--;
                        (*argv)++;
                    } catch (std::logic_error &) {
                        stringstream err;
                        err << "foo must be an integer. Found foo = " << argv[1] << endl;
                        throw CLIParsingError(err.str());
                    }

                    cout << "--foo option provided with value: " << fooArg << endl;
                } else
                    return false;

                return true;
            }

            default:
                cerr << "Unrecognized command line argument: " << (*argv)[0] << endl;
        }
    }

    return false;
}

string ShapeDecompositionBlockCode::onInterfaceDraw() {
    stringstream trace;
    trace << "Searching my shape" << "\n";
    return trace.str();
}

bool ShapeDecompositionBlockCode::isBackmost() const {
    return !lattice->cellHasBlock(module->position.offsetY(1));
}

bool ShapeDecompositionBlockCode::isFrontmost() const {
    return !lattice->cellHasBlock(module->position.offsetY(-1));
}

bool ShapeDecompositionBlockCode::isTopmost() const {
    return !lattice->cellHasBlock(module->position.offsetZ(1));
}

bool ShapeDecompositionBlockCode::isBottommost() const {
    return !lattice->cellHasBlock(module->position.offsetZ(-1));
}

bool ShapeDecompositionBlockCode::isRightmost() const {
    return !lattice->cellHasBlock(module->position.offsetX(1));
}

bool ShapeDecompositionBlockCode::isLeftmost() const {
    return !lattice->cellHasBlock(module->position.offsetX(-1));
}

bool ShapeDecompositionBlockCode::isExternalCorner() const {
    if ((isFrontmost() or isBackmost()) and (isLeftmost() or isRightmost())) return true;
    return false;
}

bool ShapeDecompositionBlockCode::isInternalCorner() const {
    if (isRightmost() and (lattice->cellHasBlock(module->position + Cell3DPosition(1, 1, 0)) or
                           lattice->cellHasBlock(module->position + Cell3DPosition(1, -1, 0))))
        return true;
    if (isLeftmost() and (lattice->cellHasBlock(module->position + Cell3DPosition(-1, 1, 0)) or
                          lattice->cellHasBlock(module->position + Cell3DPosition(-1, -1, 0))))
        return true;
    if (isBackmost() and (lattice->cellHasBlock(module->position + Cell3DPosition(-1, 1, 0)) or
                          lattice->cellHasBlock(module->position + Cell3DPosition(1, 1, 0))))
        return true;
    if (isFrontmost() and (lattice->cellHasBlock(module->position + Cell3DPosition(-1, -1, 0)) or
                           lattice->cellHasBlock(module->position + Cell3DPosition(1, -1, 0))))
        return true;
    return false;
}

bool ShapeDecompositionBlockCode::isBridgeEnd() const {
    bool isBridge = false;
    bool leftIsCorner = false;
    if (lattice->cellHasBlock(module->position + Cell3DPosition(-1, 0, 0))) {
        ShapeDecompositionBlockCode *leftModule = dynamic_cast<ShapeDecompositionBlockCode *>(
            lattice->getBlock(module->position + Cell3DPosition(-1, 0, 0))->blockCode);
        if (leftModule) {
            if (leftModule->isCorner) {
                leftIsCorner = true;
            }
        }
    }
    if (isFrontmost() && lattice->cellHasBlock(module->position + Cell3DPosition(1, -1, 0)) and
        !isLeftmost() && !leftIsCorner && module->blockId != 14) {
        isBridge = true;
    }
    return isBridge;
}

// bresenham algorithm to find points on a line
vector<Cell3DPosition> ShapeDecompositionBlockCode::bresenhamLine(Cell3DPosition &p1,
                                                                  Cell3DPosition &p2) {
    vector<Cell3DPosition> points;
    int x1 = p1.pt[0], y1 = p1.pt[1];
    int x2 = p2.pt[0], y2 = p2.pt[1];
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    while (true) {
        points.push_back(Cell3DPosition(x1, y1, 0));
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
    return points;
}

bool ShapeDecompositionBlockCode::isOnBresenhamLine(Cell3DPosition &p1, Cell3DPosition &p2,
                                                    Cell3DPosition &p3) {
    // Bresenham's line algorithm
    // Step 1: Check collinearity using the determinant method
    int x1 = p1.pt[0], y1 = p1.pt[1];
    int x2 = p2.pt[0], y2 = p2.pt[1];
    int x3 = p3.pt[0], y3 = p3.pt[1];

    // check if the slope between p1 and p2 == 1
    // if (x3 - x1 == y3 - y1) {
    //     return true;
    // }

    // if ((x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2)) == 0) {
    //     return true;  // Points are not collinear
    // }

    // check if the slope between p1 and p2 == 1
    // if (x3 - x1 == y3 - y1) {
    //     return true;
    // }

    // if ((x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2)) == 0) {
    //     return true;  // Points are not collinear
    // }

    // Step 2: Simulate Bresenham's algorithm from (x1, y1) to (x3, y3)
    int dx = std::abs(x3 - x1);
    int dy = std::abs(y3 - y1);
    int dist = abs(dx) + abs(dy);
    int err = dx - dy;            // Error term
    int sx = (x3 > x1) ? 1 : -1;  // Step direction for x
    int sy = (y3 > y1) ? 1 : -1;  // Step direction for y

    // double sx = static_cast<double>(dx) / dist;
    // double sy = static_cast<double>(dy) / dist;
    int x = x1, y = y1;
    while (x != x3 || y != y3) {   // Continue until the endpoint is reached
        if (x == x2 && y == y2) {  // Check if the intermediate point matches
            return true;
        }
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
    return false;  // (x2, y2) was not encountered in the Bresenham sequence
}

bool ShapeDecompositionBlockCode::isOnLine(Cell3DPosition &p1, Cell3DPosition &p2,
                                           Cell3DPosition &p3) {
    int x1 = p1.pt[0], y1 = p1.pt[1];
    int x2 = p2.pt[0], y2 = p2.pt[1];
    int x3 = p3.pt[0], y3 = p3.pt[1];
    float slope1 = (float)(y2 - y1) / (x2 - x1);
    float slope2 = (float)(y3 - y1) / (x3 - x1);
    return slope1 == slope2;
}

void ShapeDecompositionBlockCode::printResults(vector<Direction> &receivedDirections,
                                               vector<Corner> &receivedCorners) {
    // Open the file in read mode to check if it exists
    string configFile = getSimulator()->getCmdLine().getConfigFile();
    configFile = configFile.substr(0, configFile.find_last_of("."));
    string outputFileName = "results_" + configFile + ".json";
    ifstream inFile(outputFileName);
    bool fileExists = inFile.good();
    cout << getSimulator()->getCmdLine().getConfigFile();
    inFile.close();
    Cell3DPosition minPos = receivedCorners[0].position;
    nbCorners += receivedCorners.size();
    for(auto &corner : receivedCorners) {
        lattice->getBlock(corner.position)->setColor(GREEN);
        if(corner.position < minPos) {
            minPos = corner.position;
        }
    }
     lattice->getBlock(minPos)->setColor(RED);

    // Open the file in append mode if it exists, otherwise create a new file
    ofstream outFile;
    if (fileExists) {
        outFile.open(outputFileName, ios::in | ios::out);
        outFile.seekp(
            -6, ios::end);  // Move the cursor to the position before the last closing brackets
        outFile << ",\n";   // Close the previous border object
    } else {
        outFile.open(outputFileName);
        outFile << "{\n  \"borders\": [\n";
    }

    // Write the received directions in JSON format
    outFile << "    {\n";
    outFile << "      \"receivedDirections\": {\n";
    outFile << "        \"directions\": [";
    for (size_t i = 0; i < receivedDirections.size(); ++i) {
        outFile << (int)receivedDirections[i];
        if (i != receivedDirections.size() - 1) {
            outFile << ", ";
        }
    }
    outFile << "],\n";

    // Compress the directions using RLE
    vector<int> compressedDirections = RLEcompress(receivedDirections);
    outFile << "        \"RLEcompression\": {\n";
    outFile << "          \"originalSizeBytes\": " << receivedDirections.size() << ",\n";
    outFile << "          \"compressedSizeBytes\": " << compressedDirections.size() << ",\n";
    outFile << "          \"compressedDataBytes\": [";
    for (size_t i = 0; i < compressedDirections.size(); ++i) {
        outFile << compressedDirections[i];
        if (i != compressedDirections.size() - 1) {
            outFile << ", ";
        }
    }
    outFile << "]\n";
    outFile << "        },\n";

    // Compress the directions using LZW
    compressedDirections = LZWcompress(receivedDirections).first;
    int totalBits = LZWcompress(receivedDirections).second;
    outFile << "        \"LZWcompression\": {\n";
    outFile << "          \"originalSizeBytes\": " << receivedDirections.size() << ",\n";
    outFile << "          \"compressedSizeBytes\": " << compressedDirections.size() << ",\n";
    outFile << "          \"nbBitsCompressed\": " << totalBits << ",\n";
    outFile << "          \"compressedData\": [";
    for (size_t i = 0; i < compressedDirections.size(); ++i) {
        outFile << compressedDirections[i];
        if (i != compressedDirections.size() - 1) {
            outFile << ", ";
        }
    }
    outFile << "]\n";
    outFile << "        }\n";
    outFile << "      },\n";

    // Write the received corners in JSON format to the file
    outFile << "      \"receivedCorners\": {\n";
    outFile << "        \"corners\": [";
    for (size_t i = 0; i < receivedCorners.size(); ++i) {
        outFile << "[" << receivedCorners[i].position.pt[0] << ", "
                << receivedCorners[i].position.pt[1] <<", " << receivedCorners[i].position.pt[2] << "]";
        if (i != receivedCorners.size() - 1) {
            outFile << ", ";
        }
    }
    outFile << "],\n";

    vector<bitset<8>> bitsetBorder;
    bitsetBorder.clear();
    for (auto &corner : receivedCorners) {
        bitsetBorder.push_back(bitset<8>(corner.position.pt[0]));
        bitsetBorder.push_back(bitset<8>(corner.position.pt[1]));
    }
    vector<int> compressedBitsetBorder = LZWcompressBitset(bitsetBorder).first;
    totalBits = LZWcompressBitset(bitsetBorder).second;
    // print compressed bitset
    // Add compressed bitset border to the JSON file
    outFile << "        \"compressedBitsetBorder\": {\n";
    outFile << "          \"originalSizeBytes\": " << bitsetBorder.size() << ",\n";
    outFile << "          \"compressedSizeBytes\": " << compressedBitsetBorder.size() << ",\n";
    outFile << "          \"nbBitsCompressed\": " << totalBits << ",\n";
    outFile << "          \"compressedData\": [";
    for (size_t i = 0; i < compressedBitsetBorder.size(); ++i) {
        outFile << compressedBitsetBorder[i];
        if (i != compressedBitsetBorder.size() - 1) {
            outFile << ", ";
        }
    }
    outFile << "]\n";
    outFile << "        }\n";

    outFile << "      }\n";  // close receivedCorners
    outFile << "    }\n";    // close borders

    // Close the JSON array and object if the file was newly created
    if (!fileExists) {
        outFile << "  ]\n";
        outFile << "}";
    } else {
        outFile << "  ]\n";
        outFile << "}";
    }

    // Close the file
    outFile.close();
}

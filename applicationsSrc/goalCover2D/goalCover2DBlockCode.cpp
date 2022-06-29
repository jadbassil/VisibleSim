#include "goalCover2DBlockCode.hpp"
#include <queue>
#include <stack>

using namespace Catoms3D;

GoalCover2DBlockCode::GoalCover2DBlockCode(Catoms3DBlock *host) : Catoms3DBlockCode(host) {
    // @warning Do not remove block below, as a blockcode with a NULL host might be created
    //  for command line parsing
    if (not host) return;

    // Registers a callback (handleSampleMessage) to the message of type SAMPLE_MSG_ID
    addMessageEventFunc2(SAMPLE_MSG_ID,
                         std::bind(&GoalCover2DBlockCode::handleSampleMessage, this,
                                   std::placeholders::_1, std::placeholders::_2));

    // Set the module pointer
    module = static_cast<Catoms3DBlock *>(hostBlock);
}

int GoalCover2DBlockCode::col = 0;

void GoalCover2DBlockCode::startup() {
    console << "start\n";
    if (cellOnBorder(module->position)) {
        module->setColor(GREEN);
        setAreaBase(module->position);
        buildTree();
        myArea.clear();
        partition(areaRoot, borderSize);
        cleanTree();
        updateTree();
        console << "entryPoint: " << entryPoint << "\n";
        bool isBorder = false;
        if (countNodes(module->position, isBorder, 0) > 0 ) {
            module->setColor(Colors[col]);
            highlightRegion(col);
            col++;
        } else {
            module->setColor(GREY);
        }

    }
}

bool GoalCover2DBlockCode::cellOnBorder(Cell3DPosition pos) {
    if (lattice->cellHasBlock(pos)) {
        for (const auto &cell: lattice->getFreeNeighborCells(pos)) {
            if (target->isInTarget(cell)) {
                return true;
            }
        }
    }
    return false;
}

void GoalCover2DBlockCode::setAreaBase(Cell3DPosition &start) {
    if (not cellOnBorder(start)) {
        return;
    }
    queue<Cell3DPosition> Q;
    Cell3DPosition base = start;
    Q.push(start);
    while (!Q.empty()) {
        Cell3DPosition cur = Q.front();
        Q.pop();
        if (!myArea.insert(cur).second) {
            continue;
        } else if (cellOnBorder(cur)) {
            borderSize++;
        }

        for (auto cell: lattice->getNeighborhood(cur)) {
            if ((not lattice->cellHasBlock(cell) and target->isInTarget(cell))
                or cellOnBorder(cell)) {
                if (myArea.find(cell) == myArea.end()) {
                    Q.push(cell);
                }
            }
        }
    }
    for(auto it = myArea.begin(); it != myArea.end(); it++) {
        console << *it << "\n";
        if(cellOnBorder(*it)){
            areaRoot = *it;
            break;
        }

    }
/*
    areaRoot = *myArea.begin();
*/
}

map<Cell3DPosition, set<Cell3DPosition>> GoalCover2DBlockCode::buildTree() {
    queue<Cell3DPosition> Q;
    Q.push(areaRoot);
    while (!Q.empty()) {
        Cell3DPosition cur = Q.front();
        Q.pop();
        if (cellOnBorder(cur) and cur != areaRoot) {
            // force region's border cell to be leaves and skip the root
            continue;
        }
        for (auto cell: lattice->getNeighborhood(cur)) {
            if ((not lattice->cellHasBlock(cell) and target->isInTarget(cell))
                or (cellOnBorder(cell) and  cur != areaRoot)) {
                if (T.find(cell) == T.end()) {
                    T[cur].insert(cell);
                    T[cell] = set<Cell3DPosition>();
                    if (module->position == cell) {
                        parent = cur;
                    }
                    Q.push(cell);
                }
            }
        }
    }
    return T;
}

int GoalCover2DBlockCode::countNodes(Cell3DPosition node, bool &isBorder, int c) {
    if (cellOnBorder(node) and T[node].empty()) isBorder = true;
    if (T[node].empty())
        return 0;
    for (auto n: T[node]) {
        c += countNodes(n, isBorder) + 1;
    }
    return c;
}

pair<Cell3DPosition, Cell3DPosition> GoalCover2DBlockCode::findCut(Cell3DPosition &node) {
    int minimum = INT32_MAX;
    Cell3DPosition cutAt;
    Cell3DPosition parent;
    for (auto kv: T) {
        if (kv.first == areaRoot) continue;
        for (auto c: kv.second) {
            if ((T[c].empty() and cellOnBorder(c)))
                continue;
            bool isBorderAfter = false;
            bool isBorderBefore = true;

            int after = countNodes(c, isBorderAfter) + 1;
            int before = countNodes(node, isBorderBefore) - after;
            int diff = abs(after - before);
            if (isBorderAfter and diff < minimum) {
                cutAt = c;
                parent = kv.first;
                minimum = diff;
            }
        }

    }
    if (minimum == INT32_MAX) cutAt = node;
    else console << "cut: " << parent << "->" << cutAt << ": " << minimum << "\n";
    return make_pair(parent, cutAt);
}

Cell3DPosition GoalCover2DBlockCode::cut(Cell3DPosition &node) {
    pair<Cell3DPosition, Cell3DPosition> cutEdge = findCut(node);
    if (cutEdge.first != Cell3DPosition(0, 0, 0))
        T[cutEdge.first].erase(cutEdge.second);
    return cutEdge.second;
}

void GoalCover2DBlockCode::partition(Cell3DPosition &node, int k) {
    if (k < 2) return;
    Cell3DPosition cutAt = cut(node);
    queue<Cell3DPosition> result;
    result.push(node);
    console << "cutAt: " << cutAt << "\n";
    result.push(cutAt);
    for (int i = 2; i < k; i++) {
        Cell3DPosition root = result.front();
        result.pop();
        cutAt = cut(root);
        console << "cutAt: " << cutAt << "\n";
        result.push(cutAt);
        result.push(root);
    }
}

void GoalCover2DBlockCode::cleanTree() {
    Cell3DPosition myRoot = findRoot(module->position);
    for (auto it = T.cbegin(); it != T.cend();) {
        if (findRoot(it->first) != myRoot) {
            T.erase(it++);
        } else {
            ++it;
        }
    }

}

void GoalCover2DBlockCode::updateTree() {
    map<Cell3DPosition, set<Cell3DPosition>> tmp;
    if (module->position == areaRoot) {
        entryPoint = areaRoot;
        for (auto it = T.begin(); it != T.end();) {
            if (cellOnBorder(it->first) and it->first != entryPoint) {
                Cell3DPosition p = findParent(const_cast<Cell3DPosition &>(it->first));
                if (p != Cell3DPosition(-1, -1, -1)) {
                    T[p].erase(it->first);
                }
                T.erase(it++);
            } else {
                ++it;
            }
        }
        return;
    }
    //redirect the nodes
    entryPoint = module->position;
    for (auto kv: T) {
        if (not kv.second.empty()) {
            for (auto it = kv.second.begin(); it != kv.second.end(); it++) {
                if (cellOnBorder(*it)) {
                    tmp[*it].insert(kv.first);
                    if (*it < module->position) {
                        entryPoint = *it;
                    }
                } else {
                    tmp[kv.first].insert(*it);

                    tmp[*it] = set<Cell3DPosition>();

                }

            }

        }
    }
    if (module->position != entryPoint) {
        tmp.clear();
        T.clear();
        return;
    } else {
        for (auto it = tmp.begin(); it != tmp.end();) {
            if (cellOnBorder(it->first) and it->first != entryPoint) {
                Cell3DPosition p = findParent(const_cast<Cell3DPosition &>(it->first));
                if (p != Cell3DPosition(-1, -1, -1)) {
                    tmp[p].erase(it->first);
                }
                tmp.erase(it++);
            } else {
                ++it;
            }
        }
    }
    if (not tmp.empty())
        T = tmp;
}

Cell3DPosition GoalCover2DBlockCode::findParent(Cell3DPosition &node) {
    for (auto kv: T) {
        if (kv.second.find(node) != kv.second.end()) {
            return kv.first;
        }
    }
    return Cell3DPosition(-1, -1, -1);
}

Cell3DPosition GoalCover2DBlockCode::findRoot(Cell3DPosition node) {
    Cell3DPosition r = findParent(node);
    if (r == Cell3DPosition(-1, -1, -1)) {
        return node;
    }
    return findRoot(r);
}

void GoalCover2DBlockCode::highlightRegion(int c) {
    if (T.empty()) return;
    for (auto kv: T) {
        if (not lattice->cellHasBlock(kv.first)) {
            lattice->highlightCell(kv.first,
                                   Colors[col]);
        }
    }
}

void GoalCover2DBlockCode::handleSampleMessage(std::shared_ptr<Message> _msg,
                                               P2PNetworkInterface *sender) {
    MessageOf<int> *msg = static_cast<MessageOf<int> *>(_msg.get());

    int d = *msg->getData() + 1;
    console << " received d =" << d << " from " << sender->getConnectedBlockId() << "\n";

    if (distance == -1 || distance > d) {
        console << " updated distance = " << d << "\n";
        distance = d;
        module->setColor(Colors[distance % NB_COLORS]);

        // Broadcast to all neighbors but ignore sender
        sendMessageToAllNeighbors("Sample Broadcast",
                                  new MessageOf<int>(SAMPLE_MSG_ID, distance), 100, 200, 1, sender);
    }
}

void GoalCover2DBlockCode::onMotionEnd() {
    console << " has reached its destination" << "\n";

    // do stuff
    // ...
}

void GoalCover2DBlockCode::processLocalEvent(EventPtr pev) {
    std::shared_ptr<Message> message;
    stringstream info;

    // Do not remove line below
    Catoms3DBlockCode::processLocalEvent(pev);

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

void GoalCover2DBlockCode::onBlockSelected() {
    // Debug stuff:
    cerr << endl << "--- PRINT MODULE " << *module << "---" << endl;
    cerr << "myArea: \n\t";
    for (auto &c: myArea) {
        cerr << c << ", ";
    }
    cerr << endl;
    cerr << "myArea size: " << myArea.size() << endl;
    cerr << "areaRoot: " << areaRoot << endl;
    cerr << "borderSize: " << borderSize << endl;
    cerr << "T: \n\t";
    for (auto kv: T) {
        cerr << kv.first << ": ";
        for (auto c: kv.second) {
            cerr << c << ", ";
        }
        cerr << "\n\t";
    }
    cerr << endl;
    cerr << "T size: " << T.size() << endl;
    cerr << "parent: " << parent << endl;
    bool isBorder = false;
    cerr << "countNodes: " << countNodes(entryPoint, isBorder, 0) << endl;
}

void GoalCover2DBlockCode::onAssertTriggered() {
    console << " has triggered an assert" << "\n";

    // Print debugging some info if needed below
    // ...
}

bool GoalCover2DBlockCode::parseUserCommandLineArgument(int &argc, char **argv[]) {
    /* Reading the command line */
    if ((argc > 0) && ((*argv)[0][0] == '-')) {
        switch ((*argv)[0][1]) {

            // Single character example: -b
            case 'b': {
                cout << "-b option provided" << endl;
                return true;
            }
                break;

                // Composite argument example: --foo 13
            case '-': {
                string varg = string((*argv)[0] + 2); // argv[0] without "--"
                if (varg == string("foo")) { //
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
                } else return false;

                return true;
            }

            default:
                cerr << "Unrecognized command line argument: " << (*argv)[0] << endl;
        }
    }

    return false;
}

string GoalCover2DBlockCode::onInterfaceDraw() {
    stringstream trace;
    trace << "Some value " << 123;
    return trace.str();
}


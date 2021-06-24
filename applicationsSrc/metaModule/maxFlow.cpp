

#include "metaModuleBlockCode.hpp"
#include "maxFlow.hpp"

MaxFlow::MaxFlow(/* args */) {
    graph = Graph();
}

MaxFlow::MaxFlow(Catoms3DBlock *_coordinator) : coordinator(_coordinator) {
    graph = Graph();
}

MaxFlow::~MaxFlow() {}

void MaxFlow::initGraph() {
    // assuming empty MM only
    vector<Cell3DPosition> *positions;
    MetaModuleBlockCode *MMCoordinator = static_cast<MetaModuleBlockCode *>(coordinator->blockCode);
    positions = (MMCoordinator->shapeState == FRONTBACK) ? &FrontBackMM : &BackFrontMM;
    cout << "unblocked pos: " << endl;
    BaseSimulator::getWorld()->addBlock(9999, MetaModuleBlockCode::buildNewBlockCode,
                                        Cell3DPosition(1, 1, 1), Color(WHITE));

    MetaModuleBlockCode *virtualBlock = static_cast<MetaModuleBlockCode *>(
        BaseSimulator::getWorld()->getBlockById(9999)->blockCode);

    virtualBlock->targetPosition = Cell3DPosition(13, 10, 12);
    virtualBlock->coordinatorPosition = coordinator->position;

    for (auto pos : *positions) {
        Catoms3DBlock *block = static_cast<Catoms3DBlock *>(
            BaseSimulator::getWorld()->getBlockByPosition(MMCoordinator->seedPosition + pos));
        // vector<Cell3DPosition> reachablePositions =
        //     Catoms3DMotionEngine::getAllReachablePositions(block->module);
        vector<Cell3DPosition> nextPositions;
        for (auto freePos :
             BaseSimulator::getWorld()->lattice->getFreeNeighborCells(block->position)) {
            if (block->canMoveTo(freePos)) nextPositions.push_back(freePos);
        }
        if (nextPositions.size() > 0) {
            graphMap[block->position] = nextPositions;
            for (auto rp : nextPositions) {
                graph.addEdge(block->position, rp, 1);
                 if (find(virtualBlock->teleportingPositions.begin(), virtualBlock->teleportingPositions.end(), rp) ==
                            virtualBlock->teleportingPositions.end()) virtualBlock->teleportingPositions.push_back(rp);
            }
            // for(auto pos: nextPositions) {
            //     fillGraph(pos, Cell3DPosition(13,10,12));
            // }
        }
    }
    Cell3DPosition nextTeleportingPosition = virtualBlock->teleportingPositions.front();

    virtualBlock->teleportingPositions.pop_front();
    virtualBlock->teleporting = true;
    getScheduler()->schedule(new TeleportationStartEvent(
        getScheduler()->now(), virtualBlock->module, nextTeleportingPosition));
}

void MaxFlow::fillGraph(Cell3DPosition &initPos, Cell3DPosition targetPos) {
    // if(graph.find(initPos) != graph.end()
    //     or initPos == targetPos or !neighInMM(initPos)) {
    //     return;
    // }
    if (graph.adj.find(initPos) != graph.adj.end() or initPos == targetPos or !neighInMM(initPos)) {
        return;
    }
    MetaModuleBlockCode *MMCoordinator = static_cast<MetaModuleBlockCode *>(coordinator->blockCode);
    MetaModuleBlockCode *virtualBlock = static_cast<MetaModuleBlockCode *>(
        BaseSimulator::getWorld()->getBlockById(9999)->blockCode);
    virtualBlock->coordinatorPosition = MMCoordinator->module->position;
    cerr << "fill\n";
    if (BaseSimulator::getWorld()->lattice->isFree(initPos) and !virtualBlock->teleporting) {
        virtualBlock->teleporting = true;
        getScheduler()->schedule(
            new TeleportationStartEvent(getScheduler()->now(), virtualBlock->module, initPos));
    } else if (virtualBlock->teleporting) {
        virtualBlock->teleportingPositions.push_back(initPos);
        getScheduler()->schedule(new InterruptionEvent(getScheduler()->now(), virtualBlock->module,
                                                       IT_MODE_TELEPORTING));
    }

    // BaseSimulator::getWorld()->addBlock(0, MetaModuleBlockCode::buildNewBlockCode, initPos,
    // Color(GREY)); MetaModuleBlockCode *block = static_cast<MetaModuleBlockCode*>(
    //     BaseSimulator::getWorld()->getBlockByPosition(initPos)->blockCode
    // );
    // vector<Cell3DPosition> reachablePositions =
    //         Catoms3DMotionEngine::getAllReachablePositions(block->module);
    // Cell3DPosition blockPosition = block->module->position;
    // BaseSimulator::getWorld()->lattice->highlightCell(blockPosition);
    // BaseSimulator::getWorld()->deleteBlock(block->module);
    // if(reachablePositions.size() > 0) {
    //     // graph[blockPosition] = reachablePositions;

    //     for(auto pos: reachablePositions) {
    //         graph.addEdge(block->module->position, pos, 1);
    //         fillGraph(pos, Cell3DPosition(13,10,12));
    //     }
    // }
}

bool MaxFlow::neighInMM(Cell3DPosition &pos) {
    MetaModuleBlockCode *MMCoordinator = static_cast<MetaModuleBlockCode *>(coordinator->blockCode);
    vector<Cell3DPosition> activeCells = MMCoordinator->lattice->getActiveNeighborCells(pos);
    for (auto p : activeCells) {
        if (MMCoordinator->isInMM(p)) return true;
    }
    return false;
}

void MaxFlow::printGraph() {
    // for(auto x: graphMap) {
    //     cerr << x.first << ": ";
    //     for(auto y: x.second)
    //         cerr << y << "; ";
    //     cerr << endl;
    // }
    for (auto x : graph.adj) {
        cerr << x.first << ": ";
        for (auto y : x.second) cerr << y.v << "; ";
        cerr << endl;
    }
    for (auto x : graph.rev) {
        cerr << x.first << ": ";
        for (auto y : x.second) cerr << y.v << "; ";
        cerr << endl;
    }

//    graph.DinicMaxflow(Cell3DPosition(16, 9, 12),
//                                                Cell3DPosition(13, 10, 12));
}

bool MaxFlow::BFS(map<Cell3DPosition, map<Cell3DPosition, int>>& rGraph, Cell3DPosition& s,
             Cell3DPosition& t, map<Cell3DPosition, Cell3DPosition>& parent){

    map<Cell3DPosition, bool> visited;
    for(auto i: rGraph) {
        visited[i.first] = false;
    }

    queue<Cell3DPosition> q;
    q.push(s);
    visited[s] = true;
    //cerr << "path: ";
    while(not q.empty()) {
        Cell3DPosition u = q.front();
        q.pop();
        //cerr << u << ": ";
        for(auto v: rGraph[u]) {
            if(visited[v.first] == false and v.second > 0) {
                //cerr << v.first << " ";
                if(v.first == t) {
                    parent[v.first] = u;
                    return true;
                }
                q.push(v.first);
                parent[v.first] = u;
                visited[v.first] = true;
            }
        }
       // cerr << endl;
    }
    // can't reach sink in BFS from source
    return false;
}

 int MaxFlow::fordFulkerson(Cell3DPosition& s, Cell3DPosition& t) {
    Cell3DPosition u, v;

    map<Cell3DPosition, map<Cell3DPosition,int>> rGraph;

    for(auto i: graphMap) {
        for(auto j: graphMap[i.first]) {
            rGraph[i.first].insert(make_pair(j, 1));
        }
    }

    int max_flow = 0;
    map<Cell3DPosition, Cell3DPosition> parent;

    while(BFS(rGraph, s, t, parent)) {
        int path_flow = INT_MAX;

        for(v = t; v != s; v = parent[v]) {
            u = parent[v];
            // vector<pair<Cell3DPosition, int>>::iterator it =
            //     find(rGraph[u].begin(), rGraph[u].end(), v);
            path_flow = min(path_flow, rGraph[u][v]);
        }
        cerr << "path: " << endl;
        cerr << t << " ";
        for (v = t; v != s; v = parent[v]) {
            u = parent[v];
            cerr << u << " ";
            // vector<pair<Cell3DPosition, int>>::iterator itv =
            //     find(rGraph[u].begin(), rGraph[u].end(), v);
            // vector<pair<Cell3DPosition, int>>::iterator itu =
            //     find(rGraph[v].begin(), rGraph[v].end(), u);
            // itv->second -= path_flow; 
             rGraph[u][v] -= path_flow;
            //itu->second += path_flow; 
             rGraph[v][u] += path_flow; 
        }
        cerr << endl;
        max_flow += path_flow;
    }
    return max_flow;
 }
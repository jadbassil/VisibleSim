#include <climits>
#include <queue>
#include <stack>

#include "metaModuleBlockCode.hpp"
#include "robots/catoms3D/catoms3DMotionEngine.h"

#include "maxFlow.hpp"

MaxFlow::MaxFlow(/* args */) {}

MaxFlow::MaxFlow(Catoms3DBlock *_coordinator) : coordinator(_coordinator) {}

MaxFlow::~MaxFlow() {}

const Cell3DPosition MaxFlow::VirtualSource = Cell3DPosition(-2,-2,-2); 
const Cell3DPosition MaxFlow::VirtualSink = Cell3DPosition(-3,-3,-3);

void MaxFlow::setSources() { // TODO: choose sources in a way to avoid meta-module disconnection
    if(not coordinator)
        return;
    sources.clear();
    MetaModuleBlockCode *MMCoordinator = static_cast<MetaModuleBlockCode *>(coordinator->blockCode);
    vector<Cell3DPosition> *positions;
    positions = (MMCoordinator->shapeState == FRONTBACK) ? &FrontBackMM : &BackFrontMM;

    for(auto pos: *positions) {
        Catoms3DBlock *block = static_cast<Catoms3DBlock *>(
            BaseSimulator::getWorld()->getBlockByPosition(MMCoordinator->seedPosition + pos));
        vector<Cell3DPosition> nextPositions;
        for (auto freePos :
             BaseSimulator::getWorld()->lattice->getFreeNeighborCells(block->position)) {
            if (block->canMoveTo(freePos)) {
                sources.push_back(MMCoordinator->seedPosition + pos);
                break;
            }
        }
    }
}

void MaxFlow::initGraph() {
    // assuming empty MM only
    MetaModuleBlockCode *MMCoordinator = static_cast<MetaModuleBlockCode *>(coordinator->blockCode);
    BaseSimulator::getWorld()->addBlock(9999, MetaModuleBlockCode::buildNewBlockCode,
                                        Cell3DPosition(1, 1, 1), Color(WHITE));

    MetaModuleBlockCode *virtualBlock = static_cast<MetaModuleBlockCode *>(
        BaseSimulator::getWorld()->getBlockById(9999)->blockCode);

    virtualBlock->targetPosition = Cell3DPosition(13, 10, 12);
    virtualBlock->coordinatorPosition = coordinator->position;

    setSources();

    Cell3DPosition src = VirtualSource;
    graphMap[src] = sources;

    Cell3DPosition t = VirtualSink;
    for(auto sink: sinks) {
        graphMap[sink].push_back(t);
    }

    for (auto pos : sources) {
        Catoms3DBlock *block = static_cast<Catoms3DBlock *>(
            BaseSimulator::getWorld()->getBlockByPosition(pos));
        // vector<Cell3DPosition> reachablePositions =
        //     Catoms3DMotionEngine::getAllReachablePositions(block->module);
        vector<Cell3DPosition> nextPositions;
        for (auto freePos :
             BaseSimulator::getWorld()->lattice->getFreeNeighborCells(block->position)) {
            if (block->canMoveTo(freePos) and
                find(sources.begin(), sources.end(),
                     Catoms3DMotionEngine::findMotionPivot(block, freePos)->position) ==
                    sources.end()) {
                nextPositions.push_back(freePos);
            }
        }
        if (nextPositions.size() > 0) {
            graphMap[block->position] = nextPositions;
            for (auto rp : nextPositions) {
                if (find(virtualBlock->teleportingPositions.begin(),
                         virtualBlock->teleportingPositions.end(),
                         rp) == virtualBlock->teleportingPositions.end())
                    virtualBlock->teleportingPositions.push_back(rp);
            }
        }
    }
    Cell3DPosition nextTeleportingPosition = virtualBlock->teleportingPositions.front();

    virtualBlock->teleportingPositions.pop_front();
    virtualBlock->teleporting = true;
    getScheduler()->schedule(new TeleportationStartEvent(
        getScheduler()->now(), virtualBlock->module, nextTeleportingPosition));
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
    for(auto x: graphMap) {
        cerr << x.first << ": ";
        for(auto y: x.second)
            cerr << y << "; ";
        cerr << endl;
    }
    // for (auto x : graph.adj) {
    //     cerr << x.first << ": ";
    //     for (auto y : x.second) cerr << y.v << "; ";
    //     cerr << endl;
    // }
    // for (auto x : graph.rev) {
    //     cerr << x.first << ": ";
    //     for (auto y : x.second) cerr << y.v << "; ";
    //     cerr << endl;
    // }
}

bool MaxFlow::BFS(map<Cell3DPosition, map<Cell3DPosition, int>>& rGraph, Cell3DPosition& s,
             Cell3DPosition& t, map<Cell3DPosition, Cell3DPosition>& parent){

    map<Cell3DPosition, bool> visited;
    for(auto i: rGraph) {
        visited[i.first] = false;
    }

    // queue<Cell3DPosition> q;
    // q.push(s);
    // visited[s] = true;
    // while(not q.empty()) {
    //     Cell3DPosition u = q.front();
    //     q.pop();
    //     for(auto v: rGraph[u]) {
    //         if(visited[v.first] == false and v.second > 0) {
    //             //cerr << v.first << " ";
    //             if(v.first == t) {
    //                 parent[v.first] = u;
    //                 return true;
    //             }
    //             q.push(v.first);
    //             parent[v.first] = u;
    //             visited[v.first] = true;
    //         }
    //     }
    // }
    stack<Cell3DPosition> q;
    q.push(s);
    visited[s] = true;
    while(not q.empty()) {
        // Cell3DPosition u = q.front();
        Cell3DPosition u = q.top();
        q.pop();
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
    }
    // can't reach sink in BFS from source
    return false;
}

 int MaxFlow::fordFulkerson(Cell3DPosition& s, Cell3DPosition& t, deque<deque<Cell3DPosition>> &paths) {
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
            path_flow = min(path_flow, rGraph[u][v]);
        }
        //cerr << "path: " << endl;
        //cerr << t << " ";
        paths.push_back(deque<Cell3DPosition>());
        for (v = t; v != s; v = parent[v]) {
            u = parent[v];
            if(u != MaxFlow::VirtualSource){
                //cerr << u << " ";
                paths.back().push_front(u);
            } 
            BaseSimulator::getWorld()->lattice->highlightCell(u);
            rGraph[u][v] -= path_flow;
            rGraph[v][u] += path_flow; 
        }
        //cerr << endl;
        max_flow += path_flow;
    }
    return max_flow;
 }
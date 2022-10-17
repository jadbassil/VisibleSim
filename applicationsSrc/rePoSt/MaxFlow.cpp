//
// Created by jbassil on 14/10/22.
//

#include "MaxFlow.hpp"

map<Cell3DPosition, map<Cell3DPosition, int>> MaxFlow::graph = map<Cell3DPosition, map<Cell3DPosition, int>>();


MaxFlow::MaxFlow(RePoStBlockCode *rbc) : rbc(rbc) {
    nbMM = RePoStBlockCode::initialMap.size();
    //if(MaxFlow::graph.empty()) buildGraph();
    if(cellOnBorder(rbc->MMPosition)) {
        goalCover = new GoalCover(*rbc);
        goalCover->buildGraph();
        if(!goalCover->graph[rbc->MMPosition].empty()) {
            rbc->isDestination = true;
        }
        graph[rbc->MMPosition][sink] = goalCover->getEntryFlow();
    }
}

MaxFlow::~MaxFlow() {

}

bool MaxFlow::cellOnBorder(Cell3DPosition &cell) {
    if (rbc->lattice->cellHasBlock(rbc->getSeedPositionFromMMPosition(cell))) {
        auto *crbc = dynamic_cast<RePoStBlockCode *>(rbc->lattice->getBlock(rbc->getSeedPositionFromMMPosition(cell))->blockCode);
        for (auto &adjPos: crbc->getAdjacentMMPositions()) {
            if (not rbc->inInitialShape(adjPos) and rbc->inTargetShape(adjPos)) {
                return true;
            }
        }
    }

    return false;
}

void MaxFlow::buildGraph() {

    vector<Cell3DPosition> destinations;
    vector<Cell3DPosition> sources;
    vector<Cell3DPosition> intermediates;
    for(auto MM: RePoStBlockCode::initialMap) {
        Cell3DPosition MMPos(MM[0], MM[1], MM[2]) ;
        if(not isIn( RePoStBlockCode::targetMap, MM)) {
            sources.push_back(MMPos);
        } else if (cellOnBorder(MMPos)){
            destinations.push_back(MMPos);
        } else {
            intermediates.push_back(MMPos);
        }
    }
    cerr << "sources: ";
    for(auto &src: destinations) cerr << src << " ";
    cerr << endl;
    for(auto src: sources) {
        for(auto MM: RePoStBlockCode::initialMap) {
            Cell3DPosition MMPos(MM[0], MM[1], MM[2]) ;
            if(cellsAreAdjacent(src, MMPos)) {
                graph[src][MMPos] = nbMM;
            }
        }
        /*for(auto i: intermediates) {
            if(cellsAreAdjacent(src, i)) {
                graph[src][i] = nbMM;
            }
        }
        for(auto j: sources) {
            if(cellsAreAdjacent(src, j)) {
                graph[src][j] = nbMM;
            }
        }

        for(auto k: destinations) {
            if(cellsAreAdjacent(src, k)) {
                graph[src][k] = nbMM;
            }
        }*/
    }

    for(auto n1: intermediates) {
        for(auto n2: intermediates) {
            if(cellsAreAdjacent(n1, n2)) {
                graph[n1][n2] = nbMM;
            }
        }
    }

//    for (auto d: destinations) {
//        for (auto node: graph) {
//            if (cellsAreAdjacent(d, node.first) and not isIn(sources, node.first) and
//                not isIn(destinations, node.first)) {
//                graph[node.first][d] = nbMM;
//            }
//        }
//    }

    for(auto d: destinations) {
        for (auto MM: RePoStBlockCode::initialMap) {
            Cell3DPosition MMPos(MM[0], MM[1], MM[2]);
            if (cellsAreAdjacent(d, MMPos)) {
                graph[MMPos][d] = nbMM;
            }
        }
    }

    for(auto src: sources) {
        graph[source][src] = 1;
    }

    maxFlow();
}

bool MaxFlow::cellsAreAdjacent(Cell3DPosition p1, Cell3DPosition p2) {
    return abs(p1[0] - p2[0]) + abs(p1[1] - p2[1]) + abs(p1[2] - p2[2]) == 1;
}

bool MaxFlow::bfs(map<Cell3DPosition, map<Cell3DPosition, int>> &G, const Cell3DPosition& s, const Cell3DPosition& t,
                    map<Cell3DPosition, Cell3DPosition> &parent) {
    map<Cell3DPosition, bool> visited;
    queue<Cell3DPosition> Q;
    Q.push(s);
    visited[s] = true;
    parent[s] = Cell3DPosition(-30, -30, -30);
    while (not Q.empty()) {
        Cell3DPosition cur = Q.front();
        Q.pop();
        for(auto &edge: G[cur]) {
            if(not visited[edge.first] and edge.second > 0) {
                if(edge.first == t) {
                    parent[edge.first] = cur;
                    return true;
                }
                Q.push(edge.first);
                parent[edge.first] = cur;
                visited[edge.first] = true;
            }
        }
    }
    return false;
}

int MaxFlow::maxFlow() {
    Cell3DPosition u,v;
    map<Cell3DPosition, map<Cell3DPosition, int>> rGraph(graph);

    map<struct Cell3DPosition, struct Cell3DPosition> parent;
    int max = 0;
    while(bfs(rGraph, source, sink, parent)) {
        int pathFlow = INT32_MAX;
        for (v = sink; v != source; v = parent[v]) {
            u = parent[v];
            pathFlow = min(pathFlow, rGraph[u][v]);
        }
        for (v = sink; v != source; v = parent[v]) {
            u = parent[v];
            rGraph[u][v] -= pathFlow;
            rGraph[v][u] += pathFlow;
        }
        max += pathFlow;
    }
    cerr << "Flow: " << max << endl;
/*    printGraph();
    cerr << "RGraph: " ;
    for(const auto& node: rGraph) {
        cerr << endl << node.first << ": ";
        for (const auto& edge: node.second) {
            cerr << "(" << edge.first << ", " << edge.second << ") ; ";
        }
    }*/
    for(auto node: graph) {
        for(auto edge: node.second) {
            graph[node.first][edge.first] -= rGraph[node.first][edge.first];
            if (graph[node.first][edge.first] <= 0 /*or edge.first == sink*/) {
                graph[node.first].erase(edge.first);
            }
        }
    }

    return max;
}

void MaxFlow::printGraph() {
    cerr << "Graph: " ;
    for(const auto& node: graph) {
        cerr << endl << node.first << ": ";
        for (const auto& edge: node.second) {
            cerr << "(" << edge.first << ", " << edge.second << ") ; ";
        }
    }
}

int MaxFlow::totalNbDest() {
    int c = 0;
    for(auto MM: rbc->initialMap) {
        Cell3DPosition MMPos(MM[0], MM[1], MM[2]) ;
        if(cellOnBorder(MMPos)) c++;
    }
    return c;
}

map<Cell3DPosition, int> MaxFlow::getLocalPaths() {
    return graph[rbc->MMPosition];
}

void MaxFlow::printLocalConnections() {
    cerr << "Local Connections: ";
    if(not graph[rbc->MMPosition].empty()) {
        for(auto &edge: graph[rbc->MMPosition]) {
            cerr << "(" << edge.first << ", " << edge.second << ") ; ";
        }
    } else {
        cerr << "No Connections!\n";
    }
}

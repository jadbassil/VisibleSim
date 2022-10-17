//
// Created by jbassil on 07/10/22.
//

#include "goalCover.hpp"
#include <queue>

GoalCover::GoalCover(RePoStBlockCode &_rbc){
    rbc = &_rbc;
}

GoalCover::~GoalCover() = default;

bool GoalCover::cellOnBorder(Cell3DPosition &cell) {
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

void GoalCover::buildGraph() {
    queue<Cell3DPosition> Q;
    Q.push(rbc->MMPosition);
    int nbOfBorderPositions = 0;
    while (not Q.empty()) {
        Cell3DPosition cur = Q.front();
        Q.pop();

        if (graph[cur].empty()) {
            if(cellOnBorder(cur)) {
                nbOfBorderPositions++;
            }
            for (auto &adjPos: RePoStBlockCode::getMMNeighborhood(cur)) {
                if ((not rbc->lattice->cellHasBlock(rbc->getSeedPositionFromMMPosition(
                        adjPos)) and rbc->inTargetShape(adjPos)) or cellOnBorder(adjPos)) {
                    graph[cur][adjPos] = 0;
                    Q.push(adjPos);
                }

            }
        }
    }
/*    cerr << "graph size: " << graph.size() << endl;
    cerr << "nbBorderPositions: " << nbOfBorderPositions << endl;*/
    for(auto& node: graph) {
        for(auto &edge: node.second) {
            edge.second = (int) graph.size();
        }
    }
    int entryflow = ceil((graph.size()-nbOfBorderPositions) / (float) nbOfBorderPositions);
    if(entryflow == 0) {
        entryflow = 1;
    }

    for(auto &node: graph) {
        if(cellOnBorder(const_cast<Cell3DPosition &>(node.first))) {
            graph[source][node.first]  = entryflow;
        } else {
            graph[node.first][sink] = 1;
        }
    }
    maxFlow();
    entryFlow = graph[source][rbc->MMPosition];
    reduceGraph();
}

void GoalCover::printGraph() {
    cerr << "Graph: " ;
    for(const auto& node: graph) {
        cerr << endl << node.first << ": ";
        for (const auto& edge: node.second) {
            cerr << "(" << edge.first << ", " << edge.second << ") ; ";
        }
    }
}

bool GoalCover::bfs(map<Cell3DPosition, map<Cell3DPosition, int>> &G, const Cell3DPosition& s, const Cell3DPosition& t,
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

int GoalCover::maxFlow() {
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

    for(auto node: graph) {
        for(auto edge: node.second) {
            graph[node.first][edge.first] -= rGraph[node.first][edge.first];
            if (graph[node.first][edge.first] <= 0 or edge.first == sink) {
                graph[node.first].erase(edge.first);
            }
        }
    }
    return max;
}

void GoalCover::reduceGraph() {
    map<Cell3DPosition, Cell3DPosition> parent;
    //cerr << "reduce: " <<  bfs(graph, rbc->MMPosition, Cell3DPosition(0,0,0), parent);
    for(auto it=graph.begin(); it!=graph.end(); ) {
        if(it->first == rbc->MMPosition) {
            ++it;
            continue;
        }
        if(not bfs(graph, rbc->MMPosition, it->first, parent)) {
            rbc->console << "erase: " << it->first << "\n";
            graph.erase(it++);
        } else {
            ++it;
        }
    }
}





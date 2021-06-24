#ifndef MAXFLOW_HPP_
#define MAXFLOW_HPP_

#include "Dinic.hpp"

class MaxFlow {
   private:
    Catoms3DBlock* coordinator;

   public:
    map<Cell3DPosition, vector<Cell3DPosition>> graphMap;
    Graph graph;
    MaxFlow(/* args */);
    MaxFlow(Catoms3DBlock* _coordinator);
    ~MaxFlow();

    void initGraph();
    bool neighInMM(Cell3DPosition&);
    /**
     * Recursively fill the graph
     * @param pos: starting position
     * @param dst: target position
     **/
    void fillGraph(Cell3DPosition& pos, Cell3DPosition dst);
    void printGraph();

    bool BFS(map<Cell3DPosition, map<Cell3DPosition, int>>& rGraph, Cell3DPosition& s,
             Cell3DPosition& t, map<Cell3DPosition, Cell3DPosition>& parent);
    int fordFulkerson(Cell3DPosition& s, Cell3DPosition& t);
};

#endif
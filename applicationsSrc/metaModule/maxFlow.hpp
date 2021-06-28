#ifndef MAXFLOW_HPP_
#define MAXFLOW_HPP_

class MaxFlow {
   private:
    Catoms3DBlock* coordinator;
    
    void setSources();
   public:
    map<Cell3DPosition, vector<Cell3DPosition>> graphMap;
    vector<Cell3DPosition> sources; // all modules that can move in MM
    vector<Cell3DPosition> sinks = {Cell3DPosition(13,10,12), Cell3DPosition(13,9,13)};

    static  Cell3DPosition const VirtualSource, VirtualSink;

    MaxFlow(/* args */);
    MaxFlow(Catoms3DBlock* _coordinator);
    ~MaxFlow();

    void initGraph();
    bool neighInMM(Cell3DPosition&);
    void printGraph();

    bool BFS(map<Cell3DPosition, map<Cell3DPosition, int>>& rGraph, Cell3DPosition& s,
             Cell3DPosition& t, map<Cell3DPosition, Cell3DPosition>& parent);
    int fordFulkerson(Cell3DPosition& s, Cell3DPosition& t, deque<deque<Cell3DPosition>> &paths);
};

#endif
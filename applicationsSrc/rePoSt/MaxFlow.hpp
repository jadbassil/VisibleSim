//
// Created by jbassil on 14/10/22.
//

#ifndef VISIBLESIM_MAXFLOW_HPP
#define VISIBLESIM_MAXFLOW_HPP

#include "./rePoStBlockCode.hpp"
#include "./goalCover.hpp"
class RePoStBlockCode;
class GoalCover;
class MaxFlow {
private:
    RePoStBlockCode *rbc;
    static int nbDestination;
    const Cell3DPosition source = Cell3DPosition(-10,-10,-10);
    const Cell3DPosition sink = Cell3DPosition(-20,-20,-20);
    int nbMM;
    int totalNbDest();

    bool cellsAreAdjacent(Cell3DPosition p1, Cell3DPosition p2);
    bool bfs(map<Cell3DPosition,  map<Cell3DPosition, int>>& G, const Cell3DPosition& s, const Cell3DPosition& t,
             map<Cell3DPosition, Cell3DPosition>& parent);


public:
    GoalCover *goalCover;
    explicit MaxFlow(RePoStBlockCode *rbc);

    virtual ~MaxFlow();

    static map<Cell3DPosition, map<Cell3DPosition, int>> graph;


    void buildGraph();

    bool cellOnBorder(Cell3DPosition &cell);

    int maxFlow();

    void printGraph();

    map<Cell3DPosition, int> getLocalPaths();

    void printLocalConnections();
};


#endif //VISIBLESIM_MAXFLOW_HPP

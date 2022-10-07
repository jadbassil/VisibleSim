//
// Created by jbassil on 07/10/22.
//

#ifndef VISIBLESIM_GOALCOVER_HPP
#define VISIBLESIM_GOALCOVER_HPP

#include "./rePoStBlockCode.hpp"

class RePoStBlockCode;

class GoalCover {
private:
    RePoStBlockCode *rbc;
    map<Cell3DPosition, map<Cell3DPosition, int>> graph;
    const Cell3DPosition source = Cell3DPosition(-10,-10,-10);
    const Cell3DPosition sink = Cell3DPosition(-20,-20,-20);
    bool bfs(map<Cell3DPosition,  map<Cell3DPosition, int>>& G, const Cell3DPosition& s, const Cell3DPosition& t,
             map<Cell3DPosition, Cell3DPosition>& parent);

public:
    explicit GoalCover(RePoStBlockCode &_rbc);

    virtual ~GoalCover();

    bool cellOnBorder(Cell3DPosition &cell);
    void buildGraph();
    void printGraph();
    int maxFlow();

};


#endif //VISIBLESIM_GOALCOVER_HPP

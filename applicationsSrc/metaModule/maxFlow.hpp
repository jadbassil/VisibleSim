#ifndef MAXFLOW_HPP_
#define MAXFLOW_HPP_

#include "metaModuleBlockCode.hpp"
#include "Dinic.hpp"


class MaxFlow {
private:
    MetaModuleBlockCode *coordinator;
    bool neighInMM(Cell3DPosition &);

    
public:
    //map<Cell3DPosition, vector<Cell3DPosition>> graph;
    Graph graph;
    MaxFlow(/* args */);
    MaxFlow(MetaModuleBlockCode* _coordinator);
    ~MaxFlow();

    void initGraph();
    /**
     * Recursively fill the graph
     * @param pos: starting position
     * @param dst: target position
    **/
    void fillGraph(Cell3DPosition &pos, Cell3DPosition dst);
    void printGraph();
};

MaxFlow::MaxFlow(/* args */) {
}

MaxFlow::MaxFlow(MetaModuleBlockCode* _coordinator)
    : coordinator(_coordinator) {
}

MaxFlow::~MaxFlow() {
}

void MaxFlow::initGraph() {

    // assuming empty MM only
    vector<Cell3DPosition> *positions;
    positions = (coordinator->shapeState == FRONTBACK) ? &FrontBackMM: &BackFrontMM;
    cout << "unblocked pos: " << endl;
    for(auto pos: *positions) {
        MetaModuleBlockCode *block = static_cast<MetaModuleBlockCode*>(
                BaseSimulator::getWorld()->getBlockByPosition(coordinator->seedPosition + pos)->blockCode
            );
        vector<Cell3DPosition> reachablePositions = 
            Catoms3DMotionEngine::getAllReachablePositions(block->module);
        if(reachablePositions.size() > 0) {
            //graph[block->module->position] = reachablePositions;
            for(auto rp: reachablePositions)
                graph.addEdge(block->module->position, rp, 1);
            for(auto pos: reachablePositions) {
                fillGraph(pos, Cell3DPosition(13,10,12));
            }
        }
        
    }
}

void MaxFlow::fillGraph(Cell3DPosition &initPos, Cell3DPosition targetPos) {
    // if(graph.find(initPos) != graph.end()
    //     or initPos == targetPos or !neighInMM(initPos)) {
    //     return;
    // }
    if(graph.adj.find(initPos) != graph.adj.end()
        or initPos == targetPos or !neighInMM(initPos)) {
        return;
    }
    BaseSimulator::getWorld()->addBlock(0, MetaModuleBlockCode::buildNewBlockCode, initPos, Color(GREY));
    MetaModuleBlockCode *block = static_cast<MetaModuleBlockCode*>(
        BaseSimulator::getWorld()->getBlockByPosition(initPos)->blockCode
    );
    vector<Cell3DPosition> reachablePositions = 
            Catoms3DMotionEngine::getAllReachablePositions(block->module);
    Cell3DPosition blockPosition = block->module->position;
    BaseSimulator::getWorld()->lattice->highlightCell(blockPosition);
    BaseSimulator::getWorld()->deleteBlock(block->module);
    if(reachablePositions.size() > 0) {
        // graph[blockPosition] = reachablePositions;
        
        for(auto pos: reachablePositions) {
            graph.addEdge(block->module->position, pos, 1);
            fillGraph(pos, Cell3DPosition(13,10,12));
        }
    } 
}

bool MaxFlow::neighInMM(Cell3DPosition &pos) {
    vector<Cell3DPosition> activeCells = coordinator->lattice->getActiveNeighborCells(pos);
    for(auto p: activeCells) {
        if(coordinator->isInMM(p)) 
            return true;
    }
    return false;
}

void MaxFlow::printGraph() {
    // for(auto x: graph) {
    //     cerr << x.first << ": ";
    //     for(auto y: x.second) 
    //         cerr << y << "; ";
    //     cerr << endl;
    // }
    for(auto x: graph.adj) {
        cerr << x.first << ": ";
        for(auto y: x.second) 
            cerr << y.v << "; ";
        cerr << endl;
    }
    for(auto x: graph.rev) {
        cerr << x.first << ": ";
        for(auto y: x.second) 
            cerr << y.v << "; ";
        cerr << endl;
    }
}



#endif
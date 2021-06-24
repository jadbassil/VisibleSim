// C++ implementation of Dinic's Algorithm
#ifndef DINIC_HPP_
#define DINIC_HPP_

#include<bits/stdc++.h>
using namespace std;

#include "grid/cell3DPosition.h"

// A structure to represent a edge between
// two vertex
struct Edge
{
	Cell3DPosition v ; // Vertex v (or "to" vertex)
			// of a directed edge u-v. "From"
			// vertex u can be obtained using
			// index in adjacent array.

	int flow ; // flow of data in edge

	int C; // capacity

	Edge *rev ; // To store index of reverse
			// edge in adjacency list so that
			// we can quickly find it.

	Edge(Cell3DPosition _v, int _flow, int _C)
		:v(_v), flow(_flow), C(_C), rev(nullptr) {};

};

// Residual Graph
class Graph
{
	int V; // number of vertex
	map<Cell3DPosition, int> level; // stores level of a node
	//map<Cell3DPosition, vector<Edge>> *adj;
public :
    map<Cell3DPosition, vector<Edge>> adj;
	map<Cell3DPosition, vector<Edge>> rev;
	Graph()
	{
        //adj = new map<Cell3DPosition, vector<Edge>>();
		// adj = new vector<Edge>[V];
		// this->V = V;
        this->V = 0;
		//level = new map<Cell3DPosition, int>();
	}

	// add edge to the graph
	void addEdge(Cell3DPosition u, Cell3DPosition v, int C)
	{
		// Forward edge : 0 flow and C capacity
		Edge a(v, 0, C);

		// Back edge : 0 flow and 0 capacity
		Edge b(u, 0, 0);
        // a.rev = &b;
        // b.rev = &a;
		a.rev = &rev[v][rev[v].size()];
		b.rev = &adj[u][adj[u].size()];

        adj[u].push_back(a);
        rev[v].push_back(b);

		adj[u].rbegin()->rev = &rev[v][rev[v].size()];
		rev[v].rbegin()->rev = &adj[u][adj[u].size()];
		
	}

	bool BFS(Cell3DPosition s, Cell3DPosition t);
	int sendFlow(Cell3DPosition u, int flow, Cell3DPosition t, map<Cell3DPosition,int> start, vector<Cell3DPosition> &parents);
	int DinicMaxflow(Cell3DPosition s, Cell3DPosition t);
};


#endif
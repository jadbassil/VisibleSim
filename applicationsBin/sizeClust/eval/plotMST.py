import networkx as nx
import networkx.algorithms.community as community
import matplotlib.pyplot as plt
from random import random
import mst
import Graph

G1 = nx.DiGraph()

G2 = nx.Graph()

for row in Graph.edges:
    G2.add_edges_from(row)

# G1.add_edges_from(mst.edges)
for e in mst.edges:
    G1.add_edge(e[1], e[0])
    G1.add_node(e[1])
    G1.add_node(e[0])

print(G1.edges())

pos = nx.spring_layout(G2, iterations=1000) 

#nx.draw(G2,pos)
# plt.show()
nx.draw_networkx_nodes(G2, pos,
                     with_labels = True,
                    # nodelist=cluster,
                    # node_color=[random(), random(),random()],
                    node_size=500,
                    alpha=0.8)

nx.draw_networkx_edges(G2, pos, width=1.0)
plt.show()
# nx.draw(G1, pos, with_labels=True)

# pos = nx.spring_layout(G2, iterations=1000) 

color_map = []
size_map =[]
for node in G1:
    if node != 13:
        color_map.append('#1f78b4')
        size_map.append(500)
    else:
        color_map.append('red')
        size_map.append(900)

nx.draw_networkx_nodes(G1, pos,
                    with_labels = True,
                    # nodelist=cluster,
                    node_color=color_map,
                    node_size=size_map,
                    alpha=0.8)

nx.draw_networkx_edges(G1, pos, width=2.0, arrowsize=20)
plt.show()

import networkx as nx
import networkx.algorithms.community as community
import matplotlib.pyplot as plt
from random import random
import Graph

G = nx.Graph()

for row in Graph.edges:
    G.add_edges_from(row)

print('modularity: ', community.modularity(G, Graph.clusters))
print('performance: ', community.performance(G, Graph.clusters))
print('coverage: ', community.coverage(G, Graph.clusters))

# pos = nx.spring_layout(G, iterations=100) 
# for cluster in Graph.clusters:
#     nx.draw_networkx_nodes(G, pos,
#                        nodelist=cluster,
#                        node_color=[random(), random(),random()],
#                        node_size=500,
#                        alpha=0.8)

# nx.draw_networkx_edges(G, pos, width=1.0, alpha=0.5)
# plt.show()


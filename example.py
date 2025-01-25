import network_generation as ng
import networkx as nx

network = ng.generate_network(number_of_nodes=1000, probability_step_length_one=0.5, n_marked=5)

print('Average clustering coefficient:', nx.average_clustering(network))
print('Average shortest path length:', nx.average_shortest_path_length(network))


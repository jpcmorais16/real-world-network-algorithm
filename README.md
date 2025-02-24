# A simple and flexible algorithm to generate real-world networks

## This repository contains the implementation of an algorithm designed to generate undirected graphs with three key characteristics often observed in real-world networks, using random walks and iteratively adding edges based on a probability distribution:
### 1) Scale-freeness: Long-tailed degree distributions where a few nodes have significantly more connections than others.
### 2) Small-world phenomenon: Short average distances between nodes.
### 3) High clustering coefficients: Nodes form tightly knit communities.

## How to use
### Import the 'generate_network' function from the 'network_generation.py' file and call it with the desired parameters.

### Example:
```
import network_generation as ng
import networkx as nx

network = ng.generate_network(number_of_nodes=1000, probability_step_length_one=0.5, n_marked=5)

print('Average clustering coefficient:', nx.average_clustering(network))
print('Average shortest path length:', nx.average_shortest_path_length(network))
```
![example2](./images/example2.png)

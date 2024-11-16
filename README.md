# A simple and flexible algorithm to generate real-world networks

## This repository contains the implementation of an algorithm designed to generate undirected graphs with three key characteristics often observed in real-world networks:

### Scale-freeness: Long-tailed degree distributions where a few nodes have significantly more connections than others.
### Small-world phenomenon: Short average distances between nodes.
### High clustering coefficients: Nodes form tightly knit communities.

## The algorithm utilizes random walks across the network and iteratively adds edges based on a probability distribution. Edges are created with decreasing probability to link distant nodes, enabling the formation of a small-world structure.

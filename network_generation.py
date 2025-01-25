import networkx as nx
import random
from extra_edge import create_edge
from random_walk import random_walk


def generate_network(number_of_nodes: int, probability_step_length_one: float,
                          n_marked: float, graph=nx.cycle_graph(10)) -> nx.Graph:
    
    n_nodes = graph.number_of_nodes()

    for _ in range(0, number_of_nodes):
        
        start = random.randint(1, n_nodes-1)

        graph.add_node(n_nodes)

        marked = random_walk(graph, start, n_marked, probability_step_length_one)

        for v in marked:
            graph.add_edge(v, n_nodes)

        n_nodes += 1

        create_edge(graph, n_nodes)

    return graph
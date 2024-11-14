from collections import deque
import networkx as nx
import random
import numpy as np



def edges_in_degree_range(G, min_degree, max_degree):
    # Find nodes with degrees in the specified range
    nodes_in_range = [node for node, degree in G.degree() if min_degree <= degree <= max_degree]
    
    # Create a subgraph with only these nodes
    subgraph = G.subgraph(nodes_in_range)
    
    # Count the number of edges in this subgraph
    return subgraph.number_of_edges()

def assign_value(p):
    outcomes = [1, 2]
    probabilities = [p, 1 - p]

    x = random.choices(outcomes, probabilities)[0]
    return x




def random_steps(graph: nx.Graph, start: int, number_of_steps: int) -> int:

    current = start

    for _ in range(number_of_steps):
        neighbors = list(graph.neighbors(current))
        if len(neighbors) > 0:
            current = neighbors[random.randint(0, len(neighbors)-1)]

    return current
    


def calculate_distribution_value(graph):

    deg = 2 * graph.number_of_edges() / graph.number_of_nodes()

    diameter = int(2 * np.log(1 + graph.number_of_nodes()*(deg - 2))/np.log(deg))
 
    values = list(range(2, diameter + 1))
    
    probabilities = [1 / d**2 for d in values]

    normal_constant = 1/sum(probabilities)

    normalized_probabilities = [p / normal_constant for p in probabilities]

    return random.choices(values, weights=normalized_probabilities, k=1)[0]


def bfs(graph, n_nodes, source, distance):

    seen = [False for _ in range(n_nodes+1)]
    
    queue = deque([source])
    levels = {source: 0}
    current_level = 0

    target = source


    while current_level < distance:

        if len(queue) == 0:
            break

        node = queue.popleft()
        current_level = levels[node]

        for neighbor in graph.neighbors(node):

            if not seen[neighbor]:

                seen[neighbor] = True
                queue.append(neighbor)

                levels[neighbor] = current_level + 1

                if levels[neighbor] == distance:
                    target = neighbor
                    break

    return target

def random_walk_sequence_v3(number_of_nodes: int, probability_step_length_one: float,
                          n_marked: float, graph=nx.cycle_graph(10)) -> nx.Graph:
    
    n_nodes = graph.number_of_nodes()

    for _ in range(0, number_of_nodes):
        
        start = random.randint(1, n_nodes-1)
        current = start

        graph.add_node(n_nodes)
    
        marked = [start]

        for _ in range(int(n_marked)-1): 
            number_of_steps = assign_value(probability_step_length_one)
            current = random_steps(graph, current, number_of_steps)
        
            marked.append(current)

        for v in marked:
            graph.add_edge(v, n_nodes)

        n_nodes += 1

        distance = calculate_distribution_value(graph)
    
        source = random.randint(1, n_nodes-1)

        target = bfs(graph, n_nodes, source, distance)

        graph.add_edge(source, target)

    return graph
import numpy as np
import random
from collections import deque

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


def calculate_distribution_value(graph):

    deg = 2 * graph.number_of_edges() / graph.number_of_nodes()

    diameter = int(2 * np.log(1 + graph.number_of_nodes()*(deg - 2))/np.log(deg))
 
    values = list(range(2, diameter + 1))
    
    probabilities = [1 / d**2 for d in values]

    normal_constant = 1/sum(probabilities)

    normalized_probabilities = [p / normal_constant for p in probabilities]

    return random.choices(values, weights=normalized_probabilities, k=1)[0]


def create_edge(graph, n_nodes):
    distance = calculate_distribution_value(graph)
    
    source = random.randint(1, n_nodes-1)

    target = bfs(graph, n_nodes, source, distance)

    graph.add_edge(source, target)
import random
import networkx as nx

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


def random_walk(graph, start, n_marked, probability_step_length_one):
    marked = [start]
    current = start

    for _ in range(int(n_marked)-1): 
        number_of_steps = assign_value(probability_step_length_one)
        current = random_steps(graph, current, number_of_steps)
    
        marked.append(current)

    return marked
#!/usr/bin/env python3
"""
Basic Network Generation Example
=================================
Generates a small network and prints basic metrics.

Usage:
    python examples/basic_generation.py
"""

import sys
import os
import random
import time

# Add the python/ directory to the path so we can import network_generator
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'python'))

from network_generator import generate_network, calculate_alpha

def main():
    # ── Parameters ──────────────────────────────────────────────────────
    N = 1000          # Number of nodes to add (total nodes = N + 10)
    m = 4             # Average connections per new node
    p = 0.5           # Random walk step-length probability
    fp = 0.3          # Friendship probability (triadic closure)

    # ── Optional: fix seed for reproducibility ──────────────────────────
    random.seed(42)

    # ── Generate network ────────────────────────────────────────────────
    print(f"Generating network with N={N}, m={m}, p={p}, fp={fp} ...")
    start = time.time()
    graph = generate_network(N, m, p, fp)
    elapsed = time.time() - start

    # ── Compute basic statistics ────────────────────────────────────────
    num_nodes = len(graph)
    num_edges = sum(len(neighbors) for neighbors in graph) // 2
    avg_degree = 2 * num_edges / num_nodes
    max_degree = max(len(neighbors) for neighbors in graph)
    min_degree = min(len(neighbors) for neighbors in graph)

    # ── Clustering coefficient (local average) ──────────────────────────
    total_cc = 0.0
    for node in range(num_nodes):
        neighbors = list(graph[node])
        k = len(neighbors)
        if k < 2:
            continue
        # Count edges among neighbors
        triangles = 0
        neighbor_set = graph[node]
        for i in range(k):
            for j in range(i + 1, k):
                if neighbors[j] in graph[neighbors[i]]:
                    triangles += 1
        total_cc += 2 * triangles / (k * (k - 1))
    avg_clustering = total_cc / num_nodes

    # ── Power-law exponent ──────────────────────────────────────────────
    try:
        alpha = calculate_alpha(graph)
    except Exception:
        alpha = float('nan')

    # ── Print results ───────────────────────────────────────────────────
    print(f"\n{'='*50}")
    print(f"  Network Summary")
    print(f"{'='*50}")
    print(f"  Nodes              : {num_nodes}")
    print(f"  Edges              : {num_edges}")
    print(f"  Avg. degree        : {avg_degree:.2f}")
    print(f"  Min degree         : {min_degree}")
    print(f"  Max degree         : {max_degree}")
    print(f"  Avg. clustering    : {avg_clustering:.4f}")
    print(f"  Power-law alpha    : {alpha:.4f}")
    print(f"  Generation time    : {elapsed:.3f}s")
    print(f"{'='*50}")


if __name__ == '__main__':
    main()

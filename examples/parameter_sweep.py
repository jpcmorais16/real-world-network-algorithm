#!/usr/bin/env python3
"""
Parameter Sweep Example
========================
Sweeps over combinations of (N, m, p, fp), runs a configurable number of
simulations for each combination, and writes averaged results to a CSV file.

Usage:
    python examples/parameter_sweep.py

Output:
    examples/sweep_results.csv
"""

import sys
import os
import csv
import random
import time

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'python'))

from python.network_generator import generate_network, calculate_alpha

# ── Configuration ───────────────────────────────────────────────────────
N_VALUES = [500, 1000]
M_VALUES = [3, 5]
P_VALUES = [0.3, 0.7]
FP_VALUES = [0.1, 0.5]
N_SIMULATIONS = 3       # Repetitions per parameter combination
SEED = 42               # Set to None for non-deterministic runs
OUTPUT_FILE = os.path.join(os.path.dirname(__file__), 'sweep_results.csv')


def compute_metrics(graph):
    """Compute basic metrics for a graph (adjacency list of sets)."""
    num_nodes = len(graph)
    num_edges = sum(len(nbrs) for nbrs in graph) // 2
    max_degree = max(len(nbrs) for nbrs in graph)

    # Average local clustering coefficient
    total_cc = 0.0
    for node in range(num_nodes):
        neighbors = list(graph[node])
        k = len(neighbors)
        if k < 2:
            continue
        triangles = 0
        for i in range(k):
            for j in range(i + 1, k):
                if neighbors[j] in graph[neighbors[i]]:
                    triangles += 1
        total_cc += 2 * triangles / (k * (k - 1))
    avg_clustering = total_cc / num_nodes

    try:
        alpha = calculate_alpha(graph)
    except Exception:
        alpha = float('nan')

    return {
        'nodes': num_nodes,
        'edges': num_edges,
        'max_degree': max_degree,
        'avg_clustering': avg_clustering,
        'alpha': alpha,
    }


def main():
    if SEED is not None:
        random.seed(SEED)

    fieldnames = [
        'N', 'm', 'p', 'fp', 'simulations',
        'avg_nodes', 'avg_edges', 'avg_max_degree',
        'avg_clustering', 'std_clustering',
        'avg_alpha', 'std_alpha',
        'avg_time_s',
    ]

    combinations = [
        (N, m, p, fp)
        for N in N_VALUES
        for m in M_VALUES
        for p in P_VALUES
        for fp in FP_VALUES
    ]

    print(f"Parameter sweep: {len(combinations)} combinations × {N_SIMULATIONS} simulations")
    print(f"Output -> {OUTPUT_FILE}\n")

    with open(OUTPUT_FILE, 'w', newline='') as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()

        for idx, (N, m, p, fp) in enumerate(combinations, 1):
            print(f"[{idx}/{len(combinations)}] N={N}, m={m}, p={p}, fp={fp}")

            metrics_list = []
            times = []
            for sim in range(N_SIMULATIONS):
                t0 = time.time()
                graph = generate_network(N, m, p, fp)
                elapsed = time.time() - t0
                times.append(elapsed)
                metrics_list.append(compute_metrics(graph))

            # Average metrics
            import statistics
            avg_nodes = statistics.mean(r['nodes'] for r in metrics_list)
            avg_edges = statistics.mean(r['edges'] for r in metrics_list)
            avg_max_deg = statistics.mean(r['max_degree'] for r in metrics_list)
            clust_vals = [r['avg_clustering'] for r in metrics_list]
            alpha_vals = [r['alpha'] for r in metrics_list]

            row = {
                'N': N,
                'm': m,
                'p': p,
                'fp': fp,
                'simulations': N_SIMULATIONS,
                'avg_nodes': f"{avg_nodes:.0f}",
                'avg_edges': f"{avg_edges:.0f}",
                'avg_max_degree': f"{avg_max_deg:.1f}",
                'avg_clustering': f"{statistics.mean(clust_vals):.4f}",
                'std_clustering': f"{statistics.stdev(clust_vals):.4f}" if len(clust_vals) > 1 else "0.0000",
                'avg_alpha': f"{statistics.mean(alpha_vals):.4f}",
                'std_alpha': f"{statistics.stdev(alpha_vals):.4f}" if len(alpha_vals) > 1 else "0.0000",
                'avg_time_s': f"{statistics.mean(times):.3f}",
            }
            writer.writerow(row)
            print(f"  -> clustering={row['avg_clustering']}, alpha={row['avg_alpha']}, "
                  f"time={row['avg_time_s']}s")

    print(f"\nDone. Results saved to {OUTPUT_FILE}")


if __name__ == '__main__':
    main()

# Real-World Network Algorithm

A toolkit for generating and analyzing complex networks using a **random walk-based preferential attachment growth model**. The model starts from a ring topology and grows the network by adding nodes that connect to existing nodes discovered via random walks, producing networks with real-world properties such as power-law degree distributions, high clustering, and short average path lengths.

This project provides both a **C++ implementation** (optimized for large-scale simulations) and a **Python implementation** (for prototyping and quick experiments).

---

## Table of Contents

- [Overview](#overview)
- [Project Structure](#project-structure)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Usage](#usage)
  - [C++ Implementation](#c-implementation)
  - [Python Implementation](#python-implementation)
- [Algorithm Description](#algorithm-description)
- [Network Metrics](#network-metrics)
- [Examples](#examples)
- [Testing](#testing)
- [Reproducibility](#reproducibility)
- [Expected Output](#expected-output)
- [Troubleshooting](#troubleshooting)
- [License](#license)
- [References](#references)

---

## Overview

The network generation follows a random walk-based growth process:

1. **Initialization**: A ring of 10 nodes is created.
2. **Growth**: For each of *N* new nodes:
   - A random starting node is selected.
   - Random walks discover *m* nodes (on average) to connect to.
   - The new node is connected to all discovered (marked) nodes.
   - With probability *fp*, edges are also created between pairs of marked nodes (friendship/triadic closure).

This process produces networks exhibiting:

- **Scale-free degree distributions** (power-law tails)
- **High clustering coefficients** (triadic closure)
- **Small-world property** (short average path lengths)

---

## Project Structure

```
real-world-network-algorithm/
├── README.md                       # This file
├── Makefile                        # Build configuration for C++ project
├── main.cpp                        # Main C++ entry point (network generation + simulation)
├── networkMetrics.h                # C++ header: metric computation declarations
├── networkMetrics.cpp              # C++ implementation: metrics (path length, clustering, power law)
├── requirements.txt                # Python dependencies
├── python/
│   └── network_generator.py        # Python network generation and batch analysis
├── examples/
│   ├── basic_generation.py         # Generate a small network and print metrics
│   ├── parameter_sweep.py          # Sweep over parameter ranges, save results to CSV
│   ├── export_edge_list.py         # Generate a network and export it as an edge list
│   └── run_cpp_examples.sh         # Shell script demonstrating C++ CLI usage
├── tests/
│   ├── test_network_generator.py   # Python unit tests (pytest)
│   └── test_network.cpp            # C++ unit tests
└── v1/                             # Legacy v1 implementation (uses networkx)
    ├── example.py
    ├── network_generation.py
    ├── random_walk.py
    └── extra_edge.py
```

---

## Prerequisites

| Component | Requirement |
|-----------|-------------|
| **C++ compiler** | GCC or Clang with C++11 support |
| **Make** | GNU Make (for building C++) |
| **OS for C++** | Linux or macOS (uses `fork`/`wait`). On Windows, use **WSL**. |
| **Python** | 3.8 or later |
| **Python packages** | Listed in `requirements.txt` |

---

## Installation

### 1. Clone the repository

```bash
git clone https://github.com/<your-username>/real-world-network-algorithm.git
cd real-world-network-algorithm
```

### 2. Install Python dependencies

```bash
pip install -r requirements.txt
```

### 3. Build the C++ project (Linux / macOS / WSL)

```bash
make clean
make
```

This produces the `network_metrics` executable in the project root.

---

## Quick Start

### Python — generate a small network in 2 lines

```bash
cd examples
python basic_generation.py
```

Or interactively:

```python
import sys, os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'python'))
from network_generator import generate_network

graph = generate_network(N=1000, m=4, p=0.5, fp=0.3)
print(f"Nodes: {len(graph)}")
print(f"Edges: {sum(len(s) for s in graph) // 2}")
```

### C++ — run a simulation from the command line

```bash
# Build
make

# Run a simulation: 10 000 nodes, m=4, p=0.5, fp=0.3, 5 repetitions
./network_metrics -N 10000 -m 4 -p 0.5 -fp 0.3

# Show all options
./network_metrics --help
```

---

## Usage

### C++ Implementation

#### Command-Line Interface

```
./network_metrics -N <nodes> -m <connections> -p <probability> -fp <friendship_prob> [OPTIONS]
```

**Required parameters:**

| Flag | Description | Example |
|------|-------------|---------|
| `-N` | Number of nodes to add to the initial 10-node ring | `10000` |
| `-m` | Average number of connections per new node | `4`, `6` |
| `-p` | Probability parameter controlling random walk step length (0–1) | `0.5` |
| `-fp` | Friendship probability between marked nodes (0–1) | `0.3` |

**Optional parameters:**

| Flag | Description | Default |
|------|-------------|---------|
| `-n` | Number of simulation repetitions | `5` |
| `-o` | Output file path | `output_<N>.txt` |
| `-id` | Execution ID for checkpoint/resume | *(disabled)* |
| `-h` | Show help | — |

#### Checkpoint / Resume

For long-running simulations, pass `-id <name>` to enable checkpointing. If interrupted, re-run the same command to resume:

```bash
./network_metrics -N 200000 -m 6 -p 0.7 -fp 0.5 -id experiment_001
# ...interrupted...
./network_metrics -N 200000 -m 6 -p 0.7 -fp 0.5 -id experiment_001   # resumes
```

Checkpoint files created: `checkpoint_<id>_graph.txt`, `checkpoint_<id>_results.txt`, `checkpoint_<id>_completed.txt`.

#### Output format

Each line in the output file is tab-separated:

```
N  p  m  fp  avg_clustering // std  avg_transitivity // std  avg_path_length // std  pearson_r // std  slope // std  slope_cumulative // std  r_cumulative // std  max_degree // std  construction_time // std  path_calc_time // std  total_time // std
```

### Python Implementation

The Python module in `python/network_generator.py` provides the core `generate_network()` function and several analysis utilities. See the [Examples](#examples) section below for usage patterns.

#### Key functions

| Function | Description |
|----------|-------------|
| `generate_network(N, m, p, fp)` | Returns an adjacency-list graph (`list[set]`) |
| `calculate_alpha(graph)` | Estimates the power-law exponent α |
| `graph_to_igraph(graph)` | Converts to an `igraph.Graph` for advanced analysis |
| `average_shortest_path_length(graph)` | BFS-based average shortest path (pure Python) |

---

## Algorithm Description

### Parameters

| Symbol | Parameter | Role |
|--------|-----------|------|
| *N* | Network size | Number of new nodes added after the initial ring |
| *m* | Attachment count | Average number of existing nodes each new node connects to |
| *p* | Walk probability | Controls the geometric-like step-length distribution of the random walk |
| *fp* | Friendship probability | Probability of creating an edge between each pair of marked nodes (triadic closure) |

### Random Walk Step-Length Distribution

The number of steps *x* in each random walk is drawn from a truncated geometric distribution (x = 1, …, 10):

$$P(x) = \frac{p \cdot (1-p)^{x-1}}{1 - (1-p)^{10}}$$

- When *p* is close to 1, walks are short (mostly 1 step) → connections stay local.
- When *p* is close to 0, walks are longer → connections are more global.

### Growth Process (pseudocode)

```
initialize ring graph G with 10 nodes
for i = 1 to N:
    start ← random node in G
    marked ← {start}
    current ← start
    for j = 1 to floor(m) - 1:
        steps ← sample from P(x)
        current ← random_walk(G, current, steps)
        marked ← marked ∪ {current}
    add node v_new to G
    for each u in marked:
        add edge (v_new, u)
    for each pair (u, w) in marked:
        with probability fp:
            add edge (u, w)
```

## Network Metrics

| Metric | Description |
|--------|-------------|
| **Average path length** | Mean shortest-path distance over all reachable pairs (BFS-based, parallelized in C++) |
| **Clustering coefficient** | Average local clustering coefficient across all nodes |
| **Power-law exponent (α)** | Slope of log-log regression on the degree distribution |
| **Cumulative power-law exponent** | Slope from the complementary cumulative degree distribution |
| **Pearson r** | Correlation coefficient of the log-log fit |
| **Max degree** | Largest node degree in the network |

---

## Examples

Ready-to-run example scripts are provided in the `examples/` directory.

### 1. Basic network generation (Python)

```bash
python examples/basic_generation.py
```

Generates a small network (N=1000) and prints node count, edge count, clustering coefficient, and power-law exponent.

### 2. Parameter sweep (Python)

```bash
python examples/parameter_sweep.py
```

Sweeps over combinations of *N*, *m*, *p*, *fp* and writes results to `examples/sweep_results.csv`.

### 3. Export edge list (Python)

```bash
python examples/export_edge_list.py
```

Generates a network and saves it as a standard edge-list file that can be loaded by NetworkX, igraph, Gephi, etc.

### 4. C++ examples (shell script)

```bash
# Make executable and run
chmod +x examples/run_cpp_examples.sh
./examples/run_cpp_examples.sh
```

Demonstrates building and running the C++ tool with various parameter combinations.

---

## Testing

### Python tests

The test suite uses **pytest** and covers the core generation logic, metric computations, and edge cases.

```bash
# Install test dependencies
pip install -r requirements.txt

# Run all tests
pytest tests/ -v

# Run with coverage
pytest tests/ -v --cov=python --cov-report=term-missing
```

### C++ tests

A standalone test file is provided that compiles and runs without external frameworks:

```bash
# Build and run tests (Linux / macOS / WSL)
cd tests
g++ -std=c++11 -O2 -o test_network test_network.cpp -I..
./test_network
```

Or use the Makefile target:

```bash
make test
```

---

## Reproducibility

To ensure reproducible results across runs:

### Seeding the random number generator

**Python:** Pass a fixed seed before calling `generate_network`:

```python
import random
random.seed(42)
graph = generate_network(N=1000, m=4, p=0.5, fp=0.3)
```

**C++:** The C++ implementation currently uses `std::random_device` for seeding by default (non-deterministic). To make runs deterministic, you can modify the global generator in `main.cpp`:

```cpp
// Change:  std::mt19937 gen(rd());
// To:      std::mt19937 gen(42);  // fixed seed for reproducibility
```

### Environment

- Pin your Python dependencies using `requirements.txt` (provided).
- Record your compiler version: `g++ --version`.
- Record your OS and architecture.

### Recommended workflow

1. Fix the random seed (see above).
2. Run the simulation: `./network_metrics -N 10000 -m 4 -p 0.5 -fp 0.3 -n 5 -o results.txt`
3. Commit the output file alongside your code so reviewers can verify.

---


## License

This project is provided for research and educational purposes.

---

## References

- Network generation based on random walk growth models
- Power law analysis using log-log regression
- Small-world network properties analysis
- Clustering coefficient and transitivity metrics

---

> **Note:** The C++ implementation uses Unix-specific features (`fork`, `wait`). For Windows, use the Python implementation or WSL.

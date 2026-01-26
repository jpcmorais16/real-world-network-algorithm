# Large Network Generator

A comprehensive toolkit for generating, analyzing, and studying complex network structures using a random walk-based growth model. This project enables researchers to study network properties, power law distributions, clustering behavior, and path length characteristics through both C++ and Python implementations.

## 🔍 Overview

This project provides a complete pipeline for network generation and analysis:

1. **Network Generation**: Create networks using a random walk-based growth algorithm
2. **Network Analysis**: Compute metrics, such as power law distributions and structural properties

### Key Features

- 🌐 **Random Walk Network Generation**: Growth model starting from a ring topology with configurable parameters
- 📊 **Comprehensive Metrics**: Average path length, clustering coefficient, power law analysis
- ⚡ **High Performance**: C++ implementation with multi-process parallelization for path calculations
- 💾 **Checkpoint/Resume**: Save and resume long-running simulations
- 🔄 **Dual Implementation**: Both C++ (high performance) and Python (prototyping) versions
- 📈 **Statistical Analysis**: Power law fitting with Pearson correlation coefficients

## 📁 Project Structure

```
large-network-generator/
├── README.md                          # This file
├── main.cpp                           # Main C++ program with network generation and simulation
├── networkMetrics.h                   # C++ header for network analysis functions
├── networkMetrics.cpp                 # C++ implementation of network metrics
├── Makefile                           # Build configuration for C++ project
└── python/                            # Python implementation
    └── network_generator.py           # Python network generation and analysis
```

## 🔄 Complete Workflow

### Phase 1: Network Generation
Generate networks using random walk-based growth algorithm with configurable parameters (N, m, p, fp)

### Phase 2: Network Analysis
Calculate network metrics including path length, clustering, and power law distributions

### Phase 3: Statistical Analysis
Run multiple simulations and compute averages with standard deviations

## 🚀 Installation

### Prerequisites

- **C++ Compiler** (GCC/Clang with C++11 support)
- **Python 3.8+** (for Python implementation)
- **Make** (for building C++ project)

### 1. Build C++ Project

```bash
# Build the project
make

# Or build and run
make run
```

### 2. Install Python Dependencies

```bash
pip install numpy powerlaw igraph
```

## 🎯 Quick Start Guide

### C++ Implementation

```bash
# Build the project
make

# Run a simulation with specific parameters
./network_metrics -N 10000 -m 4 -p 0.5 -fp 0.3

# Run with all options (10 simulations, custom output file, checkpointing)
./network_metrics -N 200000 -m 6 -p 0.7 -fp 0.5 -n 10 -o results.txt -id my_simulation

# Show help message
./network_metrics --help
```

### Python Implementation

```bash
cd python
python network_generator.py
```

## 📖 Detailed Usage

### Network Generation Algorithm

The network generation follows this process:

1. **Initial Ring**: Start with 10 nodes connected in a ring topology
2. **Node Addition**: For each of N new nodes:
   - Select a random starting node
   - Perform random walks to mark `m` nodes (on average)
   - Connect the new node to all marked nodes
   - Create edges between marked nodes with probability `fp`

#### Parameters Explained

- **N**: Number of new nodes to add to the initial 10-node ring
- **m**: Average number of nodes to mark via random walks (can be fractional)
- **p**: Probability parameter controlling random walk step length distribution
- **fp**: Friendship probability - probability of creating edges between marked nodes

### C++ Implementation

#### Command-Line Usage

The C++ program accepts command-line arguments for flexible parameter configuration:

**Required Parameters:**
- `-N <number>`: Number of nodes to add to the network (e.g., 10000, 200000)
- `-m <number>`: Average number of connections per new node (e.g., 2, 4, 5, 6, 8, 10)
- `-p <probability>`: Probability parameter for random walk steps (0.0 to 1.0)
- `-fp <probability>`: Friendship probability between marked nodes (0.0 to 1.0)

**Optional Parameters:**
- `-n <number>`: Number of simulations to run (default: 5)
- `-o <filename>`: Output file path (default: `distances_<N>.txt`)
- `-id <execution_id>`: Execution ID for checkpoint/resume functionality
- `-h, --help`: Show help message with usage examples

**Examples:**

```bash
# Basic usage - run 5 simulations with default output file
./network_metrics -N 10000 -m 4 -p 0.5 -fp 0.3

# Run 10 simulations with custom output file
./network_metrics -N 200000 -m 6 -p 0.7 -fp 0.5 -n 10 -o my_results.txt

# Enable checkpointing with execution ID
./network_metrics -N 50000 -m 5 -p 0.3 -fp 0.7 -id experiment_001

# Show help message
./network_metrics --help
```

#### Checkpoint/Resume Functionality

The C++ implementation supports checkpointing for long-running simulations:

```bash
# First run - creates checkpoint files
./network_metrics -N 200000 -m 6 -p 0.7 -fp 0.5 -id my_experiment

# If interrupted, rerun with same parameters and ID to resume
./network_metrics -N 200000 -m 6 -p 0.7 -fp 0.5 -id my_experiment
```

Checkpoint files created:
- `checkpoint_<executionId>_graph.txt` - Network structure
- `checkpoint_<executionId>_results.txt` - Completed node results
- `checkpoint_<executionId>_completed.txt` - List of completed nodes


#### Output Format

Each line in the output file contains:
```
N    p    m    fp    avg_clust // std_clust    avg_trans // std_trans    avg_path // std_path    r // std_r    slope // std_slope    slope_cumulative // std_slope_cumulative    r_cumulative // std_r_cumulative    max_degree // std_max_degree    construction_time // std_construction_time    path_calc_time // std_path_calc_time    total_time // std_total_time
```

### Python Implementation

#### Basic Usage

Edit `python/network_generator.py` to modify parameter ranges:

```python
for N in [10687]:
    for m in [3]:
        for p in [0.47]:
            for fp in [0.1]:
                # Run simulations
                # Results written to test.txt
```

#### Output Format

Results are written to `python/result.txt` in CSV format:
```
N, m, p, fp, avg_alpha, avg_path_length, avg_clustering, avg_edges, avg_max_degree
```

## 🔬 Analysis Capabilities

### Network Metrics

#### 1. Average Path Length
- **Definition**: Average shortest path length between all pairs of connected nodes
- **Calculation**: BFS-based shortest path algorithm
- **Parallelization**: Uses child processes for large networks
- **Checkpointing**: Supports resuming interrupted calculations

#### 2. Clustering Coefficient
- **Definition**: Average local clustering coefficient across all nodes
- **Formula**: For each node, ratio of actual triangles to possible triangles
- **Range**: 0 (no clustering) to 1 (fully clustered)

#### 3. Power Law Analysis
- **Degree Distribution**: Fits power law to node degree distribution
- **Cumulative Distribution**: Fits power law to complementary cumulative degree distribution
- **Metrics**: 
  - Power law coefficient (α)
  - Pearson correlation coefficient (r)
  - Maximum degree

#### 4. Additional Metrics
- **Transitivity**: Global clustering coefficient
- **Maximum Degree**: Highest node degree in the network
- **Number of Edges**: Total edge count

### Performance Features

#### Parallel Processing
- **Path Length Calculation**: Uses 20 forked processes for BFS calculations
- **Threading**: Utilizes hardware concurrency for clustering calculations
- **Process Management**: Automatic cleanup and result aggregation

#### Checkpoint/Resume
- **Graph Persistence**: Saves network structure for resuming
- **Incremental Results**: Saves individual node results as they complete
- **Smart Resumption**: Automatically skips already-computed nodes

## 🔧 Customization

### Running Parameter Sweeps

#### C++ (Using Shell Scripts)

To run multiple parameter combinations, create a shell script:

```bash
#!/bin/bash
# Example: run_parameter_sweep.sh

for N in 10000 50000 100000; do
    for m in 2 3 4 5; do
        for p in 0.1 0.3 0.5; do
            for fp in 0.1 0.3 0.5; do
                ./network_metrics -N $N -m $m -p $p -fp $fp -n 5 -o results.txt
            done
        done
    done
done
```

Or use the program's built-in resume functionality to skip already-completed combinations by checking the output file.

#### Python (network_generator.py)
Edit the parameter loops:

```python
for N in [10000, 50000, 100000]:
    for m in [2, 3, 4, 5]:
        for p in [0.1, 0.3, 0.5]:
            for fp in [0.1, 0.3, 0.5]:
                # ...
```

### Adjusting Number of Simulations

#### C++ (Command-Line)
Use the `-n` parameter:

```bash
# Run 10 simulations
./network_metrics -N 10000 -m 4 -p 0.5 -fp 0.3 -n 10

# Run 20 simulations
./network_metrics -N 10000 -m 4 -p 0.5 -fp 0.3 -n 20
```

#### Python (network_generator.py)
Change the range in the simulation loop:

```python
for i in range(10):  # 10 simulations
    # ...
```

### Changing Output File Names

#### C++ (Command-Line)
Use the `-o` parameter:

```bash
# Specify custom output file
./network_metrics -N 10000 -m 4 -p 0.5 -fp 0.3 -o my_results.txt

# Default output file is distances_<N>.txt (e.g., distances_10000.txt)
```

#### Python (network_generator.py)
```python
with open("my_results.txt", "a") as f:  # Change output filename
    f.write(result_line)
```

## 📝 Algorithm Details

### Random Walk Step Distribution

The number of steps in a random walk follows a geometric-like distribution:

```
P(x) = (p * (1-p)^(x-1)) / (1 - (1-p)^10)
```

Where:
- `x` ranges from 1 to 10 steps
- `p` is the probability parameter
- The denominator normalizes the distribution

### Network Growth Process

1. **Initialization**: Create ring of 10 nodes
2. **For each new node**:
   - Start at random node
   - Perform random walks to mark `m` nodes (on average)
   - Connect new node to all marked nodes
   - With probability `fp`, connect pairs of marked nodes

### Complexity

- **Time Complexity**: O(N * m * E) where E is average edges per node
- **Space Complexity**: O(N + E) for adjacency list representation
- **Path Calculation**: O(N * (N + E)) for full BFS from each node

## 📄 License

This project is provided for research and educational purposes.

## 🔗 References

- Network generation based on random walk growth models
- Power law analysis using log-log regression
- Small-world network properties analysis
- Clustering coefficient and transitivity metrics

---

**Note**: The C++ implementation is optimized for performance and uses Unix-specific features (fork, wait). For Windows compatibility, use the Python implementation or WSL.

#!/usr/bin/env python3
"""
Unit tests for the Python network generator.

Run with:
    pytest tests/test_network_generator.py -v
"""

import sys
import os
import random
import math

import pytest

# Ensure the python/ package is importable
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from python.network_generator import (
    generate_network,
    event_occurs,
    get_random_integer,
    P as step_probability,
    get_number_of_steps,
    random_walk,
    calculate_alpha,
    average_shortest_path_length,
    graph_to_igraph,
)


# ────────────────────────────────────────────────────────────────────────────
# Fixtures
# ────────────────────────────────────────────────────────────────────────────

@pytest.fixture(autouse=True)
def seed_rng():
    """Fix the random seed before every test for determinism."""
    random.seed(12345)
    yield


@pytest.fixture
def small_graph():
    """A deterministic small graph (N=200, m=3, p=0.5, fp=0.3)."""
    random.seed(42)
    return generate_network(N=200, m=3, p=0.5, fp=0.3)


@pytest.fixture
def tiny_graph():
    """A very small graph (N=50) for fast tests."""
    random.seed(99)
    return generate_network(N=50, m=2, p=0.5, fp=0.2)


# ────────────────────────────────────────────────────────────────────────────
# Tests for helper functions
# ────────────────────────────────────────────────────────────────────────────

class TestEventOccurs:
    """Tests for the event_occurs() helper."""

    def test_always_true(self):
        """Probability 1.0 should always return True."""
        assert all(event_occurs(1.0) for _ in range(100))

    def test_never_true(self):
        """Probability 0.0 should always return False."""
        assert not any(event_occurs(0.0) for _ in range(100))

    def test_approximate_frequency(self):
        """With p=0.5, roughly half the trials should be True."""
        random.seed(0)
        results = [event_occurs(0.5) for _ in range(10000)]
        proportion = sum(results) / len(results)
        assert 0.45 < proportion < 0.55


class TestGetRandomInteger:
    """Tests for get_random_integer()."""

    def test_range_inclusive(self):
        """Returned value should be within [min, max]."""
        for _ in range(200):
            val = get_random_integer(3, 7)
            assert 3 <= val <= 7

    def test_single_value(self):
        """If min == max, the only possible result is that value."""
        assert get_random_integer(5, 5) == 5

    def test_all_values_reachable(self):
        """Over many trials every value in [0, 4] should appear."""
        random.seed(0)
        seen = set()
        for _ in range(500):
            seen.add(get_random_integer(0, 4))
        assert seen == {0, 1, 2, 3, 4}


class TestStepProbability:
    """Tests for the P(p, x) step-length distribution function."""

    def test_sums_to_one(self):
        """For any valid p, P(p, 1) + ... + P(p, 10) ≈ 1.0."""
        for p in [0.1, 0.3, 0.5, 0.7, 0.9]:
            total = sum(step_probability(p, x) for x in range(1, 11))
            assert abs(total - 1.0) < 1e-10, f"Sum for p={p} was {total}"

    def test_p_zero_returns_constant(self):
        """When p=0.0, the function should return 0.1 for all x."""
        for x in range(1, 11):
            assert step_probability(0.0, x) == 0.1

    def test_monotone_decreasing(self):
        """For p > 0, P(p, x) should decrease with x."""
        for p in [0.3, 0.5, 0.8]:
            values = [step_probability(p, x) for x in range(1, 11)]
            for i in range(len(values) - 1):
                assert values[i] >= values[i + 1]


class TestGetNumberOfSteps:
    """Tests for get_number_of_steps()."""

    def test_returns_valid_range(self):
        """Should always return an integer in [1, 10]."""
        for _ in range(500):
            steps = get_number_of_steps(0.5)
            assert 1 <= steps <= 10

    def test_high_p_favors_short_walks(self):
        """With p close to 1, most walks should be 1 step."""
        random.seed(0)
        steps = [get_number_of_steps(0.95) for _ in range(1000)]
        proportion_one = steps.count(1) / len(steps)
        assert proportion_one > 0.8


class TestRandomWalk:
    """Tests for random_walk()."""

    def test_zero_steps_returns_start(self):
        """A walk with 0 steps should stay at the start node."""
        graph = [set() for _ in range(5)]
        # Simple chain: 0—1—2—3—4
        for i in range(4):
            graph[i].add(i + 1)
            graph[i + 1].add(i)
        assert random_walk(graph, 2, 0) == 2

    def test_returns_valid_node(self):
        """Result must be a valid node index in the graph."""
        random.seed(0)
        graph = generate_network(N=100, m=2, p=0.5, fp=0.1)
        for _ in range(50):
            node = random_walk(graph, 0, 5)
            assert 0 <= node < len(graph)

    def test_isolated_node_stays(self):
        """If a node has no neighbors, the walk stays there."""
        graph = [set(), set(), set()]  # 3 isolated nodes
        assert random_walk(graph, 1, 10) == 1


# ────────────────────────────────────────────────────────────────────────────
# Tests for network generation
# ────────────────────────────────────────────────────────────────────────────

class TestGenerateNetwork:
    """Tests for generate_network()."""

    def test_correct_node_count(self):
        """Graph should have exactly N + 10 nodes."""
        for N in [0, 10, 100, 500]:
            random.seed(0)
            graph = generate_network(N=N, m=2, p=0.5, fp=0.3)
            assert len(graph) == N + 10

    def test_initial_ring_structure(self):
        """With N=0, the graph is just the initial 10-node ring."""
        random.seed(0)
        graph = generate_network(N=0, m=2, p=0.5, fp=0.3)
        assert len(graph) == 10
        # Each node in the ring should have at least its two ring neighbors
        for i in range(10):
            left = (i - 1) % 10
            right = (i + 1) % 10
            assert left in graph[i], f"Node {i} missing left neighbor {left}"
            assert right in graph[i], f"Node {i} missing right neighbor {right}"

    def test_edges_are_symmetric(self):
        """If u→v exists then v→u must exist (undirected graph)."""
        random.seed(0)
        graph = generate_network(N=300, m=4, p=0.5, fp=0.3)
        for u in range(len(graph)):
            for v in graph[u]:
                assert u in graph[v], f"Edge ({u},{v}) is not symmetric"

    def test_no_self_loops(self):
        """No node should have itself in its neighbor set."""
        random.seed(0)
        graph = generate_network(N=300, m=4, p=0.5, fp=0.3)
        for u in range(len(graph)):
            assert u not in graph[u], f"Self-loop found at node {u}"

    def test_every_new_node_has_edges(self):
        """Each new node (index ≥ 10) must have at least 1 edge."""
        random.seed(0)
        graph = generate_network(N=200, m=2, p=0.5, fp=0.3)
        for u in range(10, len(graph)):
            assert len(graph[u]) >= 1, f"New node {u} has no edges"

    def test_edge_count_grows_with_m(self):
        """Higher m should produce more edges on average."""
        random.seed(42)
        g_low = generate_network(N=500, m=2, p=0.5, fp=0.0)
        random.seed(42)
        g_high = generate_network(N=500, m=6, p=0.5, fp=0.0)
        edges_low = sum(len(s) for s in g_low) // 2
        edges_high = sum(len(s) for s in g_high) // 2
        assert edges_high > edges_low

    def test_friendship_probability_increases_edges(self):
        """Higher fp should produce more edges."""
        random.seed(42)
        g_no_fp = generate_network(N=500, m=4, p=0.5, fp=0.0)
        random.seed(42)
        g_high_fp = generate_network(N=500, m=4, p=0.5, fp=1.0)
        edges_no = sum(len(s) for s in g_no_fp) // 2
        edges_high = sum(len(s) for s in g_high_fp) // 2
        assert edges_high > edges_no

    def test_invalid_p_raises(self):
        """p outside [0, 1] should raise ValueError."""
        with pytest.raises(ValueError):
            generate_network(N=10, m=2, p=-0.1, fp=0.3)
        with pytest.raises(ValueError):
            generate_network(N=10, m=2, p=1.5, fp=0.3)

    def test_invalid_fp_raises(self):
        """fp outside [0, 1] should raise ValueError."""
        with pytest.raises(ValueError):
            generate_network(N=10, m=2, p=0.5, fp=-0.1)
        with pytest.raises(ValueError):
            generate_network(N=10, m=2, p=0.5, fp=1.5)

    def test_invalid_N_raises(self):
        """Negative N should raise ValueError."""
        with pytest.raises(ValueError):
            generate_network(N=-1, m=2, p=0.5, fp=0.3)

    def test_reproducibility_with_same_seed(self):
        """Two runs with the same seed must produce identical graphs."""
        random.seed(777)
        g1 = generate_network(N=200, m=3, p=0.5, fp=0.3)
        random.seed(777)
        g2 = generate_network(N=200, m=3, p=0.5, fp=0.3)
        assert len(g1) == len(g2)
        for i in range(len(g1)):
            assert g1[i] == g2[i], f"Mismatch at node {i}"


# ────────────────────────────────────────────────────────────────────────────
# Tests for network metrics
# ────────────────────────────────────────────────────────────────────────────

class TestAverageShortestPathLength:
    """Tests for average_shortest_path_length()."""

    def test_single_node(self):
        """A single node has path length 0."""
        graph = [set()]
        assert average_shortest_path_length(graph) == 0.0

    def test_two_connected_nodes(self):
        """Two connected nodes have avg path length 1.0."""
        graph = [{1}, {0}]
        assert average_shortest_path_length(graph) == 1.0

    def test_linear_chain(self):
        """A chain 0-1-2-3 should have avg path length 5/3."""
        graph = [{1}, {0, 2}, {1, 3}, {2}]
        # Pairs: (0,1)=1, (0,2)=2, (0,3)=3, (1,2)=1, (1,3)=2, (2,3)=1
        # Sum = 1+2+3+1+2+1 = 10, counted twice (both directions) = 20
        # Avg per node = 20/4 = 5, then /4 again...
        # Actually: average over all nodes of avg-dist-from-that-node
        # Node 0: (1+2+3)/3 = 2.0
        # Node 1: (1+1+2)/3 = 4/3
        # Node 2: (2+1+1)/3 = 4/3
        # Node 3: (3+2+1)/3 = 2.0
        # Total = 2+4/3+4/3+2 = 20/3, divided by 4 = 5/3
        expected = (2.0 + 4/3 + 4/3 + 2.0) / 4
        result = average_shortest_path_length(graph)
        assert abs(result - expected) < 1e-10

    def test_complete_graph(self):
        """A complete graph on 4 nodes has avg path length 1.0."""
        graph = [{1, 2, 3}, {0, 2, 3}, {0, 1, 3}, {0, 1, 2}]
        assert abs(average_shortest_path_length(graph) - 1.0) < 1e-10

    def test_disconnected_components(self):
        """Disconnected graph: only reachable pairs count."""
        graph = [{1}, {0}, {3}, {2}]  # Two components: {0,1} and {2,3}
        # Each node can only reach 1 other node at distance 1
        # Node avg = 1.0 for each, total = 4.0/4 = 1.0
        result = average_shortest_path_length(graph)
        assert abs(result - 1.0) < 1e-10

    def test_positive_for_generated_network(self, small_graph):
        """Average path length on a real generated network should be positive."""
        apl = average_shortest_path_length(small_graph)
        assert apl > 0


class TestCalculateAlpha:
    """Tests for calculate_alpha() (power-law exponent)."""

    def test_returns_positive(self, small_graph):
        """Alpha should be a positive number for a non-trivial network."""
        alpha = calculate_alpha(small_graph)
        assert alpha > 0

    def test_typical_range(self, small_graph):
        """For this growth model, α is typically between 1.5 and 5.0."""
        alpha = calculate_alpha(small_graph)
        assert 1.0 < alpha < 6.0, f"alpha={alpha} outside expected range"


class TestGraphToIgraph:
    """Tests for graph_to_igraph() conversion."""

    def test_node_count_preserved(self, tiny_graph):
        """igraph conversion should preserve node count."""
        ig = graph_to_igraph(tiny_graph)
        assert ig.vcount() == len(tiny_graph)

    def test_edge_count_preserved(self, tiny_graph):
        """igraph conversion should preserve edge count."""
        ig = graph_to_igraph(tiny_graph)
        expected_edges = sum(len(s) for s in tiny_graph) // 2
        assert ig.ecount() == expected_edges

    def test_undirected(self, tiny_graph):
        """Converted graph should be undirected."""
        ig = graph_to_igraph(tiny_graph)
        assert not ig.is_directed()


# ────────────────────────────────────────────────────────────────────────────
# Tests for structural properties (statistical)
# ────────────────────────────────────────────────────────────────────────────

class TestNetworkProperties:
    """Statistical tests on generated network properties."""

    def test_clustering_positive(self, small_graph):
        """A network with fp > 0 should have positive clustering."""
        ig = graph_to_igraph(small_graph)
        cc = ig.transitivity_avglocal_undirected()
        assert cc > 0

    def test_connected(self, small_graph):
        """The generated network should typically be connected."""
        ig = graph_to_igraph(small_graph)
        assert ig.is_connected()

    def test_degree_distribution_has_variance(self, small_graph):
        """Degree distribution should not be uniform (characteristic of scale-free)."""
        degrees = [len(s) for s in small_graph]
        mean_deg = sum(degrees) / len(degrees)
        variance = sum((d - mean_deg) ** 2 for d in degrees) / len(degrees)
        assert variance > 1.0, "Degree distribution has unexpectedly low variance"

    def test_small_world_path_length(self):
        """Avg path length should grow slowly (sub-linearly) with N."""
        random.seed(42)
        g1 = generate_network(N=200, m=3, p=0.5, fp=0.3)
        ig1 = graph_to_igraph(g1)
        apl1 = ig1.average_path_length()

        random.seed(42)
        g2 = generate_network(N=2000, m=3, p=0.5, fp=0.3)
        ig2 = graph_to_igraph(g2)
        apl2 = ig2.average_path_length()

        # 10× more nodes should not produce 10× longer paths
        assert apl2 < 3 * apl1, (
            f"Path length scales too fast: {apl1:.2f} → {apl2:.2f} for 10× nodes"
        )


# ────────────────────────────────────────────────────────────────────────────
# Edge-case / regression tests
# ────────────────────────────────────────────────────────────────────────────

class TestEdgeCases:
    """Edge cases and boundary conditions."""

    def test_n_zero(self):
        """N=0 should produce the initial ring only."""
        graph = generate_network(N=0, m=2, p=0.5, fp=0.3)
        assert len(graph) == 10

    def test_p_zero(self):
        """p=0.0 should produce a valid network (uniform step distribution)."""
        random.seed(0)
        graph = generate_network(N=100, m=3, p=0.0, fp=0.3)
        assert len(graph) == 110
        # All edges should still be symmetric
        for u in range(len(graph)):
            for v in graph[u]:
                assert u in graph[v]

    def test_p_one(self):
        """p=1.0 should produce a valid network (all walks are 1 step)."""
        random.seed(0)
        graph = generate_network(N=100, m=3, p=1.0, fp=0.3)
        assert len(graph) == 110

    def test_fp_zero(self):
        """fp=0 means no friendship edges between marked nodes."""
        random.seed(0)
        graph = generate_network(N=100, m=3, p=0.5, fp=0.0)
        assert len(graph) == 110

    def test_fp_one(self):
        """fp=1 means all pairs of marked nodes get connected."""
        random.seed(0)
        graph = generate_network(N=100, m=3, p=0.5, fp=1.0)
        assert len(graph) == 110

    def test_large_m(self):
        """m larger than typical should still work."""
        random.seed(0)
        graph = generate_network(N=50, m=10, p=0.5, fp=0.3)
        assert len(graph) == 60
        # New nodes should have high degree
        for u in range(10, 60):
            assert len(graph[u]) >= 1

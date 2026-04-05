/*
 * Unit tests for the C++ network generator.
 *
 * This is a self-contained test runner (no external framework required).
 * Build & run:
 *   g++ -std=c++11 -O2 -DTESTING -I. -o tests/test_network tests/test_network.cpp main.cpp networkMetrics.cpp -fopenmp
 *   ./tests/test_network
 *
 * Or from the project root:
 *   make test
 */

#include <iostream>
#include <vector>
#include <unordered_set>
#include <cassert>
#include <cmath>
#include <string>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <random>
#include <cstring>
#include <fstream>
#include <cstdio>

#include "networkMetrics.h"

// ── Minimal test framework ──────────────────────────────────────────────────

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name)                                                          \
    static void test_##name();                                              \
    static struct TestRegistrar_##name {                                    \
        TestRegistrar_##name() { register_test(#name, test_##name); }       \
    } _registrar_##name;                                                    \
    static void test_##name()

struct TestEntry {
    const char* name;
    void (*func)();
};

static std::vector<TestEntry>& get_tests() {
    static std::vector<TestEntry> tests;
    return tests;
}

static void register_test(const char* name, void (*func)()) {
    get_tests().push_back({name, func});
}

#define ASSERT_TRUE(cond)                                                   \
    do {                                                                    \
        if (!(cond)) {                                                      \
            std::cerr << "  FAIL: " << #cond                                \
                      << " (" << __FILE__ << ":" << __LINE__ << ")\n";      \
            throw std::runtime_error("assertion failed");                   \
        }                                                                   \
    } while (0)

#define ASSERT_EQ(a, b)                                                     \
    do {                                                                    \
        if ((a) != (b)) {                                                   \
            std::cerr << "  FAIL: " << #a << " == " << #b                   \
                      << " (" << (a) << " != " << (b) << ")"               \
                      << " (" << __FILE__ << ":" << __LINE__ << ")\n";      \
            throw std::runtime_error("assertion failed");                   \
        }                                                                   \
    } while (0)

#define ASSERT_NEAR(a, b, tol)                                              \
    do {                                                                    \
        if (std::abs((a) - (b)) > (tol)) {                                  \
            std::cerr << "  FAIL: |" << #a << " - " << #b << "| <= " << tol \
                      << " (" << (a) << " vs " << (b) << ")"               \
                      << " (" << __FILE__ << ":" << __LINE__ << ")\n";      \
            throw std::runtime_error("assertion failed");                   \
        }                                                                   \
    } while (0)

#define ASSERT_GT(a, b)                                                     \
    do {                                                                    \
        if (!((a) > (b))) {                                                 \
            std::cerr << "  FAIL: " << #a << " > " << #b                    \
                      << " (" << (a) << " <= " << (b) << ")"               \
                      << " (" << __FILE__ << ":" << __LINE__ << ")\n";      \
            throw std::runtime_error("assertion failed");                   \
        }                                                                   \
    } while (0)

#define ASSERT_GE(a, b)                                                     \
    do {                                                                    \
        if (!((a) >= (b))) {                                                \
            std::cerr << "  FAIL: " << #a << " >= " << #b                   \
                      << " (" << (a) << " < " << (b) << ")"                \
                      << " (" << __FILE__ << ":" << __LINE__ << ")\n";      \
            throw std::runtime_error("assertion failed");                   \
        }                                                                   \
    } while (0)

#define ASSERT_LE(a, b)                                                     \
    do {                                                                    \
        if (!((a) <= (b))) {                                                \
            std::cerr << "  FAIL: " << #a << " <= " << #b                   \
                      << " (" << (a) << " > " << (b) << ")"                \
                      << " (" << __FILE__ << ":" << __LINE__ << ")\n";      \
            throw std::runtime_error("assertion failed");                   \
        }                                                                   \
    } while (0)

// ── Helper: build a small graph manually ────────────────────────────────────

static std::vector<std::unordered_set<int>> make_ring(int n) {
    std::vector<std::unordered_set<int>> g(n);
    for (int i = 0; i < n; ++i) {
        g[i].insert((i + 1) % n);
        g[(i + 1) % n].insert(i);
    }
    return g;
}

static std::vector<std::vector<int>> to_fast(std::vector<std::unordered_set<int>>& g) {
    return convertToFastGraph(g);
}

// ── Tests ───────────────────────────────────────────────────────────────────

// ---------- Network generation ----------

TEST(generate_correct_node_count) {
    int N = 200;
    auto g = generate_network(N, 3.0, 0.5, 0.3);
    ASSERT_EQ((int)g.size(), N + 10);
}

TEST(generate_n_zero_gives_ring) {
    auto g = generate_network(0, 2.0, 0.5, 0.3);
    ASSERT_EQ((int)g.size(), 10);
    // Verify ring: each node has at least 2 neighbors
    for (int i = 0; i < 10; ++i) {
        ASSERT_GE((int)g[i].size(), 2);
    }
}

TEST(generate_edges_symmetric) {
    auto g = generate_network(300, 4.0, 0.5, 0.3);
    for (size_t u = 0; u < g.size(); ++u) {
        for (int v : g[u]) {
            ASSERT_TRUE(g[v].count(u) > 0);
        }
    }
}

TEST(generate_no_self_loops) {
    auto g = generate_network(300, 4.0, 0.5, 0.3);
    for (size_t u = 0; u < g.size(); ++u) {
        ASSERT_TRUE(g[u].count(u) == 0);
    }
}

TEST(generate_new_nodes_have_edges) {
    auto g = generate_network(200, 2.0, 0.5, 0.3);
    for (size_t u = 10; u < g.size(); ++u) {
        ASSERT_GE((int)g[u].size(), 1);
    }
}

TEST(generate_more_m_more_edges) {
    auto g_low = generate_network(300, 2.0, 0.5, 0.0);
    auto g_high = generate_network(300, 8.0, 0.5, 0.0);
    size_t edges_low = 0, edges_high = 0;
    for (auto& s : g_low)  edges_low  += s.size();
    for (auto& s : g_high) edges_high += s.size();
    ASSERT_GT(edges_high, edges_low);
}

TEST(generate_fp_increases_edges) {
    auto g_no  = generate_network(300, 4.0, 0.5, 0.0);
    auto g_yes = generate_network(300, 4.0, 0.5, 1.0);
    size_t edges_no = 0, edges_yes = 0;
    for (auto& s : g_no)  edges_no  += s.size();
    for (auto& s : g_yes) edges_yes += s.size();
    ASSERT_GT(edges_yes, edges_no);
}

// ---------- convertToFastGraph ----------

TEST(convert_preserves_structure) {
    auto g = generate_network(100, 3.0, 0.5, 0.3);
    auto fg = to_fast(g);
    ASSERT_EQ(g.size(), fg.size());
    for (size_t i = 0; i < g.size(); ++i) {
        ASSERT_EQ(g[i].size(), fg[i].size());
        for (int v : fg[i]) {
            ASSERT_TRUE(g[i].count(v) > 0);
        }
    }
}

// ---------- Clustering coefficient ----------

TEST(clustering_complete_graph) {
    // K4: clustering coefficient should be 1.0
    std::vector<std::unordered_set<int>> g(4);
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            if (i != j) g[i].insert(j);
    auto fg = to_fast(g);
    double cc = calculateClusteringCoefficient(fg);
    ASSERT_NEAR(cc, 1.0, 1e-10);
}

TEST(clustering_star_graph) {
    // Star: center (0) connected to 1,2,3,4; no edges among leaves.
    // Clustering: center has CC=0, leaves have degree 1 → skip.
    // Average CC = 0.
    std::vector<std::unordered_set<int>> g(5);
    for (int i = 1; i <= 4; ++i) {
        g[0].insert(i);
        g[i].insert(0);
    }
    auto fg = to_fast(g);
    double cc = calculateClusteringCoefficient(fg);
    ASSERT_NEAR(cc, 0.0, 1e-10);
}

TEST(clustering_ring) {
    auto g = make_ring(6);
    auto fg = to_fast(g);
    double cc = calculateClusteringCoefficient(fg);
    // In a ring of 6, each node has 2 neighbors and they are not connected → CC=0
    ASSERT_NEAR(cc, 0.0, 1e-10);
}

TEST(clustering_positive_for_generated) {
    auto g = generate_network(500, 4.0, 0.5, 0.5);
    auto fg = to_fast(g);
    double cc = calculateClusteringCoefficient(fg);
    ASSERT_GT(cc, 0.0);
    ASSERT_LE(cc, 1.0);
}

// ---------- Power law ----------

TEST(power_law_returns_valid) {
    auto g = generate_network(500, 4.0, 0.5, 0.3);
    auto fg = to_fast(g);
    auto result = calculatePowerLaw(fg);
    double alpha = std::get<0>(result);
    double r     = std::get<1>(result);
    int maxDeg   = std::get<2>(result);
    ASSERT_GT(alpha, 0.0);
    ASSERT_GT(maxDeg, 0);
    // r should be a correlation coefficient: -1 <= r <= 1
    ASSERT_GE(r, -1.0 - 1e-9);
    ASSERT_LE(r, 1.0 + 1e-9);
}

TEST(power_law_cumulative_returns_valid) {
    auto g = generate_network(500, 4.0, 0.5, 0.3);
    auto fg = to_fast(g);
    auto result = calculatePowerLawCumulative(fg);
    double alpha = std::get<0>(result);
    double r     = std::get<1>(result);
    ASSERT_GT(alpha, 0.0);
    ASSERT_GE(r, -1.0 - 1e-9);
    ASSERT_LE(r, 1.0 + 1e-9);
}

TEST(power_law_small_graph) {
    // Tiny graph — should not crash
    auto g = make_ring(5);
    auto fg = to_fast(g);
    auto result = calculatePowerLaw(fg);
    // All nodes have same degree → only 1 data point → alpha = 0
    // Just verify it doesn't crash
    (void)result;
}

// ---------- Network save/load roundtrip ----------

TEST(save_load_roundtrip) {
    auto g = generate_network(100, 3.0, 0.5, 0.3);
    auto fg = to_fast(g);

    std::string testId = "test_roundtrip_tmp";
    saveFastGraphToFile(fg, testId);
    auto loaded = loadFastGraphFromFile(testId);

    ASSERT_EQ(fg.size(), loaded.size());
    for (size_t i = 0; i < fg.size(); ++i) {
        ASSERT_EQ(fg[i].size(), loaded[i].size());
        std::vector<int> sorted_orig(fg[i].begin(), fg[i].end());
        std::vector<int> sorted_load(loaded[i].begin(), loaded[i].end());
        std::sort(sorted_orig.begin(), sorted_orig.end());
        std::sort(sorted_load.begin(), sorted_load.end());
        ASSERT_TRUE(sorted_orig == sorted_load);
    }

    // Cleanup
    std::remove(("checkpoint_" + testId + "_graph.txt").c_str());
}

// ---------- save_network_to_file ----------

TEST(save_network_to_file_creates_file) {
    auto g = generate_network(50, 2.0, 0.5, 0.3);
    std::string filename = "test_edge_list_tmp.txt";
    save_network_to_file(g, filename);

    // Read back and verify
    std::ifstream f(filename);
    ASSERT_TRUE(f.is_open());

    int edge_count = 0;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        int u, v;
        std::istringstream iss(line);
        iss >> u >> v;
        ASSERT_TRUE(u < v);  // Only u < v edges written
        ASSERT_TRUE(g[u].count(v) > 0);
        ASSERT_TRUE(g[v].count(u) > 0);
        ++edge_count;
    }
    f.close();

    size_t expected_edges = 0;
    for (size_t i = 0; i < g.size(); ++i)
        for (int nb : g[i])
            if ((int)i < nb) ++expected_edges;
    ASSERT_EQ((size_t)edge_count, expected_edges);

    std::remove(filename.c_str());
}

// ---------- Edge cases ----------

TEST(generate_p_zero) {
    // p=0 triggers uniform step distribution
    auto g = generate_network(100, 3.0, 0.0, 0.3);
    ASSERT_EQ((int)g.size(), 110);
    for (size_t u = 0; u < g.size(); ++u)
        for (int v : g[u])
            ASSERT_TRUE(g[v].count(u) > 0);
}

TEST(generate_p_one) {
    auto g = generate_network(100, 3.0, 1.0, 0.3);
    ASSERT_EQ((int)g.size(), 110);
}


// ── Main ────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "========================================\n";
    std::cout << "  C++ Network Unit Tests\n";
    std::cout << "========================================\n\n";

    auto& tests = get_tests();
    for (auto& t : tests) {
        std::cout << "  Running: " << t.name << " ... ";
        tests_run++;
        try {
            t.func();
            tests_passed++;
            std::cout << "OK\n";
        } catch (const std::exception& e) {
            tests_failed++;
            std::cout << "FAILED\n";
        }
    }

    std::cout << "\n========================================\n";
    std::cout << "  Results: " << tests_passed << "/" << tests_run << " passed";
    if (tests_failed > 0) {
        std::cout << " (" << tests_failed << " FAILED)";
    }
    std::cout << "\n========================================\n";

    return tests_failed > 0 ? 1 : 0;
}

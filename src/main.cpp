#include "mst.hpp"
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>
struct BenchmarkRow {
    int n;
    int m;
    int trial;
    double boruvka_ms;
    double kruskal_ms;
    double prim_ms;
    int boruvka_phases;
    long long mst_weight;
};
static std::vector<Edge> generate_connected_graph(int n, int m, std::mt19937& rng) {
    if (n < 1 || m < n - 1 || m > n * (n - 1) / 2) {
        throw std::invalid_argument("Invalid n/m for a simple connected graph.");
    }
    std::vector<Edge> edges;
    edges.reserve(m);
    for (int v = 1; v < n; ++v) {
        std::uniform_int_distribution<int> parent_dist(0, v - 1);
        int u = parent_dist(rng);

        std::uniform_int_distribution<int> weight_dist(1, 1000000);
        edges.push_back({u, v, weight_dist(rng), static_cast<int>(edges.size())});
    }
    std::unordered_set<std::uint64_t> existing;
    existing.reserve(static_cast<std::size_t>(m) * 2);
    auto key = [](int a, int b) -> std::uint64_t {
        if (a > b) std::swap(a, b);
        return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(a)) << 32)
             | static_cast<std::uint32_t>(b);
    };
    for (const Edge& e : edges) {
        existing.insert(key(e.u, e.v));
    }
    std::uniform_int_distribution<int> vertex_dist(0, n - 1);
    std::uniform_int_distribution<int> weight_dist(1, 1000000);
    while (static_cast<int>(edges.size()) < m) {
        int u = vertex_dist(rng);
        int v = vertex_dist(rng);
        if (u == v) {
            continue;
        }
        auto k = key(u, v);
        if (existing.insert(k).second) {
            edges.push_back({u, v, weight_dist(rng), static_cast<int>(edges.size())});
        }
    }
    return edges;
}
template <typename Function>
static double timed_ms(Function&& function) {
    const auto start = std::chrono::steady_clock::now();
    function();
    const auto end = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}
static void run_correctness_demo() {
    // A small graph with an easy-to-check MST.
    std::vector<Edge> edges = {
        {0, 1, 4, 0},
        {0, 2, 3, 1},
        {1, 2, 1, 2},
        {1, 3, 2, 3},
        {2, 3, 4, 4},
        {3, 4, 2, 5},
        {2, 4, 5, 6}
    };
    auto b = boruvka_mst(5, edges);
    auto k = kruskal_mst(5, edges);
    auto p = prim_mst(5, edges);
    std::cout << "Correctness demo\n";
    std::cout << "Boruvka weight: " << b.total_weight
              << ", phases: " << b.phases << '\n';
    std::cout << "Kruskal weight: " << k.total_weight << '\n';
    std::cout << "Prim weight:    " << p.total_weight << '\n';
    if (b.edges.size() != 4 ||
        k.edges.size() != 4 ||
        p.edges.size() != 4 ||
        b.total_weight != k.total_weight ||
        b.total_weight != p.total_weight) {
        throw std::runtime_error("Correctness test failed.");
    }
    std::cout << "PASS: all three algorithms produce an MST of the same weight.\n\n";
}
static void run_benchmark(
    const std::string& filename,
    int trials) {
    const std::vector<int> sizes = {100, 500, 1000, 2000};
    const std::vector<double> densities = {2.0, 5.0, 10.0};
    std::ofstream out(filename);
    if (!out) {
        throw std::runtime_error("Could not open output CSV.");
    }
    out << "n,m,trial,boruvka_ms,kruskal_ms,prim_ms,"
           "boruvka_phases,mst_weight\n";
    std::mt19937 rng(20260923);
    std::cout << "Running benchmark...\n";
    for (int n : sizes) {
        for (double density : densities) {
            int m = static_cast<int>(density * n);
            m = std::min(m, n * (n - 1) / 2);
            for (int trial = 1; trial <= trials; ++trial) {
                auto graph = generate_connected_graph(n, m, rng);
                MSTResult b, k, p;
                double b_ms = timed_ms([&] {
                    b = boruvka_mst(n, graph);
                });
                double k_ms = timed_ms([&] {
                    k = kruskal_mst(n, graph);
                });
                double p_ms = timed_ms([&] {
                    p = prim_mst(n, graph);
                });
                if (b.edges.size() != static_cast<std::size_t>(n - 1) ||
                    k.edges.size() != static_cast<std::size_t>(n - 1) ||
                    p.edges.size() != static_cast<std::size_t>(n - 1) ||
                    b.total_weight != k.total_weight ||
                    b.total_weight != p.total_weight) {
                    throw std::runtime_error(
                        "MST validation failed during benchmark.");
                }
                out << n << ','
                    << m << ','
                    << trial << ','
                    << std::setprecision(10) << b_ms << ','
                    << k_ms << ','
                    << p_ms << ','
                    << b.phases << ','
                    << b.total_weight << '\n';
                std::cout
                    << "n=" << n
                    << " m=" << m
                    << " trial=" << trial
                    << " | Boruvka=" << b_ms << " ms"
                    << " | Kruskal=" << k_ms << " ms"
                    << " | Prim=" << p_ms << " ms"
                    << " | phases=" << b.phases
                    << '\n';
            }
        }
    }
    std::cout << "\nSaved results to " << filename << '\n';
}
int main(int argc, char** argv) {
    try {
        run_correctness_demo();
        int trials = 5;
        std::string output = "results.csv";
        if (argc >= 2) {
            trials = std::stoi(argv[1]);
        }
        if (argc >= 3) {
            output = argv[2];
        }
        if (trials <= 0) {
            throw std::invalid_argument("Trials must be positive.");
        }
        run_benchmark(output, trials);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
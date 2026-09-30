#pragma once
#include <cstdint>
#include <vector>
struct Edge {
    int u;
    int v;
    int weight;
    int id;
};
struct MSTResult {
    long long total_weight = 0;
    std::vector<Edge> edges;
    int phases = 0;
};
class UnionFind {
public:
    explicit UnionFind(int n);
    int find(int x);
    bool unite(int a, int b);
    int components() const;
private:
    std::vector<int> parent;
    std::vector<int> size;
    int component_count;
};
MSTResult boruvka_mst(int n, const std::vector<Edge>& edges);
MSTResult kruskal_mst(int n, const std::vector<Edge>& edges);
MSTResult prim_mst(int n, const std::vector<Edge>& edges);
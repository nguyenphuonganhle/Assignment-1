#include "mst.hpp"
#include <algorithm>
#include <limits>
#include <queue>
#include <tuple>
#include <utility>
#include <vector>
UnionFind::UnionFind(int n)
    : parent(n), size(n, 1), component_count(n) {
    for (int i = 0; i < n; ++i) {
        parent[i] = i;
    }
}
int UnionFind::find(int x) {
    if (parent[x] == x) {
        return x;
    }
    parent[x] = find(parent[x]);  
    return parent[x];
}
bool UnionFind::unite(int a, int b) {
    a = find(a);
    b = find(b);
    if (a == b) {
        return false;
    }
    if (size[a] < size[b]) {
        std::swap(a, b);
    }
    parent[b] = a;
    size[a] += size[b];
    --component_count;
    return true;
}
int UnionFind::components() const {
    return component_count;
}
static bool edge_less(const Edge& a, const Edge& b) {
    if (a.weight != b.weight) {
        return a.weight < b.weight;
    }
    return a.id < b.id; 
}
MSTResult boruvka_mst(int n, const std::vector<Edge>& edges) {
    MSTResult result;
    if (n <= 0) {
        return result;
    }
    UnionFind uf(n);
    while (uf.components() > 1) {
        ++result.phases;
        std::vector<int> cheapest(n, -1);
        for (const Edge& e : edges) {
            int ru = uf.find(e.u);
            int rv = uf.find(e.v);
            if (ru == rv) {
                continue;  
            }
            if (cheapest[ru] == -1 ||
                edge_less(e, edges[cheapest[ru]])) {
                cheapest[ru] = e.id;
            }
            if (cheapest[rv] == -1 ||
                edge_less(e, edges[cheapest[rv]])) {
                cheapest[rv] = e.id;
            }
        }
        bool merged_any = false;
        for (int v = 0; v < n; ++v) {
            if (uf.find(v) != v) {
                continue;  
            }
            int edge_id = cheapest[v];
            if (edge_id == -1) {
                continue;
            }
            const Edge& e = edges[edge_id];
            if (uf.unite(e.u, e.v)) {
                result.edges.push_back(e);
                result.total_weight += e.weight;
                merged_any = true;
            }
        }
        if (!merged_any) {
            result.edges.clear();
            result.total_weight = 0;
            result.phases = 0;
            return result;
        }
    }
    return result;
}
MSTResult kruskal_mst(int n, const std::vector<Edge>& edges) {
    MSTResult result;
    std::vector<Edge> sorted = edges;
    std::sort(sorted.begin(), sorted.end(), edge_less);
    UnionFind uf(n);
    for (const Edge& e : sorted) {
        if (uf.unite(e.u, e.v)) {
            result.edges.push_back(e);
            result.total_weight += e.weight;
            if (static_cast<int>(result.edges.size()) == n - 1) {
                break;
            }
        }
    }
    return result;
}
MSTResult prim_mst(int n, const std::vector<Edge>& edges) {
    MSTResult result;
    if (n <= 0) {
        return result;
    }
    std::vector<std::vector<int>> adj(n);
    for (int i = 0; i < static_cast<int>(edges.size()); ++i) {
        adj[edges[i].u].push_back(i);
        adj[edges[i].v].push_back(i);
    }
    using QueueItem = std::tuple<int, int, int>; 
    std::priority_queue<
        QueueItem,
        std::vector<QueueItem>,
        std::greater<QueueItem>
    > pq;
    std::vector<bool> used(n, false);
    used[0] = true;
    for (int edge_id : adj[0]) {
        const Edge& e = edges[edge_id];
        int next = (e.u == 0 ? e.v : e.u);
        pq.emplace(e.weight, e.id, next);
    }
    while (!pq.empty() && static_cast<int>(result.edges.size()) < n - 1) {
        auto [weight, edge_id, v] = pq.top();
        pq.pop();
        if (used[v]) {
            continue;
        }
        const Edge& e = edges[edge_id];
        result.edges.push_back(e);
        result.total_weight += weight;
        used[v] = true;
        for (int next_edge_id : adj[v]) {
            const Edge& next_edge = edges[next_edge_id];
            int next = (next_edge.u == v ? next_edge.v : next_edge.u);
            if (!used[next]) {
                pq.emplace(next_edge.weight, next_edge.id, next);
            }
        }
    }
    return result;
}
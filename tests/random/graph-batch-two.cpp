#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <cstdlib>
#include <numeric>
#include <random>
#include <set>
#include "blueberry/graph/cycle-detection.hpp"
#include "blueberry/graph/topological-sort.hpp"
#include "blueberry/graph/tree-diameter.hpp"
#include "blueberry/graph/triangle-enumeration.hpp"
using namespace std;
int main(int argc, char** argv) {
  const unsigned seed = argc > 1 ? static_cast<unsigned>(std::strtoul(argv[1], nullptr, 10)) : 20260918;
  mt19937 rng(seed);
  cerr << "graph-batch-two seed=" << seed << '\n';
  assert(blueberry::find_cycle<true>(0, {}).empty());
  assert(blueberry::topological_sort({})->empty());
  assert(blueberry::tree_diameter(vector<vector<pair<int, long long>>>{}).second.empty());
  for (int trial = 0; trial < 3000; ++trial) {
    cerr << "trial=" << trial << '\n';
    int n = 1 + rng() % 12, m = rng() % 40;
    vector<pair<int, int>> edges;
    vector<vector<int>> graph(n), reach(n, vector<int>(n));
    vector<int> dsu(n); iota(dsu.begin(), dsu.end(), 0);
    auto root = [&](int x) { while (x != dsu[x]) x = dsu[x]; return x; };
    bool undirected = false;
    for (int i = 0; i < m; ++i) {
      int u = rng() % n, v = rng() % n;
      edges.emplace_back(u, v); graph[u].push_back(v); reach[u][v] = 1;
      int a = root(u), b = root(v);
      if (a == b) undirected = true; else dsu[a] = b;
    }
    for (int k = 0; k < n; ++k) for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j)
      reach[i][j] |= reach[i][k] && reach[k][j];
    bool directed = false;
    for (int v = 0; v < n; ++v) directed |= reach[v][v];
    auto c = blueberry::find_cycle<true>(n, edges);
    assert(!c.empty() == directed);
    for (size_t i = 0; i < c.size(); ++i) assert(edges[c[i]].second == edges[c[(i + 1) % c.size()]].first);
    auto d = blueberry::find_cycle<false>(n, edges);
    assert(!d.empty() == undirected);
    assert(set<int>(d.begin(), d.end()).size() == d.size());
    if (!d.empty()) {
      bool valid = false;
      for (int start : {edges[d[0]].first, edges[d[0]].second}) {
        int v = start; bool ok = true; set<int> seen;
        for (int id : d) { auto [a, b] = edges[id]; if (v != a && v != b) { ok = false; break; } seen.insert(v); v ^= a ^ b; }
        valid |= ok && v == start && seen.size() == d.size();
      }
      assert(valid);
    }
    auto order = blueberry::topological_sort(graph); assert(order.has_value() == !directed);
    if (order) { vector<int> pos(n); for (int i = 0; i < n; ++i) pos[(*order)[i]] = i; for (auto [u, v] : edges) assert(pos[u] < pos[v]); }
    edges.clear(); vector<vector<int>> adj(n, vector<int>(n));
    for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v) if (rng() % 2) { edges.emplace_back(u, v); adj[u][v] = adj[v][u] = 1; }
    set<array<int, 3>> expected, got;
    for (int a = 0; a < n; ++a) for (int b = a + 1; b < n; ++b) for (int z = b + 1; z < n; ++z) if (adj[a][b] && adj[b][z] && adj[z][a]) expected.insert({a, b, z});
    blueberry::enumerate_triangles(n, edges, [&](int a, int b, int z) { array<int, 3> t{a,b,z}; sort(t.begin(), t.end()); assert(got.insert(t).second); });
    assert(got == expected);
    vector<vector<pair<int, long long>>> tree(n);
    vector<vector<long long>> dist(n, vector<long long>(n, 1000000));
    for (int v = 0; v < n; ++v) dist[v][v] = 0;
    for (int v = 1; v < n; ++v) { int p = rng() % v; long long w = rng() % 10; tree[v].emplace_back(p,w); tree[p].emplace_back(v,w); dist[v][p] = dist[p][v] = w; }
    for (int k = 0; k < n; ++k) for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) dist[i][j] = min(dist[i][j], dist[i][k] + dist[k][j]);
    long long best = 0; for (auto& row : dist) for (auto x : row) best = max(best, x);
    auto [length,path] = blueberry::tree_diameter(tree); assert(length == best && !path.empty());
    long long sum = 0; for (size_t i = 1; i < path.size(); ++i) { bool found = false; for (auto [v,w] : tree[path[i-1]]) if (v == path[i]) { found = true; sum += w; } assert(found); }
    assert(sum == length);
  }
  // Iterative traversal must survive a deep path without recursion.
  int n = 200000; vector<pair<int,int>> edges; vector<vector<int>> g(n);
  for (int i = 1; i < n; ++i) { edges.emplace_back(i-1,i); g[i-1].push_back(i); }
  assert(blueberry::find_cycle<true>(n, edges).empty()); assert(blueberry::find_cycle<false>(n, edges).empty()); assert(blueberry::topological_sort(g)->size() == static_cast<size_t>(n));
  vector<vector<pair<int,long long>>> deep(n);
  for (auto [u,v] : edges) { deep[u].emplace_back(v,1); deep[v].emplace_back(u,1); }
  assert(blueberry::tree_diameter(deep).first == n-1);
  assert(blueberry::tree_diameter(vector<vector<pair<int,int>>>(1)).second == vector<int>{0});
  vector<vector<pair<int,int>>> zero{{{1,0}},{{0,0}}};
  assert(blueberry::tree_diameter(zero).first == 0);
}

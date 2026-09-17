#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

namespace blueberry {
// Returns original edge IDs in traversal order; loops and parallel edges are valid.
template<bool Directed>
std::vector<int> find_cycle(int n, const std::vector<std::pair<int, int>>& edges) {
  assert(n >= 0 && edges.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
  std::vector<std::vector<int>> adj(n);
  for (int i = 0; i < static_cast<int>(edges.size()); ++i) {
    auto [u, v] = edges[i];
    assert(0 <= u && u < n && 0 <= v && v < n);
    adj[u].push_back(i);
    if constexpr (!Directed) adj[v].push_back(i);
  }
  std::vector<int> state(n), parent(n, -1), incoming(n, -1), stack;
  std::vector<std::size_t> next(n);
  stack.reserve(n);
  for (int root = 0; root < n; ++root) {
    if (state[root]) continue;
    stack.push_back(root); state[root] = 1;
    while (!stack.empty()) {
      int v = stack.back();
      if (next[v] == adj[v].size()) { state[v] = 2; stack.pop_back(); continue; }
      int id = adj[v][next[v]++];
      if constexpr (!Directed) if (id == incoming[v]) continue;
      auto [a, b] = edges[id];
      int u = Directed ? b : (a ^ b ^ v);
      if (state[u] == 1) {
        std::vector<int> cycle;
        for (int x = v; x != u; x = parent[x]) cycle.push_back(incoming[x]);
        std::reverse(cycle.begin(), cycle.end()); cycle.push_back(id);
        return cycle;
      }
      if (!state[u]) {
        state[u] = 1; parent[u] = v; incoming[u] = id; stack.push_back(u);
      }
    }
  }
  return {};
}
}  // namespace blueberry

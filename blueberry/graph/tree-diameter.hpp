#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

namespace blueberry {
// Nonnegative weighted undirected tree; returns {length, vertex path}.
template<class T>
std::pair<T, std::vector<int>> tree_diameter(const std::vector<std::vector<std::pair<int, T>>>& graph) {
  assert(graph.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
  const int n = static_cast<int>(graph.size());
  if (!n) return {T{}, {}};
  std::vector<int> parent(n, -1), order;
  std::vector<T> distance(n);
  order.reserve(n);
  auto farthest = [&](int root) {
    std::fill(parent.begin(), parent.end(), -1);
    order.clear(); order.push_back(root); parent[root] = root; distance[root] = T{};
    int best = root;
    for (std::size_t i = 0; i < order.size(); ++i) {
      int v = order[i];
      for (const auto& [u, weight] : graph[v]) {
        assert(0 <= u && u < n && !(weight < T{}));
        if (u == parent[v]) continue;
        assert(parent[u] == -1);
        parent[u] = v; distance[u] = distance[v] + weight; order.push_back(u);
        if (distance[best] < distance[u]) best = u;
      }
    }
    assert(order.size() == graph.size());
    return best;
  };
  const int a = farthest(0), b = farthest(a);
  std::vector<int> path;
  for (int v = b;; v = parent[v]) { path.push_back(v); if (v == a) break; }
  std::reverse(path.begin(), path.end());
  return {distance[b], std::move(path)};
}
}  // namespace blueberry

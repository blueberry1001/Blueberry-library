#pragma once
#include <cassert>
#include <limits>
#include <numeric>
#include <vector>

namespace blueberry {
inline std::vector<std::vector<int>> complement_components(const std::vector<std::vector<int>>& graph) {
  assert(graph.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
  const int n = static_cast<int>(graph.size());
  std::vector<int> next(n + 1), mark(n, -1);
  std::iota(next.begin(), next.end(), 1);
  next[n] = 0;  // n is the sentinel before the unvisited list.
  std::vector<std::vector<int>> result;
  while (next[n] != n) {
    const int root = next[n]; next[n] = next[root];
    result.push_back({root});
    auto& component = result.back();
    for (std::size_t head = 0; head < component.size(); ++head) {
      const int v = component[head];
      for (int u : graph[v]) { assert(0 <= u && u < n); mark[u] = v; }
      int prev = n;
      while (next[prev] != n) {
        const int u = next[prev];
        if (mark[u] == v) prev = u;
        else { next[prev] = next[u]; component.push_back(u); }
      }
    }
  }
  return result;
}
}  // namespace blueberry

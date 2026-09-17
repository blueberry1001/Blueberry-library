#pragma once
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

namespace blueberry {
// A simple undirected graph is required. The callback runs once per triangle.
template<class Callback>
void enumerate_triangles(int n, const std::vector<std::pair<int, int>>& edges, Callback&& emit) {
  assert(n >= 0 && edges.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
  std::vector<int> degree(n), mark(n, -1);
  for (auto [u, v] : edges) {
    assert(0 <= u && u < n && 0 <= v && v < n && u != v);
    ++degree[u]; ++degree[v];
  }
  std::vector<std::vector<int>> out(n);
  for (auto [u, v] : edges) {
    if (std::pair{degree[u], u} > std::pair{degree[v], v}) std::swap(u, v);
    out[u].push_back(v);
  }
  for (int u = 0; u < n; ++u) {
    for (int v : out[u]) mark[v] = u;
    for (int v : out[u]) for (int w : out[v]) if (mark[w] == u) emit(u, v, w);
  }
}
}  // namespace blueberry

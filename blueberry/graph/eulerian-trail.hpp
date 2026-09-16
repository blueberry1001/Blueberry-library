#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

namespace blueberry {

/** @brief Iterative Hierholzer with original edge IDs and optional start. */
template<bool Directed>
class EulerianTrail {
 public:
  EulerianTrail(int n, const std::vector<std::pair<int, int>>& edges, int start = -1) {
    assert(n >= 0 && (-1 <= start && start < n));
    assert(edges.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
    const int m = static_cast<int>(edges.size());
    std::vector<std::size_t> offset(static_cast<std::size_t>(n) + 1);
    std::vector<int> degree(n);
    for (const auto& [u, v] : edges) {
      assert(0 <= u && u < n && 0 <= v && v < n);
      ++offset[u + 1];
      if constexpr (Directed) { ++degree[u]; --degree[v]; }
      else { ++offset[v + 1]; degree[u] ^= 1; degree[v] ^= 1; }
    }
    if (n == 0) { exists_ = true; return; }
    int first = -1, starts = 0, ends = 0;
    for (int v = 0; v < n; ++v) {
      if constexpr (Directed) {
        if (degree[v] < -1 || degree[v] > 1) return;
        if (degree[v] == 1) { first = v; ++starts; }
        if (degree[v] == -1) ++ends;
      } else if (degree[v]) { first = v; ++starts; }
    }
    if constexpr (Directed) {
      if (!((starts == 0 && ends == 0) || (starts == 1 && ends == 1))) return;
      if (start != -1 && starts && start != first) return;
    } else {
      if (starts != 0 && starts != 2) return;
      if (start != -1 && starts && !degree[start]) return;
    }
    if (start == -1) {
      start = first;
      if (start == -1) {
        start = 0;
        for (int v = 0; v < n; ++v) if (offset[v + 1]) { start = v; break; }
      }
    }
    for (int v = 0; v < n; ++v) offset[v + 1] += offset[v];
    std::vector<int> adjacency(offset.back());
    auto next = offset;
    for (int id = 0; id < m; ++id) {
      const auto [u, v] = edges[id];
      adjacency[next[u]++] = id;
      if constexpr (!Directed) adjacency[next[v]++] = id;
    }
    next = offset;
    std::vector<unsigned char> used(Directed ? 0 : m);
    std::vector<std::pair<int, int>> stack;
    stack.reserve(static_cast<std::size_t>(m) + 1);
    vertices_.reserve(static_cast<std::size_t>(m) + 1);
    edges_.reserve(m);
    stack.emplace_back(start, -1);
    while (!stack.empty()) {
      const auto [v, incoming] = stack.back();
      if constexpr (!Directed) {
        while (next[v] < offset[v + 1] && used[adjacency[next[v]]]) ++next[v];
      }
      if (next[v] < offset[v + 1]) {
        const int id = adjacency[next[v]++];
        const auto [a, b] = edges[id];
        if constexpr (!Directed) used[id] = true;
        stack.emplace_back(Directed ? b : (a ^ b ^ v), id);
      } else {
        vertices_.push_back(v);
        if (incoming != -1) edges_.push_back(incoming);
        stack.pop_back();
      }
    }
    if (static_cast<int>(edges_.size()) != m) {
      vertices_.clear(); edges_.clear();
      return;
    }
    std::reverse(vertices_.begin(), vertices_.end());
    std::reverse(edges_.begin(), edges_.end());
    exists_ = true;
  }

  bool exists() const { return exists_; }
  const std::vector<int>& vertices() const { return vertices_; }
  const std::vector<int>& edges() const { return edges_; }

 private:
  bool exists_ = false;
  std::vector<int> vertices_, edges_;
};

}  // namespace blueberry

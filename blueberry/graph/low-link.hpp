#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

namespace blueberry {

/** @brief Iterative lowlink and two-edge-connected components of a multigraph. */
class LowLink {
 public:
  LowLink(int n, const std::vector<std::pair<int, int>>& edges) {
    assert(n >= 0);
    assert(edges.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    const int m = static_cast<int>(edges.size());
    bridge_.assign(m, false);
    articulation_.assign(n, false);
    component_.assign(n, -1);
    std::vector<std::vector<std::pair<int, int>>> graph(n);
    for (int id = 0; id < m; ++id) {
      const auto [u, v] = edges[id];
      assert(0 <= u && u < n && 0 <= v && v < n);
      graph[u].emplace_back(v, id);
      graph[v].emplace_back(u, id);
    }

    std::vector<int> order(n, -1), low(n), parent(n, -1), parent_edge(n, -1), children(n);
    std::vector<std::size_t> next(n);
    std::vector<int> stack;
    stack.reserve(n);
    int timer = 0;
    for (int root = 0; root < n; ++root) {
      if (order[root] != -1) continue;
      order[root] = low[root] = timer++;
      stack.push_back(root);
      while (!stack.empty()) {
        const int v = stack.back();
        if (next[v] < graph[v].size()) {
          const auto [to, id] = graph[v][next[v]++];
          // Skip the exact tree edge, not all parallel edges to the parent.
          if (id == parent_edge[v]) continue;
          if (order[to] == -1) {
            parent[to] = v;
            parent_edge[to] = id;
            ++children[v];
            order[to] = low[to] = timer++;
            stack.push_back(to);
          } else {
            low[v] = std::min(low[v], order[to]);
          }
        } else {
          stack.pop_back();
          const int p = parent[v];
          if (p == -1) {
            articulation_[v] = children[v] > 1;
          } else {
            low[p] = std::min(low[p], low[v]);
            bridge_[parent_edge[v]] = low[v] > order[p];
            if (parent[p] != -1 && low[v] >= order[p]) articulation_[p] = true;
          }
        }
      }
    }
    for (int id = 0; id < m; ++id) if (bridge_[id]) bridges_.push_back(id);
    for (int v = 0; v < n; ++v) if (articulation_[v]) articulation_points_.push_back(v);

    // Deleting all bridges leaves precisely the two-edge-connected components.
    for (int root = 0; root < n; ++root) {
      if (component_[root] != -1) continue;
      component_[root] = component_count_++;
      stack.push_back(root);
      while (!stack.empty()) {
        const int v = stack.back();
        stack.pop_back();
        for (const auto& [to, id] : graph[v]) {
          if (bridge_[id] || component_[to] != -1) continue;
          component_[to] = component_[root];
          stack.push_back(to);
        }
      }
    }
  }

  int size() const { return static_cast<int>(component_.size()); }

  bool is_bridge(int edge_id) const {
    assert(0 <= edge_id && edge_id < static_cast<int>(bridge_.size()));
    return bridge_[edge_id];
  }

  bool is_articulation(int v) const {
    check_vertex(v);
    return articulation_[v];
  }

  const std::vector<int>& bridges() const { return bridges_; }
  const std::vector<int>& articulation_points() const { return articulation_points_; }

  int component(int v) const {
    check_vertex(v);
    return component_[v];
  }

  std::vector<std::vector<int>> groups() const {
    std::vector<std::vector<int>> result(component_count_);
    for (int v = 0; v < size(); ++v) result[component_[v]].push_back(v);
    return result;
  }

 private:
  void check_vertex([[maybe_unused]] int v) const { assert(0 <= v && v < size()); }

  int component_count_ = 0;
  std::vector<unsigned char> bridge_, articulation_;
  std::vector<int> bridges_, articulation_points_, component_;
};

}  // namespace blueberry

#pragma once

#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

namespace blueberry {

/**
 * @brief Iterative all-roots tree DP; merge follows incident edge ID order.
 *
 * add_vertex(merged, v) includes vertex v. transfer(value, edge_id, from, to)
 * moves a rooted component's value across the edge from `from` to `to`.
 * merge is associative with two-sided identity; commutativity is not required.
 */
template <class T, class Merge, class AddVertex, class Transfer>
std::vector<T> rerooting(int n, const std::vector<std::pair<int, int>>& edges,
                        T identity, Merge merge, AddVertex add_vertex, Transfer transfer) {
  assert(n >= 0);
  assert(edges.size() == static_cast<std::size_t>(n == 0 ? 0 : n - 1));
  if (n == 0) return {};
  std::vector<std::vector<std::pair<int, int>>> graph(n);
  for (int id = 0; id < n - 1; ++id) {
    const auto [u, v] = edges[id];
    assert(0 <= u && u < n && 0 <= v && v < n && u != v);
    graph[u].emplace_back(v, id);
    graph[v].emplace_back(u, id);
  }
  std::vector<int> parent(n, -1), order{0};
  order.reserve(n);
  parent[0] = 0;
  for (int i = 0; i < static_cast<int>(order.size()); ++i) {
    const int v = order[i];
    for (const auto& [to, id] : graph[v]) {
      (void)id;
      if (to == parent[v]) continue;
      assert(parent[to] == -1);
      parent[to] = v;
      order.push_back(to);
    }
  }
  assert(static_cast<int>(order.size()) == n);

  std::vector<T> down(n, identity), up(n, identity), result(n, identity);
  for (auto it = order.rbegin(); it != order.rend(); ++it) {
    const int v = *it;
    T value = identity;
    for (const auto& [to, id] : graph[v]) {
      if (to != parent[v]) value = merge(value, transfer(down[to], id, to, v));
    }
    down[v] = add_vertex(value, v);
  }

  // Reuse temporary storage; no per-vertex prefix/suffix allocations on chains.
  std::vector<T> incoming, prefix;
  for (const int v : order) {
    incoming.clear();
    prefix.clear();
    prefix.push_back(identity);
    for (const auto& [to, id] : graph[v]) {
      incoming.push_back(to == parent[v] ? up[v] : transfer(down[to], id, to, v));
      prefix.push_back(merge(prefix.back(), incoming.back()));
    }
    result[v] = add_vertex(prefix.back(), v);
    T suffix = identity;
    for (int i = static_cast<int>(graph[v].size()) - 1; i >= 0; --i) {
      const auto [to, id] = graph[v][i];
      if (to != parent[v]) {
        up[to] = transfer(add_vertex(merge(prefix[i], suffix), v), id, v, to);
      }
      suffix = merge(incoming[i], suffix);
    }
  }
  return result;
}

}  // namespace blueberry

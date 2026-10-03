#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <numeric>
#include <vector>

namespace blueberry {

// Simple Lengauer-Tarjan: O((N + M) log(N + 1)) time, O(N + M) memory.
// idom[root] = root; unreachable vertices have idom = -1.
// The empty graph accepts root = -1. All traversals are iterative.
inline std::vector<int> dominator_tree(const std::vector<std::vector<int>>& graph, int root) {
  assert(graph.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
  const int n = static_cast<int>(graph.size());
  if (n == 0) { assert(root == -1); return {}; }
  assert(0 <= root && root < n);

  std::vector<int> index(n, -1), order, parent(n, -1), path;
  order.reserve(n);
  path.reserve(n);
  index[root] = 0;
  order.push_back(root);
  path.push_back(root);
  {
    std::vector<std::size_t> next(n);
    while (!path.empty()) {
      const int v = path.back();
      if (next[v] == graph[v].size()) { path.pop_back(); continue; }
      const int to = graph[v][next[v]++];
      assert(0 <= to && to < n);
      if (index[to] != -1) continue;
      const int k = static_cast<int>(order.size());
      index[to] = k;
      parent[k] = index[v];
      order.push_back(to);
      path.push_back(to);
    }
  }
  const int reached = static_cast<int>(order.size());

  // Flat reverse adjacency contains only edges from reachable vertices.
  std::vector<std::size_t> offset(static_cast<std::size_t>(reached) + 1);
  for (int v = 0; v < n; ++v) for (int to : graph[v]) {
    assert(0 <= to && to < n);
    if (index[v] != -1) ++offset[index[to] + 1];
  }
  for (int i = 0; i < reached; ++i) offset[i + 1] += offset[i];
  std::vector<int> predecessor(offset.back());
  {
    auto next = offset;
    for (int i = 0; i < reached; ++i)
      for (int to : graph[order[i]]) predecessor[next[index[to]]++] = i;
  }

  std::vector<int> semi(reached), label(reached), ancestor(reached, -1);
  std::vector<int> idom(reached, -1), bucket(reached, -1), next_bucket(reached, -1);
  std::iota(semi.begin(), semi.end(), 0);
  std::iota(label.begin(), label.end(), 0);
  auto eval = [&](int v) {
    // Compress from the top down; the forest root is excluded from the minimum.
    for (int u = v; ancestor[u] != -1 && ancestor[ancestor[u]] != -1; u = ancestor[u])
      path.push_back(u);
    while (!path.empty()) {
      const int u = path.back();
      path.pop_back();
      const int p = ancestor[u];
      if (semi[label[p]] < semi[label[u]]) label[u] = label[p];
      ancestor[u] = ancestor[p];
    }
    return label[v];
  };

  for (int w = reached - 1; w > 0; --w) {
    for (std::size_t i = offset[w]; i < offset[w + 1]; ++i)
      semi[w] = std::min(semi[w], semi[eval(predecessor[i])]);
    next_bucket[w] = bucket[semi[w]];
    bucket[semi[w]] = w;
    const int p = parent[w];
    ancestor[w] = p;
    for (int v = bucket[p]; v != -1; v = next_bucket[v]) {
      const int u = eval(v);
      idom[v] = semi[u] < semi[v] ? u : p;
    }
    bucket[p] = -1;
  }
  for (int w = 1; w < reached; ++w)
    if (idom[w] != semi[w]) idom[w] = idom[idom[w]];

  std::vector<int> result(n, -1);
  result[root] = root;
  for (int w = 1; w < reached; ++w) result[order[w]] = order[idom[w]];
  return result;
}

}  // namespace blueberry

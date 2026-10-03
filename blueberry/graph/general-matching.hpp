#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <numeric>
#include <utility>
#include <vector>

namespace blueberry {

// Edmonds' blossom algorithm. O(n^3 + edges.size()) time, O(n^2) memory.
// Duplicate edges are accepted; self-loops are ignored. Unmatched vertices map to -1.
inline std::vector<int> general_matching(int n, const std::vector<std::pair<int, int>>& edges) {
  assert(n >= 0);
  const auto size = static_cast<std::size_t>(n);
  assert(size == 0 || size <= std::numeric_limits<std::size_t>::max() / size);
  std::vector<int> mate(n, -1), adjacency;
  std::vector<std::size_t> offset(size + 1);
  {
    // Normalize in O(n^2 + edges.size()), then retain compact adjacency lists.
    std::vector<unsigned char> present(size * size);
    for (const auto& [u, v] : edges) {
      assert(0 <= u && u < n && 0 <= v && v < n);
      if (u == v || present[static_cast<std::size_t>(u) * size + v]) continue;
      present[static_cast<std::size_t>(u) * size + v] = true;
      present[static_cast<std::size_t>(v) * size + u] = true;
      ++offset[u + 1];
      ++offset[v + 1];
    }
    for (int v = 0; v < n; ++v) offset[v + 1] += offset[v];
    adjacency.resize(offset.back());
    auto next = offset;
    for (const auto& [u, v] : edges) {
      if (u == v || !present[static_cast<std::size_t>(u) * size + v]) continue;
      present[static_cast<std::size_t>(u) * size + v] = false;
      present[static_cast<std::size_t>(v) * size + u] = false;
      adjacency[next[u]++] = v;
      adjacency[next[v]++] = u;
    }
  }

  // A maximal initial matching avoids most searches on dense graphs.
  int matching_size = 0;
  for (int u = 0; u < n; ++u) if (mate[u] == -1) {
    for (std::size_t i = offset[u]; i < offset[u + 1]; ++i) {
      const int v = adjacency[i];
      if (mate[v] == -1) {
        mate[u] = v;
        mate[v] = u;
        ++matching_size;
        break;
      }
    }
  }
  if (matching_size == n / 2) return mate;

  std::vector<int> parent(n), base(n), queue;
  std::vector<unsigned char> outer(n), blossom(n), ancestor(n);
  queue.reserve(n);
  auto common_base = [&](int u, int v) {
    std::fill(ancestor.begin(), ancestor.end(), false);
    while (true) {
      u = base[u];
      ancestor[u] = true;
      if (mate[u] == -1) break;
      u = parent[mate[u]];
    }
    while (!ancestor[base[v]]) v = parent[mate[base[v]]];
    return base[v];
  };
  auto mark_path = [&](int v, int join, int child) {
    while (base[v] != join) {
      blossom[base[v]] = blossom[base[mate[v]]] = true;
      // Reverse the alternating-tree links on this side of the odd cycle.
      parent[v] = child;
      child = mate[v];
      v = parent[mate[v]];
    }
  };

  for (int root = 0; root < n; ++root) if (mate[root] == -1) {
    std::fill(parent.begin(), parent.end(), -1);
    std::iota(base.begin(), base.end(), 0);
    std::fill(outer.begin(), outer.end(), false);
    outer[root] = true;
    queue.assign(1, root);
    int finish = -1;
    for (std::size_t head = 0; head < queue.size() && finish == -1; ++head) {
      const int u = queue[head];
      for (std::size_t i = offset[u]; i < offset[u + 1]; ++i) {
        const int v = adjacency[i];
        if (base[u] == base[v] || mate[u] == v) continue;
        if (outer[v]) {
          const int join = common_base(u, v);
          std::fill(blossom.begin(), blossom.end(), false);
          mark_path(u, join, v);
          mark_path(v, join, u);
          for (int x = 0; x < n; ++x) if (blossom[base[x]]) {
            base[x] = join;
            if (!outer[x]) {
              outer[x] = true;
              queue.push_back(x);
            }
          }
        } else if (parent[v] == -1) {
          parent[v] = u;
          if (mate[v] == -1) {
            finish = v;
            break;
          }
          outer[mate[v]] = true;
          queue.push_back(mate[v]);
        }
      }
    }
    if (finish == -1) continue;
    ++matching_size;
    // Parent links already include the route through every contracted blossom.
    while (finish != -1) {
      const int u = parent[finish], next = mate[u];
      mate[u] = finish;
      mate[finish] = u;
      finish = next;
    }
    if (matching_size == n / 2) break;
  }
  return mate;
}

}  // namespace blueberry

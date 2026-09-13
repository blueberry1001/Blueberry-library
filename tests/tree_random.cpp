#include <algorithm>
#include <cassert>
#include <numeric>
#include <queue>
#include <random>
#include <vector>

#include "blueberry/graph/heavy-light-decomposition.hpp"
#include "blueberry/graph/lowest-common-ancestor.hpp"

int main() {
  std::mt19937 rng(20260913);
  for (int test = 0; test < 300; ++test) {
    const int n = 1 + static_cast<int>(rng() % 120);
    std::vector<std::vector<int>> tree(n);
    for (int v = 1; v < n; ++v) {
      const int p = static_cast<int>(rng() % v);
      tree[p].push_back(v);
      tree[v].push_back(p);
    }
    const int root = static_cast<int>(rng() % n);

    std::vector<int> parent(n, -1), depth(n, -1), order{root};
    parent[root] = root;
    depth[root] = 0;
    for (int i = 0; i < n; ++i) {
      const int v = order[i];
      for (const int to : tree[v]) {
        if (depth[to] != -1) continue;
        parent[to] = v;
        depth[to] = depth[v] + 1;
        order.push_back(to);
      }
    }

    const blueberry::LowestCommonAncestor doubling(tree, root);
    const blueberry::LowestCommonAncestorRMQ rmq(tree, root);
    const blueberry::HLD hld(tree, root);
    assert(doubling.size() == n && rmq.size() == n && hld.size() == n);
    assert(hld.root() == root);

    auto naive_lca = [&](int u, int v) {
      while (depth[u] > depth[v]) u = parent[u];
      while (depth[v] > depth[u]) v = parent[v];
      while (u != v) u = parent[u], v = parent[v];
      return u;
    };
    auto naive_path = [&](int u, int v) {
      const int ancestor = naive_lca(u, v);
      std::vector<int> result, tail;
      for (int x = u; x != ancestor; x = parent[x]) result.push_back(x);
      result.push_back(ancestor);
      for (int x = v; x != ancestor; x = parent[x]) tail.push_back(x);
      result.insert(result.end(), tail.rbegin(), tail.rend());
      return result;
    };

    std::vector<int> positions(n);
    for (int v = 0; v < n; ++v) {
      assert(hld.parent(v) == parent[v]);
      assert(hld.depth(v) == depth[v]);
      assert(hld.vertex_at(hld.in(v)) == v);
      positions[hld.in(v)] = v;
      int expected_size = 0;
      for (int x = 0; x < n; ++x) {
        int y = x;
        while (depth[y] > depth[v]) y = parent[y];
        if (y == v) ++expected_size;
      }
      assert(hld.subtree_size(v) == expected_size);
      assert(hld.out(v) - hld.in(v) == expected_size);
      assert(hld.subtree_range(v) == std::pair(hld.in(v), hld.out(v)));
      assert(hld.subtree_range(v, true) ==
             std::pair(hld.in(v) + 1, hld.out(v)));

      std::vector<int> subtree;
      hld.subtree_query(v, [&](int left, int right) {
        for (int i = left; i < right; ++i) subtree.push_back(hld.vertex_at(i));
      });
      for (const int x : subtree) assert(hld.is_ancestor(v, x));
      assert(static_cast<int>(subtree.size()) == expected_size);

      std::vector<int> subtree_edges;
      hld.subtree_query(v, [&](int left, int right) {
        for (int i = left; i < right; ++i)
          subtree_edges.push_back(hld.vertex_at(i));
      }, true);
      for (const int x : subtree_edges) {
        assert(x != v && hld.is_ancestor(v, x));
      }
      assert(static_cast<int>(subtree_edges.size()) == expected_size - 1);

      int single_vertex = -1;
      hld.vertex_query(v, [&](int left, int right) {
        assert(right == left + 1);
        single_vertex = hld.vertex_at(left);
      });
      assert(single_vertex == v);
    }
    std::sort(positions.begin(), positions.end());
    for (int v = 0; v < n; ++v) assert(positions[v] == v);

    for (int query = 0; query < 300; ++query) {
      const int u = static_cast<int>(rng() % n);
      const int v = static_cast<int>(rng() % n);
      const int ancestor = naive_lca(u, v);
      const int length = depth[u] + depth[v] - 2 * depth[ancestor];
      assert(doubling.lca(u, v) == ancestor);
      assert(rmq.lca(u, v) == ancestor);
      assert(hld.lca(u, v) == ancestor);
      assert(doubling.distance(u, v) == length);
      assert(rmq.distance(u, v) == length);
      assert(hld.distance(u, v) == length);

      const int k = static_cast<int>(rng() % (depth[u] + 1));
      int expected_ancestor = u;
      for (int i = 0; i < k; ++i) expected_ancestor = parent[expected_ancestor];
      assert(doubling.kth_ancestor(u, k) == expected_ancestor);
      assert(doubling.kth(u, k) == expected_ancestor);
      assert(hld.kth_ancestor(u, k) == expected_ancestor);
      assert(hld.kth(u, k) == expected_ancestor);

      const std::vector<int> path = naive_path(u, v);
      const int jump = static_cast<int>(rng() % path.size());
      assert(hld.jump(u, v, jump) == path[jump]);

      std::vector<int> actual_path;
      hld.vertex_query(u, v, [&](int left, int right, bool reversed) {
        if (reversed) {
          for (int i = right; i-- > left;) actual_path.push_back(hld.vertex_at(i));
        } else {
          for (int i = left; i < right; ++i) actual_path.push_back(hld.vertex_at(i));
        }
      });
      assert(actual_path == path);

      std::vector<int> actual_edges;
      hld.edge_query(u, v, [&](int left, int right, bool reversed) {
        if (reversed) {
          for (int i = right; i-- > left;) actual_edges.push_back(hld.vertex_at(i));
        } else {
          for (int i = left; i < right; ++i) actual_edges.push_back(hld.vertex_at(i));
        }
      });
      std::vector<int> expected_edges;
      for (int i = 1; i < static_cast<int>(path.size()); ++i) {
        const int a = path[i - 1], b = path[i];
        expected_edges.push_back(depth[a] > depth[b] ? a : b);
        assert(hld.edge_index(a, b) == hld.in(expected_edges.back()));
      }
      assert(actual_edges == expected_edges);

      std::vector<int> unordered_path;
      hld.path_query(u, v, [&](int left, int right) {
        for (int i = left; i < right; ++i) unordered_path.push_back(hld.vertex_at(i));
      });
      std::sort(unordered_path.begin(), unordered_path.end());
      std::vector<int> sorted_path = path;
      std::sort(sorted_path.begin(), sorted_path.end());
      assert(unordered_path == sorted_path);
    }
  }

  const int n = 200000;
  std::vector<std::vector<int>> path(n);
  for (int v = 1; v < n; ++v) path[v - 1].push_back(v);
  const blueberry::HLD hld(path);
  assert(hld.kth(n - 1, n - 1) == 0);
  assert(hld.jump(n - 1, 0, n - 1) == 0);
}

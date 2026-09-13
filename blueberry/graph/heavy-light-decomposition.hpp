#pragma once

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

namespace blueberry {

/**
 * @brief Heavy-light decomposition of a rooted tree.
 *
 * Vertices are assigned a preorder position so every subtree is one interval
 * `[in(v), out(v))`.  A path is split into O(log N) such intervals.  The
 * class only performs the decomposition; combine the intervals with ACL's
 * `segtree`/`lazy_segtree` or another range data structure.
 */
class HeavyLightDecomposition {
 public:
  explicit HeavyLightDecomposition(const std::vector<std::vector<int>>& tree, int root = 0)
      : n_(static_cast<int>(tree.size())),
        root_(root),
        parent_(n_, -1),
        depth_(n_, -1),
        subtree_size_(n_, 0),
        heavy_(n_, -1),
        head_(n_, -1),
        in_(n_, -1),
        out_(n_, -1),
        vertex_at_(n_, -1) {
    assert(n_ > 0);
    assert(0 <= root && root < n_);

    std::vector<int> order;
    order.reserve(n_);
    std::vector<int> stack{root};
    parent_[root] = root;
    depth_[root] = 0;
    while (!stack.empty()) {
      const int v = stack.back();
      stack.pop_back();
      order.push_back(v);
      for (const int to : tree[v]) {
        assert(0 <= to && to < n_);
        if (depth_[to] != -1) continue;
        parent_[to] = v;
        depth_[to] = depth_[v] + 1;
        stack.push_back(to);
      }
    }
    assert(static_cast<int>(order.size()) == n_);

    for (auto it = order.rbegin(); it != order.rend(); ++it) {
      const int v = *it;
      subtree_size_[v] = 1;
      int largest = 0;
      for (const int to : tree[v]) {
        if (parent_[to] != v) continue;
        subtree_size_[v] += subtree_size_[to];
        if (subtree_size_[to] > largest) {
          largest = subtree_size_[to];
          heavy_[v] = to;
        }
      }
    }

    int timer = 0;
    std::vector<std::pair<int, int>> chains{{root, root}};
    while (!chains.empty()) {
      const auto [start, chain_head] = chains.back();
      chains.pop_back();
      for (int v = start; v != -1; v = heavy_[v]) {
        head_[v] = chain_head;
        in_[v] = timer;
        vertex_at_[timer++] = v;
        for (const int to : tree[v]) {
          if (parent_[to] == v && to != heavy_[v]) chains.emplace_back(to, to);
        }
      }
    }
    assert(timer == n_);
    for (int v = 0; v < n_; ++v) out_[v] = in_[v] + subtree_size_[v];
  }

  int size() const { return n_; }
  int root() const { return root_; }

  int parent(int v) const {
    check_vertex(v);
    return parent_[v];
  }

  int depth(int v) const {
    check_vertex(v);
    return depth_[v];
  }

  int head(int v) const {
    check_vertex(v);
    return head_[v];
  }

  int in(int v) const {
    check_vertex(v);
    return in_[v];
  }

  int out(int v) const {
    check_vertex(v);
    return out_[v];
  }

  int vertex_at(int position) const {
    assert(0 <= position && position < n_);
    return vertex_at_[position];
  }

  bool is_ancestor(int ancestor, int descendant) const {
    check_vertex(ancestor);
    check_vertex(descendant);
    return in_[ancestor] <= in_[descendant] && out_[descendant] <= out_[ancestor];
  }

  int lca(int u, int v) const {
    check_vertex(u);
    check_vertex(v);
    while (head_[u] != head_[v]) {
      if (depth_[head_[u]] < depth_[head_[v]]) std::swap(u, v);
      u = parent_[head_[u]];
    }
    return depth_[u] < depth_[v] ? u : v;
  }

  int kth_ancestor(int v, int distance) const {
    check_vertex(v);
    assert(0 <= distance && distance <= depth_[v]);
    while (depth_[v] - depth_[head_[v]] < distance) {
      distance -= depth_[v] - depth_[head_[v]] + 1;
      v = parent_[head_[v]];
    }
    return vertex_at_[in_[v] - distance];
  }

  // Short spelling; keep `kth_ancestor` for descriptive call sites.
  int kth(int v, int distance) const { return kth_ancestor(v, distance); }

  int distance(int u, int v) const {
    const int ancestor = lca(u, v);
    return depth_[u] + depth_[v] - 2 * depth_[ancestor];
  }

  /**
   * Call `f(l, r)` for the O(log N) half-open ranges covering the u-v path.
   * The order of callbacks is unspecified, so this overload is intended for
   * commutative operations.  With `vertex=false`, the LCA vertex is omitted;
   * this is useful when edge values are stored at their child vertex.
   */
  template <class F>
  void path_query(int u, int v, F&& f, bool vertex = true) const {
    check_vertex(u);
    check_vertex(v);
    while (head_[u] != head_[v]) {
      if (depth_[head_[u]] < depth_[head_[v]]) std::swap(u, v);
      f(in_[head_[u]], in_[u] + 1);
      u = parent_[head_[u]];
    }
    int left = std::min(in_[u], in_[v]);
    const int right = std::max(in_[u], in_[v]) + 1;
    if (!vertex) ++left;
    if (left < right) f(left, right);
  }

  /** Call `f(in(v), out(v))` for the vertices in v's subtree. */
  template <class F>
  void subtree_query(int v, F&& f, bool vertex = true) const {
    check_vertex(v);
    const int left = in_[v] + (vertex ? 0 : 1);
    if (left < out_[v]) f(left, out_[v]);
  }

  /** Call `f(in(v), in(v) + 1)` for one vertex. */
  template <class F>
  void vertex_query(int v, F&& f) const {
    check_vertex(v);
    f(in_[v], in_[v] + 1);
  }

 private:
  void check_vertex(int v) const { assert(0 <= v && v < n_); }

  int n_;
  int root_;
  std::vector<int> parent_;
  std::vector<int> depth_;
  std::vector<int> subtree_size_;
  std::vector<int> heavy_;
  std::vector<int> head_;
  std::vector<int> in_;
  std::vector<int> out_;
  std::vector<int> vertex_at_;
};

}  // namespace blueberry

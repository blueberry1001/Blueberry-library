#pragma once

#include <algorithm>
#include <cassert>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

namespace blueberry {

/**
 * @brief Heavy-light decomposition of a rooted tree.
 *
 * Each vertex and the edge from its parent are represented by `in(v)`.  A
 * subtree is one half-open interval and a path is O(log N) intervals.
 */
class HeavyLightDecomposition {
 public:
  explicit HeavyLightDecomposition(const std::vector<std::vector<int>>& tree, int root = 0)
      : n_(static_cast<int>(tree.size())),
        root_(root),
        parent_(n_, -1),
        depth_(n_, -1),
        subtree_size_(n_, 1),
        head_(n_, -1),
        in_(n_, -1),
        out_(n_, -1),
        vertex_at_(n_, -1) {
    assert(n_ > 0);
    assert(0 <= root && root < n_);

    // vertex_at_ temporarily stores DFS order; it is filled with HLD order below.
    std::vector<int> heavy(n_, -1);
    std::vector<int> stack;
    stack.reserve(n_);
    stack.push_back(root);
    int count = 0;
    parent_[root] = root;
    depth_[root] = 0;
    while (!stack.empty()) {
      const int v = stack.back();
      stack.pop_back();
      vertex_at_[count++] = v;
      for (const int to : tree[v]) {
        assert(0 <= to && to < n_);
        if (depth_[to] != -1) continue;
        parent_[to] = v;
        depth_[to] = depth_[v] + 1;
        stack.push_back(to);
      }
    }
    assert(count == n_);

    // Reversed DFS order visits a parent's children in adjacency-list order,
    // preserving the first-largest-child tie rule without scanning edges again.
    for (int i = n_ - 1; i > 0; --i) {
      const int v = vertex_at_[i];
      const int p = parent_[v];
      subtree_size_[p] += subtree_size_[v];
      if (heavy[p] == -1 || subtree_size_[heavy[p]] < subtree_size_[v]) {
        heavy[p] = v;
      }
    }

    int timer = 0;
    stack.push_back(root);
    while (!stack.empty()) {
      const int start = stack.back();
      stack.pop_back();
      for (int v = start; v != -1; v = heavy[v]) {
        head_[v] = start;
        in_[v] = timer;
        vertex_at_[timer++] = v;
        for (const int to : tree[v]) {
          if (parent_[to] == v && to != heavy[v]) stack.push_back(to);
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

  int subtree_size(int v) const {
    check_vertex(v);
    return subtree_size_[v];
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

  int edge_index(int u, int v) const {
    check_vertex(u);
    check_vertex(v);
    assert(u != v && (parent_[u] == v || parent_[v] == u));
    return depth_[u] < depth_[v] ? in_[v] : in_[u];
  }

  std::pair<int, int> subtree_range(int v, bool edge = false) const {
    check_vertex(v);
    return {in_[v] + static_cast<int>(edge), out_[v]};
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
      if (in_[u] < in_[v]) std::swap(u, v);
      u = parent_[head_[u]];
    }
    return in_[u] < in_[v] ? u : v;
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

  int kth(int v, int distance) const { return kth_ancestor(v, distance); }

  int distance(int u, int v) const {
    const int ancestor = lca(u, v);
    return (depth_[u] - depth_[ancestor]) + (depth_[v] - depth_[ancestor]);
  }

  int jump(int u, int v, int distance) const {
    check_vertex(u);
    check_vertex(v);
    const int ancestor = lca(u, v);
    const int up = depth_[u] - depth_[ancestor];
    const int length = up + depth_[v] - depth_[ancestor];
    assert(0 <= distance && distance <= length);
    if (distance <= up) return kth_ancestor(u, distance);
    return kth_ancestor(v, length - distance);
  }

  /**
   * Visits the intervals of the u-v path in path order.
   *
   * A callback accepting `(left, right, reversed)` receives the direction of
   * each interval.  A callback accepting only `(left, right)` may be used for
   * commutative queries and range updates.  Set `edge` to omit the LCA vertex.
   */
  template <class F>
  void path_query(int u, int v, F&& f, bool edge = false) const {
    check_vertex(u);
    check_vertex(v);
    path_query_impl(u, v, f, edge);
  }

  template <class F>
  void vertex_query(int u, int v, F&& f) const {
    path_query(u, v, std::forward<F>(f), false);
  }

  template <class F>
  void edge_query(int u, int v, F&& f) const {
    path_query(u, v, std::forward<F>(f), true);
  }

  template <class F>
  void vertex_query(int v, F&& f) const {
    check_vertex(v);
    invoke(f, in_[v], in_[v] + 1, false);
  }

  template <class F>
  void subtree_query(int v, F&& f, bool edge = false) const {
    const auto [left, right] = subtree_range(v, edge);
    if (left < right) invoke(f, left, right, false);
  }

 private:
  template <class F>
  static void invoke(F& f, int left, int right, bool reversed) {
    if constexpr (std::is_invocable_v<F&, int, int, bool>) {
      f(left, right, reversed);
    } else {
      static_assert(std::is_invocable_v<F&, int, int>);
      f(left, right);
    }
  }

  template <class F>
  void path_query_impl(int u, int v, F& f, bool edge) const {
    struct Interval { int left, right; };
    // Each light edge at least halves the remaining subtree. Since N fits int,
    // this bound accommodates every possible v-side suffix, without allocation.
    Interval suffix[std::numeric_limits<unsigned int>::digits];
    int suffix_size = 0;
    while (head_[u] != head_[v]) {
      if (in_[u] > in_[v]) {
        invoke(f, in_[head_[u]], in_[u] + 1, true);
        u = parent_[head_[u]];
      } else {
        const int head = head_[v];
        suffix[suffix_size++] = {in_[head], in_[v] + 1};
        v = parent_[head];
      }
    }
    if (in_[u] <= in_[v]) {
      const int left = in_[u] + static_cast<int>(edge);
      if (left <= in_[v]) invoke(f, left, in_[v] + 1, false);
    } else {
      const int left = in_[v] + static_cast<int>(edge);
      if (left <= in_[u]) invoke(f, left, in_[u] + 1, true);
    }
    while (suffix_size > 0) {
      const auto [left, right] = suffix[--suffix_size];
      invoke(f, left, right, false);
    }
  }

  void check_vertex([[maybe_unused]] int v) const { assert(0 <= v && v < n_); }

  int n_;
  int root_;
  std::vector<int> parent_;
  std::vector<int> depth_;
  std::vector<int> subtree_size_;
  std::vector<int> head_;
  std::vector<int> in_;
  std::vector<int> out_;
  std::vector<int> vertex_at_;
};

using HLD = HeavyLightDecomposition;

}  // namespace blueberry

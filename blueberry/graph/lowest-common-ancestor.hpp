#pragma once

#include <cassert>
#include <optional>
#include <utility>
#include <vector>

#include "blueberry/data-structure/sparse-table.hpp"

namespace blueberry {

/**
 * @brief Lowest Common Ancestor (Doubling)
 */
class LowestCommonAncestor {
 public:
  explicit LowestCommonAncestor(const std::vector<std::vector<int>>& tree, int root = 0)
      : n_(static_cast<int>(tree.size())), depth_(n_, -1) {
    assert(n_ > 0);
    assert(0 <= root && root < n_);
    int levels = 1;
    while ((1LL << levels) <= n_) ++levels;
    parent_.assign(levels, std::vector<int>(n_, root));

    std::vector<int> stack{root};
    depth_[root] = 0;
    parent_[0][root] = root;
    while (!stack.empty()) {
      const int v = stack.back();
      stack.pop_back();
      for (const int to : tree[v]) {
        assert(0 <= to && to < n_);
        if (depth_[to] != -1) continue;
        depth_[to] = depth_[v] + 1;
        parent_[0][to] = v;
        stack.push_back(to);
      }
    }
    for (const int depth : depth_) assert(depth != -1);
    for (int level = 1; level < levels; ++level) {
      for (int v = 0; v < n_; ++v) {
        parent_[level][v] = parent_[level - 1][parent_[level - 1][v]];
      }
    }
  }

  int kth_ancestor(int v, int distance) const {
    assert(0 <= v && v < n_);
    assert(0 <= distance && distance <= depth_[v]);
    for (int level = 0; distance > 0; ++level, distance >>= 1) {
      if (distance & 1) v = parent_[level][v];
    }
    return v;
  }

  int kth(int v, int distance) const { return kth_ancestor(v, distance); }

  int lca(int u, int v) const {
    assert(0 <= u && u < n_ && 0 <= v && v < n_);
    if (depth_[u] < depth_[v]) std::swap(u, v);
    u = kth_ancestor(u, depth_[u] - depth_[v]);
    if (u == v) return u;
    for (int level = static_cast<int>(parent_.size()) - 1; level >= 0; --level) {
      if (parent_[level][u] != parent_[level][v]) {
        u = parent_[level][u];
        v = parent_[level][v];
      }
    }
    return parent_[0][u];
  }

  int distance(int u, int v) const {
    const int ancestor = lca(u, v);
    return depth_[u] + depth_[v] - 2 * depth_[ancestor];
  }

  int depth(int v) const {
    assert(0 <= v && v < n_);
    return depth_[v];
  }

  int size() const { return n_; }

 private:
  int n_;
  std::vector<int> depth_;
  std::vector<std::vector<int>> parent_;
};

/**
 * @brief Lowest Common Ancestor by Euler tour and Sparse Table RMQ.
 */
class LowestCommonAncestorRMQ {
 private:
  using Item = std::pair<int, int>;

  struct MinDepth {
    Item operator()(const Item& lhs, const Item& rhs) const {
      return lhs.first <= rhs.first ? lhs : rhs;
    }
  };

 public:
  explicit LowestCommonAncestorRMQ(const std::vector<std::vector<int>>& tree, int root = 0)
      : n_(static_cast<int>(tree.size())), depth_(n_, -1), first_(n_, -1) {
    assert(n_ > 0);
    assert(0 <= root && root < n_);

    std::vector<int> parent(n_, -1);
    std::vector<int> next_edge(n_, 0);
    std::vector<int> stack{root};
    std::vector<int> euler;
    euler.reserve(2 * n_ - 1);
    parent[root] = root;
    depth_[root] = 0;

    while (!stack.empty()) {
      const int v = stack.back();
      if (next_edge[v] == 0) {
        first_[v] = static_cast<int>(euler.size());
        euler.push_back(v);
      }
      if (next_edge[v] == static_cast<int>(tree[v].size())) {
        stack.pop_back();
        if (!stack.empty()) euler.push_back(stack.back());
        continue;
      }
      const int to = tree[v][next_edge[v]++];
      assert(0 <= to && to < n_);
      if (to == parent[v] || depth_[to] != -1) continue;
      parent[to] = v;
      depth_[to] = depth_[v] + 1;
      stack.push_back(to);
    }
    for (const int first : first_) assert(first != -1);

    std::vector<Item> values;
    values.reserve(euler.size());
    for (const int v : euler) values.emplace_back(depth_[v], v);
    sparse_.emplace(values, MinDepth{});
  }

  int lca(int u, int v) const {
    check_vertex(u);
    check_vertex(v);
    int left = first_[u];
    int right = first_[v];
    if (left > right) std::swap(left, right);
    return sparse_->prod(left, right + 1).second;
  }

  int distance(int u, int v) const {
    const int ancestor = lca(u, v);
    return depth_[u] + depth_[v] - 2 * depth_[ancestor];
  }

  int depth(int v) const {
    check_vertex(v);
    return depth_[v];
  }

  int size() const { return n_; }

 private:
  void check_vertex(int v) const { assert(0 <= v && v < n_); }

  int n_;
  std::vector<int> depth_;
  std::vector<int> first_;
  std::optional<SparseTable<Item, MinDepth>> sparse_;
};

}  // namespace blueberry

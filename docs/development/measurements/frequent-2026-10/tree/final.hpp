#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <optional>
#include <utility>
#include <vector>

#include "sparse-table-baseline.hpp"

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
    parent_.emplace_back(n_, root);

    std::vector<int> stack{root};
    int max_depth = 0;
    depth_[root] = 0;
    parent_[0][root] = root;
    while (!stack.empty()) {
      const int v = stack.back();
      stack.pop_back();
      for (const int to : tree[v]) {
        assert(0 <= to && to < n_);
        if (depth_[to] != -1) continue;
        depth_[to] = depth_[v] + 1;
        max_depth = std::max(max_depth, depth_[to]);
        parent_[0][to] = v;
        stack.push_back(to);
      }
    }
    for ([[maybe_unused]] const int depth : depth_) assert(depth != -1);
    const int levels = std::max(1, static_cast<int>(std::bit_width(static_cast<unsigned>(max_depth))));
    parent_.resize(levels);
    for (int level = 1; level < levels; ++level) {
      parent_[level].resize(n_);
      for (int v = 0; v < n_; ++v) {
        parent_[level][v] = parent_[level - 1][parent_[level - 1][v]];
      }
    }
  }

  int kth_ancestor(int v, int distance) const {
    assert(0 <= v && v < n_);
    assert(0 <= distance && distance <= depth_[v]);
    while (distance > 0) {
      const int level = std::countr_zero(static_cast<unsigned>(distance));
      v = parent_[level][v];
      distance &= distance - 1;
    }
    return v;
  }

  int kth(int v, int distance) const { return kth_ancestor(v, distance); }

  int lca(int u, int v) const {
    assert(0 <= u && u < n_ && 0 <= v && v < n_);
    if (depth_[u] < depth_[v]) std::swap(u, v);
    u = kth_ancestor(u, depth_[u] - depth_[v]);
    if (u == v) return u;
    for (int level = std::bit_width(static_cast<unsigned>(depth_[u])) - 1;
         level >= 0; --level) {
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
 * @brief Lowest Common Ancestor by DFS preorder and Sparse Table RMQ.
 */
class LowestCommonAncestorRMQ {
 private:
  struct MinPreorder {
    int operator()(int lhs, int rhs) const { return std::min(lhs, rhs); }
  };

 public:
  explicit LowestCommonAncestorRMQ(const std::vector<std::vector<int>>& tree, int root = 0)
      : n_(static_cast<int>(tree.size())), depth_(n_, -1), first_(n_, -1) {
    assert(n_ > 0);
    assert(0 <= root && root < n_);

    std::vector<int> stack{root};
    std::vector<int> values;
    values.reserve(n_);
    vertex_at_.reserve(n_);
    first_[root] = 0;
    depth_[root] = 0;

    while (!stack.empty()) {
      const int v = stack.back();
      stack.pop_back();
      // Before visiting v, first_[v] holds its parent's preorder index.
      values.push_back(first_[v]);
      first_[v] = static_cast<int>(vertex_at_.size());
      vertex_at_.push_back(v);
      for (const int to : tree[v]) {
        assert(0 <= to && to < n_);
        if (depth_[to] != -1) continue;
        first_[to] = first_[v];
        depth_[to] = depth_[v] + 1;
        stack.push_back(to);
      }
    }
    for ([[maybe_unused]] const int first : first_) assert(first != -1);

    // The smallest parent preorder in (first[u], first[v]] is their LCA.
    // Folklore reduction, also described in KACTL content/graph/LCA.h (CC0).
    sparse_.emplace(values, MinPreorder{});
  }

  int lca(int u, int v) const {
    check_vertex(u);
    check_vertex(v);
    if (u == v) return u;
    int left = first_[u];
    int right = first_[v];
    if (left > right) std::swap(left, right);
    return vertex_at_[sparse_->prod(left + 1, right + 1)];
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
  void check_vertex([[maybe_unused]] int v) const { assert(0 <= v && v < n_); }

  int n_;
  std::vector<int> depth_;
  std::vector<int> first_;
  std::vector<int> vertex_at_;
  std::optional<SparseTable<int, MinPreorder>> sparse_;
};

}  // namespace blueberry

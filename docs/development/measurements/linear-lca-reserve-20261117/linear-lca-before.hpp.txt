#pragma once
#include <cassert>
#include <limits>
#include <vector>
#include "blueberry/data-structure/linear-rmq.hpp"

namespace blueberry {
// Euler tour + linear-space RMQ. Input must be a connected undirected tree.
class LinearLCA {
  std::vector<int> depth_, first_, tour_;
  LinearRMQ<int> rmq_;
 public:
  explicit LinearLCA(const std::vector<std::vector<int>>& tree, int root = 0)
      : depth_(tree.size(), -1), first_(tree.size(), -1) {
    assert(!tree.empty() && tree.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max() / 2));
    assert(0 <= root && root < size());
    std::vector<int> stack{root}, parent(size(), -1), next(size(), 0), depths;
    depth_[root] = 0;
    auto visit = [&](int v) {
      if (first_[v] == -1) first_[v] = static_cast<int>(tour_.size());
      tour_.push_back(v); depths.push_back(depth_[v]);
    };
    visit(root);
    while (!stack.empty()) {
      const int v = stack.back();
      if (next[v] == static_cast<int>(tree[v].size())) {
        stack.pop_back();
        if (!stack.empty()) visit(stack.back());
      } else {
        const int w = tree[v][next[v]++];
        assert(0 <= w && w < size());
        if (w == parent[v]) continue;
        assert(depth_[w] == -1);
        parent[w] = v; depth_[w] = depth_[v] + 1;
        stack.push_back(w); visit(w);
      }
    }
    for ([[maybe_unused]] int d : depth_) assert(d >= 0);
    rmq_ = LinearRMQ<int>(depths);
  }
  int size() const { return static_cast<int>(depth_.size()); }
  int depth(int v) const { assert(0 <= v && v < size()); return depth_[v]; }
  int lca(int u, int v) const {
    assert(0 <= u && u < size() && 0 <= v && v < size());
    int l = first_[u], r = first_[v];
    if (l > r) std::swap(l, r);
    return tour_[rmq_.argmin(l, r + 1)];
  }
  int distance(int u, int v) const { return depth(u) + depth(v) - 2 * depth(lca(u, v)); }
};
}  // namespace blueberry

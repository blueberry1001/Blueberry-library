#pragma once

#include <cassert>
#include <functional>
#include <numeric>
#include <optional>
#include <utility>
#include <vector>

namespace blueberry {

// merge(u,v,w) imposes P[u] = op(P[v],w), including noncommutative groups.
template <class T, class Op = std::plus<T>, class Inv = std::negate<T>>
class PotentialUnionFind {
 public:
  explicit PotentialUnionFind(int n, Op op = {}, Inv inv = {}, T identity = {})
      : op_(std::move(op)), inv_(std::move(inv)), identity_(std::move(identity)) {
    assert(n >= 0);
    parent_.assign(n, -1);
    weight_.assign(n, identity_);
  }

  int size() const { return static_cast<int>(parent_.size()); }

  int leader(int v) {
    check(v);
    if (parent_[v] < 0) return v;
    int p = parent_[v];
    int root = leader(p);
    weight_[v] = op_(weight_[p], weight_[v]);
    return parent_[v] = root;
  }

  bool same(int u, int v) { return leader(u) == leader(v); }
  int comp_size(int v) { return -parent_[leader(v)]; }

  bool merge(int u, int v, const T& w) {
    int a = leader(u), b = leader(v);
    if (a == b) return weight_[u] == op_(weight_[v], w);
    T delta = op_(op_(weight_[v], w), inv_(weight_[u]));
    if (parent_[a] < parent_[b]) {
      std::swap(a, b);
      delta = inv_(delta);
    }
    parent_[b] += parent_[a];
    parent_[a] = b;
    weight_[a] = std::move(delta);
    return true;
  }

  std::optional<T> diff(int u, int v) {
    if (leader(u) != leader(v)) return std::nullopt;
    return op_(inv_(weight_[v]), weight_[u]);
  }

 private:
  void check([[maybe_unused]] int v) const { assert(0 <= v && v < size()); }
  Op op_;
  Inv inv_;
  T identity_;
  std::vector<int> parent_;
  // inv(P[parent[v]]) * P[v]; roots hold identity.
  std::vector<T> weight_;
};

}  // namespace blueberry

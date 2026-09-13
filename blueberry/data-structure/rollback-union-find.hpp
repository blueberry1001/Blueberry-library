#pragma once

#include <algorithm>
#include <cassert>
#include <vector>

namespace blueberry {

/**
 * @brief Rollback Union Find
 */
class RollbackUnionFind {
 public:
  explicit RollbackUnionFind(int n) : components_(n) {
    assert(n >= 0);
    parent_or_size_.assign(n, -1);
  }

  int leader(int v) const {
    assert(0 <= v && v < size());
    while (parent_or_size_[v] >= 0) v = parent_or_size_[v];
    return v;
  }

  bool merge(int u, int v) {
    u = leader(u);
    v = leader(v);
    if (-parent_or_size_[u] < -parent_or_size_[v]) std::swap(u, v);
    history_.push_back({u, parent_or_size_[u], v, parent_or_size_[v]});
    if (u == v) return false;
    parent_or_size_[u] += parent_or_size_[v];
    parent_or_size_[v] = u;
    --components_;
    return true;
  }

  bool same(int u, int v) const { return leader(u) == leader(v); }

  int component_size(int v) const { return -parent_or_size_[leader(v)]; }

  int components() const { return components_; }

  int size() const { return static_cast<int>(parent_or_size_.size()); }

  int state() const { return static_cast<int>(history_.size()); }

  void snapshot() { snapshot_ = state(); }

  void undo() {
    assert(!history_.empty());
    const Change change = history_.back();
    history_.pop_back();
    parent_or_size_[change.u] = change.parent_u;
    parent_or_size_[change.v] = change.parent_v;
    if (change.u != change.v) ++components_;
  }

  void rollback(int target_state) {
    assert(0 <= target_state && target_state <= state());
    while (state() > target_state) undo();
  }

  void rollback() { rollback(snapshot_); }

 private:
  struct Change {
    int u;
    int parent_u;
    int v;
    int parent_v;
  };

  std::vector<int> parent_or_size_;
  std::vector<Change> history_;
  int components_;
  int snapshot_ = 0;
};

}  // namespace blueberry

#pragma once

#include <cassert>
#include <utility>
#include <vector>

namespace blueberry {

// Version 0 is the initial array. All updates append an independent version.
template <class T, class Op>
class PersistentSegmentTree {
 public:
  PersistentSegmentTree(int n, Op op, T identity)
      : n_(n), op_(std::move(op)), identity_(std::move(identity)) {
    assert(n >= 0);
    nodes_.push_back({identity_, 0, 0});
    roots_.push_back(0);
  }
  PersistentSegmentTree(const std::vector<T>& a, Op op, T identity)
      : PersistentSegmentTree(static_cast<int>(a.size()), std::move(op), std::move(identity)) {
    if (n_) roots_[0] = build(a, 0, n_);
  }
  int size() const { return n_; }
  int versions() const { return static_cast<int>(roots_.size()); }
  int set(int version, int p, const T& value) {
    check_version(version);
    assert(0 <= p && p < n_);
    int root = update(roots_[version], 0, n_, p, value, false);
    roots_.push_back(root);
    return versions() - 1;
  }
  int apply(int version, int p, const T& value) {
    check_version(version);
    assert(0 <= p && p < n_);
    int root = update(roots_[version], 0, n_, p, value, true);
    roots_.push_back(root);
    return versions() - 1;
  }
  T get(int version, int p) const {
    assert(0 <= p && p < n_);
    return prod(version, p, p + 1);
  }
  T prod(int version, int l, int r) const {
    check_version(version);
    assert(0 <= l && l <= r && r <= n_);
    return query(roots_[version], 0, n_, l, r);
  }
  T all_prod(int version) const {
    check_version(version);
    return nodes_[roots_[version]].value;
  }

 private:
  struct Node { T value; int left, right; };
  void check_version([[maybe_unused]] int version) const {
    assert(0 <= version && version < versions());
  }
  int append(Node node) {
    nodes_.push_back(std::move(node));
    return static_cast<int>(nodes_.size()) - 1;
  }
  int build(const std::vector<T>& a, int l, int r) {
    if (r - l == 1) return append({a[l], 0, 0});
    int m = l + (r - l) / 2;
    int left = build(a, l, m), right = build(a, m, r);
    return append({op_(nodes_[left].value, nodes_[right].value), left, right});
  }
  int update(int v, int l, int r, int p, const T& value, bool combine) {
    if (r - l == 1) return append({combine ? op_(nodes_[v].value, value) : value, 0, 0});
    int m = l + (r - l) / 2;
    int left = nodes_[v].left, right = nodes_[v].right;
    if (p < m) left = update(left, l, m, p, value, combine);
    else right = update(right, m, r, p, value, combine);
    return append({op_(nodes_[left].value, nodes_[right].value), left, right});
  }
  T query(int v, int l, int r, int a, int b) const {
    if (v == 0 || b <= l || r <= a) return identity_;
    if (a <= l && r <= b) return nodes_[v].value;
    int m = l + (r - l) / 2;
    return op_(query(nodes_[v].left, l, m, a, b), query(nodes_[v].right, m, r, a, b));
  }
  int n_;
  Op op_;
  T identity_;
  std::vector<Node> nodes_;
  std::vector<int> roots_;
};

}  // namespace blueberry

#pragma once
#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

namespace blueberry {

// Arena-owned path copying; version 0 is the initial array.
template <class T>
class PersistentArray {
  struct Node { int left = 0, right = 0; T value{}; };
  int n_ = 0;
  T initial_{};
  std::vector<Node> nodes_{Node{}};
  std::vector<int> roots_{0};

  int append(Node node) {
    assert(nodes_.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
    nodes_.push_back(node);
    return static_cast<int>(nodes_.size()) - 1;
  }
  int build(const std::vector<T>& a, int l, int r) {
    if (r - l == 1) return append({0, 0, a[l]});
    int m = l + (r - l) / 2;
    int left = build(a, l, m), right = build(a, m, r);
    return append({left, right, T{}});
  }
  int update(int node, int l, int r, int p, const T& value) {
    if (r - l == 1) return append({0, 0, value});
    int m = l + (r - l) / 2;
    Node copy = nodes_[node];
    if (p < m) copy.left = update(copy.left, l, m, p, value);
    else copy.right = update(copy.right, m, r, p, value);
    return append(copy);
  }
  void check_version([[maybe_unused]] int version) const {
    assert(0 <= version && static_cast<std::size_t>(version) < roots_.size());
  }

 public:
  PersistentArray() = default;
  explicit PersistentArray(int n, const T& initial = T{}) : n_(n), initial_(initial) { assert(n >= 0); }
  explicit PersistentArray(const std::vector<T>& values) {
    assert(values.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max() / 2));
    n_ = static_cast<int>(values.size());
    if (n_) roots_[0] = build(values, 0, n_);
  }
  int size() const { return n_; }
  int versions() const { return static_cast<int>(roots_.size()); }
  int fork(int version) {
    check_version(version);
    assert(roots_.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
    int node = roots_[version];
    roots_.push_back(node);
    return versions() - 1;
  }
  T get(int version, int p) const {
    check_version(version);
    assert(0 <= p && p < n_);
    int node = roots_[version], l = 0, r = n_;
    while (node && r - l > 1) {
      int m = l + (r - l) / 2;
      if (p < m) { node = nodes_[node].left; r = m; }
      else { node = nodes_[node].right; l = m; }
    }
    return node ? nodes_[node].value : initial_;
  }
  int set(int version, int p, const T& value) {
    check_version(version);
    assert(0 <= p && p < n_);
    assert(roots_.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
    int node = update(roots_[version], 0, n_, p, value);
    roots_.push_back(node);
    return versions() - 1;
  }
};

}  // namespace blueberry

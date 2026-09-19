#pragma once
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

namespace blueberry {
// Range actions / point values. composition(f,g) means f after g.
template <class S, class F, auto mapping, auto composition, auto id>
class DualSegmentTree {
  std::vector<S> values_;
  std::vector<F> lazy_;
  std::vector<bool> pending_;
  std::size_t base_ = 1;
  int height_ = 0;
  void apply_node(std::size_t node, const F& f) {
    if (node >= base_) {
      if (node - base_ < values_.size()) values_[node - base_] = mapping(f, values_[node - base_]);
    } else {
      lazy_[node] = pending_[node] ? composition(f, lazy_[node]) : f;
      pending_[node] = true;
    }
  }
  void push(std::size_t node) {
    if (!pending_[node]) return;
    apply_node(node * 2, lazy_[node]); apply_node(node * 2 + 1, lazy_[node]);
    pending_[node] = false;
  }
  void expose(int p) { for (int h = height_; h > 0; --h) push((base_ + p) >> h); }
  static std::size_t checked_size(int n) { assert(n >= 0); return static_cast<std::size_t>(n); }
 public:
  DualSegmentTree() = default;
  explicit DualSegmentTree(const std::vector<S>& values) : values_(values) {
    assert(values.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    while (base_ < values.size()) { base_ *= 2; ++height_; }
    lazy_.assign(base_, id()); pending_.assign(base_, false);
  }
  DualSegmentTree(int n, const S& value) : DualSegmentTree(std::vector<S>(checked_size(n), value)) {}
  DualSegmentTree(const DualSegmentTree&) = default;
  DualSegmentTree& operator=(const DualSegmentTree&) = default;
  DualSegmentTree(DualSegmentTree&& other) noexcept
      : values_(std::move(other.values_)), lazy_(std::move(other.lazy_)), pending_(std::move(other.pending_)),
        base_(std::exchange(other.base_, 1)), height_(std::exchange(other.height_, 0)) {
    other.values_.clear(); other.lazy_.clear(); other.pending_.clear();
  }
  DualSegmentTree& operator=(DualSegmentTree&& other) noexcept {
    if (this != &other) {
      values_ = std::move(other.values_); lazy_ = std::move(other.lazy_); pending_ = std::move(other.pending_);
      base_ = std::exchange(other.base_, 1); height_ = std::exchange(other.height_, 0);
      other.values_.clear(); other.lazy_.clear(); other.pending_.clear();
    }
    return *this;
  }
  int size() const { return static_cast<int>(values_.size()); }
  S get(int p) { assert(0 <= p && p < size()); expose(p); return values_[p]; }
  void set(int p, const S& value) { assert(0 <= p && p < size()); expose(p); values_[p] = value; }
  void apply(int p, const F& f) { assert(0 <= p && p < size()); expose(p); values_[p] = mapping(f, values_[p]); }
  void apply(int l, int r, const F& f) {
    assert(0 <= l && l <= r && r <= size());
    if (l == r) return;
    std::size_t left = base_ + l, right = base_ + r;
    for (int h = height_; h > 0; --h) {
      if (((left >> h) << h) != left) push(left >> h);
      if (((right >> h) << h) != right) push((right - 1) >> h);
    }
    while (left < right) {
      if (left & 1) apply_node(left++, f);
      if (right & 1) apply_node(--right, f);
      left /= 2; right /= 2;
    }
  }
  std::vector<S> get_all() {
    if (values_.empty()) return {};
    for (std::size_t node = 1; node < base_; ++node) push(node);
    return values_;
  }
};
}  // namespace blueberry

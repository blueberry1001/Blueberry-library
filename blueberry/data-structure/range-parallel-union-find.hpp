#pragma once
#include <algorithm>
#include <bit>
#include <cassert>
#include <concepts>
#include <vector>
#include <atcoder/dsu>

namespace blueberry {

// Layer k remembers already equated length-2^k blocks; layer 0 is the real DSU.
class RangeParallelUnionFind {
  int n_, components_;
  std::vector<atcoder::dsu> layers_;
  template <class F>
  void merge_block(int layer, int a, int b, F& callback) {
    a = layers_[layer].leader(a);
    b = layers_[layer].leader(b);
    if (a == b) return;
    int leader = layers_[layer].merge(a, b);
    if (layer == 0) {
      --components_;
      callback(leader, a == leader ? b : a);
    } else {
      const int half = 1 << (layer - 1);
      merge_block(layer - 1, a, b, callback);
      merge_block(layer - 1, a + half, b + half, callback);
    }
  }

 public:
  explicit RangeParallelUnionFind(int n = 0) : n_(n), components_(n) {
    assert(n >= 0);
    const int count = std::max(1, static_cast<int>(std::bit_width(static_cast<unsigned>(n))));
    layers_.reserve(count);
    for (int k = 0; k < count; ++k) layers_.emplace_back(std::max(0, n - (1 << k) + 1));
  }
  int size() const { return n_; }
  int components() const { return components_; }
  int leader(int p) { assert(0 <= p && p < n_); return layers_[0].leader(p); }
  int comp_size(int p) { assert(0 <= p && p < n_); return layers_[0].size(p); }
  bool same(int a, int b) { return leader(a) == leader(b); }
  template <class F> requires std::invocable<F&, int, int>
  void range_merge(int a, int b, int length, F callback) {
    assert(0 <= a && a <= n_ && 0 <= b && b <= n_);
    assert(0 <= length && length <= n_ - a && length <= n_ - b);
    if (length == 0) return;
    const int k = std::bit_width(static_cast<unsigned>(length)) - 1;
    const int span = 1 << k;
    merge_block(k, a, b, callback);
    merge_block(k, a + length - span, b + length - span, callback);
  }
  void range_merge(int a, int b, int length) { range_merge(a, b, length, [](int, int) {}); }
  template <class F> requires std::invocable<F&, int, int>
  void merge(int a, int b, F callback) { range_merge(a, b, 1, callback); }
  void merge(int a, int b) { range_merge(a, b, 1); }
};

}  // namespace blueberry

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include "blueberry/extra-fast/segment-tree.hpp"

namespace blueberry::extra_fast {

// Range add/sum modulo 2^64, using two SIMD prefix trees over differences.
// This specializes additive updates; it is not a general lazy segment tree.
class RangeAddSum {
 public:
  explicit RangeAddSum(int n = 0, bool use_simd = true)
      : difference_(n, use_simd), weighted_(n, use_simd) {}

  explicit RangeAddSum(const std::vector<std::uint64_t>& values,
                       bool use_simd = true)
      : difference_(differences(values, false), use_simd),
        weighted_(differences(values, true), use_simd) {
    for (auto x : values) total_ += x;
  }

  RangeAddSum(const RangeAddSum&) = default;
  RangeAddSum(RangeAddSum&&) noexcept = default;
  RangeAddSum& operator=(RangeAddSum other) noexcept {
    std::swap(difference_, other.difference_);
    std::swap(weighted_, other.weighted_);
    std::swap(total_, other.total_);
    return *this;
  }

  int size() const { return difference_.size(); }

  void apply(int l, int r, std::uint64_t x) {
    assert(0 <= l && l <= r && r <= size());
    if (l == r) return;
    endpoint(l, x);
    if (r < size()) endpoint(r, -x);
    total_ += static_cast<std::uint64_t>(r - l) * x;
  }

  std::uint64_t get(int p) const {
    assert(0 <= p && p < size());
    return difference_.pref(p + 1);
  }
  void set(int p, std::uint64_t x) { apply(p, p + 1, x - get(p)); }

  std::uint64_t pref(int r) const {
    assert(0 <= r && r <= size());
    return static_cast<std::uint64_t>(r) * difference_.pref(r) - weighted_.pref(r);
  }
  std::uint64_t prod(int l, int r) const {
    assert(0 <= l && l <= r && r <= size());
    if (l == r) return 0;
    return pref(r) - pref(l);
  }
  std::uint64_t all_prod() const { return total_; }
  bool uses_simd() const { return difference_.uses_simd(); }
  static bool simd_available() { return SumSegmentTree::simd_available(); }

 private:
  static std::vector<std::uint64_t> differences(
      const std::vector<std::uint64_t>& a, bool weighted) {
    std::vector<std::uint64_t> result(a.size());
    std::uint64_t previous = 0;
    for (std::size_t i = 0; i < a.size(); ++i) {
      result[i] = (a[i] - previous) * (weighted ? i : 1);
      previous = a[i];
    }
    return result;
  }
  void endpoint(int p, std::uint64_t x) {
    difference_.add(p, x);
    weighted_.add(p, x * static_cast<std::uint64_t>(p));
  }

  SumSegmentTree difference_, weighted_;
  std::uint64_t total_ = 0;
};

}  // namespace blueberry::extra_fast

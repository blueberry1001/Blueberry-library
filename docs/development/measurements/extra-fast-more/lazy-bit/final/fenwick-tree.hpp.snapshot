#pragma once

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace blueberry::extra_fast {

// uint64_t addition modulo 2^64. Padding separates power-of-two strides
// that otherwise map to the same cache sets; it adds about 0.1% storage.
class FenwickTree {
 public:
  explicit FenwickTree(int n = 0) : data_(storage(n), 0), n_(n) {}

  explicit FenwickTree(const std::vector<std::uint64_t>& values)
      : FenwickTree(checked_size(values.size())) {
    for (int i = 1; i <= n_; ++i) {
      data_[slot(i)] += values[i - 1];
      const int jump = i & -i;
      if (jump <= n_ - i) data_[slot(i + jump)] += data_[slot(i)];
    }
  }

  int size() const { return n_; }

  void add(int p, std::uint64_t x) {
    assert(0 <= p && p < n_);
    for (unsigned i = static_cast<unsigned>(p) + 1; i <= static_cast<unsigned>(n_);
         i += i & -i)
      data_[slot(i)] += x;
  }

  std::uint64_t pref(int r) const {
    assert(0 <= r && r <= n_);
    std::uint64_t result = 0;
    for (unsigned i = r; i; i &= i - 1) result += data_[slot(i)];
    return result;
  }

  std::uint64_t prod(int l, int r) const {
    assert(0 <= l && l <= r && r <= n_);
    return pref(r) - pref(l);
  }

  std::uint64_t get(int p) const {
    assert(0 <= p && p < n_);
    // The element and its smaller Fenwick children share one interval.
    unsigned i = static_cast<unsigned>(p) + 1;
    std::uint64_t result = data_[slot(i)];
    const unsigned end = i & (i - 1);
    for (--i; i != end; i &= i - 1) result -= data_[slot(i)];
    return result;
  }

  void set(int p, std::uint64_t x) { add(p, x - get(p)); }
  std::uint64_t all_prod() const { return pref(n_); }

  // Requires nondecreasing (non-wrapping) prefix sums. Returns the first r
  // in [0,n] with pref(r) >= x, or n+1 if no such prefix exists.
  int lower_bound(std::uint64_t x) const {
    if (x == 0) return 0;
    unsigned p = 0;
    std::uint64_t sum = 0;
    for (unsigned step = std::bit_floor(static_cast<unsigned>(n_)); step; step >>= 1) {
      if (step <= static_cast<unsigned>(n_) - p && sum + data_[slot(p + step)] < x) {
        sum += data_[slot(p + step)];
        p += step;
      }
    }
    return static_cast<int>(p) + 1;
  }

 private:
  static std::size_t slot(unsigned i) { return i + (i >> 10); }
  static std::size_t storage(int n) {
    assert(0 <= n && n < std::numeric_limits<int>::max());
    return slot(static_cast<unsigned>(n)) + 1;
  }
  static int checked_size(std::size_t n) {
    assert(n < static_cast<std::size_t>(std::numeric_limits<int>::max()));
    return static_cast<int>(n);
  }

  std::vector<std::uint64_t> data_;
  int n_;
};

}  // namespace blueberry::extra_fast

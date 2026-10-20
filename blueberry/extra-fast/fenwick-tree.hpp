#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

#include "blueberry/extra-fast/segment-tree.hpp"

#if (defined(__x86_64__) || defined(__i386__)) && \
    (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#define BLUEBERRY_EXTRA_FAST_FENWICK_AVX2 1
#else
#define BLUEBERRY_EXTRA_FAST_FENWICK_AVX2 0
#endif

namespace blueberry::extra_fast {

// Fenwick-compatible API over 16-way child prefixes modulo 2^64.
// Updates reuse the AVX2 suffix-add kernel; searches compare 16 prefixes
// per level. Runtime CPU detection retains a scalar fallback.
class FenwickTree {
 public:
  explicit FenwickTree(int n = 0) : tree_(checked_size(n)) {}

  explicit FenwickTree(const std::vector<std::uint64_t>& values)
      : tree_(checked_values(values)) {}

  int size() const { return tree_.size(); }
  void add(int p, std::uint64_t x) { tree_.add(p, x); }
  void set(int p, std::uint64_t x) { tree_.set(p, x); }
  std::uint64_t get(int p) const { return tree_.get(p); }
  std::uint64_t pref(int r) const { return tree_.pref(r); }
  std::uint64_t prod(int l, int r) const { return tree_.prod(l, r); }
  std::uint64_t all_prod() const { return tree_.all_prod(); }

  // Requires nondecreasing, non-wrapping prefix sums. The result is the first
  // r in [0,n] with pref(r) >= x, or n+1 if none exists; x==0 returns 0.
  int lower_bound(std::uint64_t x) const {
    if (x == 0) return 0;
    if (x > all_prod()) return size() + 1;
#if BLUEBERRY_EXTRA_FAST_FENWICK_AVX2
    if (tree_.uses_simd()) return lower_bound_simd(x);
#endif
    std::size_t p = 0;
    for (int level = tree_.levels_ - 1; ; --level) {
      const auto* node = tree_.data_.data() + tree_.offsets_[level] + p;
      int l = 0, r = 16;
      while (l < r) {
        const int m = (l + r) / 2;
        if (node[m] < x) l = m + 1;
        else r = m;
      }
      if (l) x -= node[l - 1];
      p += l;
      if (level == 0) return static_cast<int>(p) + 1;
      p *= 16;
    }
  }

 private:
  static int checked_size(int n) {
    assert(0 <= n && n < std::numeric_limits<int>::max());
    return n;
  }
  static const std::vector<std::uint64_t>& checked_values(
      const std::vector<std::uint64_t>& values) {
    assert(values.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
    return values;
  }

#if BLUEBERRY_EXTRA_FAST_FENWICK_AVX2
  __attribute__((target("avx2"))) int lower_bound_simd(std::uint64_t x) const {
    const __m256i sign = _mm256_set1_epi64x(std::numeric_limits<long long>::min());
    std::size_t p = 0;
    for (int level = tree_.levels_ - 1; ; --level) {
      const auto* node = tree_.data_.data() + tree_.offsets_[level] + p;
      const __m256i target = _mm256_xor_si256(
          _mm256_set1_epi64x(static_cast<long long>(x)), sign);
      unsigned mask = 0;
#pragma GCC unroll 4
      for (int j = 0; j < 16; j += 4) {
        const __m256i values = _mm256_xor_si256(
            _mm256_loadu_si256(reinterpret_cast<const __m256i*>(node + j)), sign);
        const __m256i less = _mm256_cmpgt_epi64(target, values);
        mask |= static_cast<unsigned>(_mm256_movemask_pd(_mm256_castsi256_pd(less))) << j;
      }
      // Prefixes are sorted, and at least the final one is >= x.
      const int j = __builtin_ctz(~mask);
      if (j) x -= node[j - 1];
      p += j;
      if (level == 0) return static_cast<int>(p) + 1;
      p *= 16;
    }
  }
#endif

  SumSegmentTree tree_;
};

}  // namespace blueberry::extra_fast

#undef BLUEBERRY_EXTRA_FAST_FENWICK_AVX2

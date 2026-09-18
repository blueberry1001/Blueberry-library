#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

#include "blueberry/data-structure/fenwick-tree.hpp"

namespace blueberry {

// Fixed values, point-add weights, and index/value rectangle sums.
template <class Value, class Weight = long long>
class WeightedWaveletMatrix {
  struct Level {
    std::vector<std::uint64_t> bits;
    std::vector<int> prefix;
    int zeros = 0;
    FenwickTree<Weight> weights{0};
    int rank(int r) const {
      return prefix[r / 64] +
             std::popcount(bits[r / 64] & ((std::uint64_t{1} << (r % 64)) - 1));
    }
  };
  int n_ = 0;
  std::vector<Value> values_;
  std::vector<Level> levels_;
  FenwickTree<Weight> total_{0};

  int lower(const Value& value) const {
    return static_cast<int>(std::lower_bound(values_.begin(), values_.end(), value) - values_.begin());
  }
  void check([[maybe_unused]] int l, [[maybe_unused]] int r) const {
    assert(0 <= l && l <= r && r <= n_);
  }
  Weight less(int l, int r, int code) const {
    if (code == static_cast<int>(values_.size())) return total_.sum(l, r);
    Weight result{};
    int shift = static_cast<int>(levels_.size());
    for (const auto& level : levels_) {
      --shift;
      const int zl = l - level.rank(l), zr = r - level.rank(r);
      if ((code >> shift) & 1) {
        result += level.weights.sum(zl, zr);
        l = level.zeros + l - zl;
        r = level.zeros + r - zr;
      } else {
        l = zl;
        r = zr;
      }
    }
    return result;
  }

 public:
  WeightedWaveletMatrix() = default;

  WeightedWaveletMatrix(const std::vector<Value>& values, const std::vector<Weight>& weights)
      : values_(values) {
    assert(values.size() == weights.size());
    assert(values.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max() / 2));
    n_ = static_cast<int>(values.size());
    total_ = FenwickTree<Weight>(weights);
    std::sort(values_.begin(), values_.end());
    values_.erase(std::unique(values_.begin(), values_.end()), values_.end());
    if (n_ == 0) return;
    const int height = std::bit_width(static_cast<unsigned>(values_.size() - 1));
    std::vector<int> codes(n_), next(n_);
    std::vector<Weight> current(weights), next_weights(n_);
    for (int i = 0; i < n_; ++i) codes[i] = lower(values[i]);
    levels_.resize(height);
    for (int h = 0; h < height; ++h) {
      auto& level = levels_[h];
      const int shift = height - 1 - h;
      level.bits.assign(n_ / 64 + 1, 0);
      level.prefix.assign(n_ / 64 + 1, 0);
      for (int i = 0; i < n_; ++i)
        if ((codes[i] >> shift) & 1) level.bits[i / 64] |= std::uint64_t{1} << (i % 64);
      for (int b = 1; b <= n_ / 64; ++b)
        level.prefix[b] = level.prefix[b - 1] + std::popcount(level.bits[b - 1]);
      level.zeros = n_ - level.rank(n_);
      int zero = 0, one = level.zeros;
      for (int i = 0; i < n_; ++i) {
        const int p = ((codes[i] >> shift) & 1) ? one++ : zero++;
        next[p] = codes[i];
        next_weights[p] = current[i];
      }
      level.weights = FenwickTree<Weight>(next_weights);
      codes.swap(next);
      current.swap(next_weights);
    }
  }

  int size() const { return n_; }

  void add(int p, const Weight& delta) {
    assert(0 <= p && p < n_);
    total_.add(p, delta);
    for (auto& level : levels_) {
      const bool one = (level.bits[p / 64] >> (p % 64)) & 1;
      const int rank = level.rank(p);
      p = one ? level.zeros + rank : p - rank;
      level.weights.add(p, delta);
    }
  }

  Weight sum(int l, int r) const { check(l, r); return total_.sum(l, r); }
  Weight sum(int l, int r, const Value& upper) const {
    check(l, r);
    return less(l, r, lower(upper));
  }
  Weight sum(int l, int r, const Value& low, const Value& upper) const {
    check(l, r);
    assert(!(upper < low));
    return less(l, r, lower(upper)) - less(l, r, lower(low));
  }
};

}  // namespace blueberry

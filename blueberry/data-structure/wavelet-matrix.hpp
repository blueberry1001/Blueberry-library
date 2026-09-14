#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>
#include <vector>

namespace blueberry {

/**
 * @brief Wavelet Matrix
 */
template <class T>
class WaveletMatrix {
  static_assert(std::is_integral_v<T> && !std::is_same_v<T, bool>);

  struct Level {
    std::vector<std::uint64_t> bits;
    std::vector<int> prefix;
    int zeros = 0;

    int rank(int r) const {
      const int block = r / 64, offset = r % 64;
      return prefix[block] + std::popcount(bits[block] & ((std::uint64_t{1} << offset) - 1));
    }
  };

  int n_ = 0;
  int height_ = 0;
  std::vector<T> values_;
  std::vector<Level> levels_;

  int lower(const T& x) const {
    return static_cast<int>(std::lower_bound(values_.begin(), values_.end(), x) - values_.begin());
  }

  void check_range([[maybe_unused]] int l, [[maybe_unused]] int r) const {
    assert(0 <= l && l <= r && r <= n_);
  }

  int count_less(int l, int r, int code) const {
    if (code == static_cast<int>(values_.size())) return r - l;
    int answer = 0;
    for (int h = 0; h < height_; ++h) {
      const auto& level = levels_[h];
      const int ones_l = level.rank(l), ones_r = level.rank(r);
      if ((code >> (height_ - 1 - h)) & 1) {
        answer += (r - l) - (ones_r - ones_l);
        l = level.zeros + ones_l;
        r = level.zeros + ones_r;
      } else {
        l -= ones_l;
        r -= ones_r;
      }
    }
    return answer;
  }

 public:
  WaveletMatrix() = default;

  explicit WaveletMatrix(const std::vector<T>& a) : values_(a) {
    assert(a.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    n_ = static_cast<int>(a.size());
    std::sort(values_.begin(), values_.end());
    values_.erase(std::unique(values_.begin(), values_.end()), values_.end());
    if (values_.empty()) return;
    height_ = std::bit_width(static_cast<unsigned>(values_.size() - 1));
    std::vector<int> current(n_), next(n_);
    for (int i = 0; i < n_; ++i) current[i] = lower(a[i]);
    levels_.resize(height_);
    for (int h = 0; h < height_; ++h) {
      auto& level = levels_[h];
      const int shift = height_ - 1 - h;
      const int blocks = n_ / 64 + 1;
      level.bits.assign(blocks, 0);
      level.prefix.assign(blocks, 0);
      for (int i = 0; i < n_; ++i) {
        if ((current[i] >> shift) & 1) level.bits[i / 64] |= std::uint64_t{1} << (i % 64);
      }
      for (int b = 1; b < blocks; ++b) {
        level.prefix[b] = level.prefix[b - 1] + std::popcount(level.bits[b - 1]);
      }
      level.zeros = n_ - level.rank(n_);
      int zero = 0, one = level.zeros;
      for (int code : current) {
        next[((code >> shift) & 1) ? one++ : zero++] = code;
      }
      current.swap(next);
    }
  }

  int size() const { return n_; }

  T get(int p) const {
    assert(0 <= p && p < n_);
    int code = 0;
    for (const auto& level : levels_) {
      const int bit = static_cast<int>((level.bits[p / 64] >> (p % 64)) & 1);
      code = (code << 1) | bit;
      const int ones = level.rank(p);
      p = bit ? level.zeros + ones : p - ones;
    }
    return values_[code];
  }

  T kth_smallest(int l, int r, int k) const {
    check_range(l, r);
    assert(0 <= k && k < r - l);
    int code = 0;
    for (const auto& level : levels_) {
      const int ones_l = level.rank(l), ones_r = level.rank(r);
      const int zeros = (r - l) - (ones_r - ones_l);
      const bool bit = k >= zeros;
      code = (code << 1) | static_cast<int>(bit);
      if (bit) {
        k -= zeros;
        l = level.zeros + ones_l;
        r = level.zeros + ones_r;
      } else {
        l -= ones_l;
        r -= ones_r;
      }
    }
    return values_[code];
  }

  T kth_largest(int l, int r, int k) const {
    check_range(l, r);
    assert(0 <= k && k < r - l);
    return kth_smallest(l, r, r - l - 1 - k);
  }

  int count(int l, int r, const T& x) const {
    check_range(l, r);
    const int code = lower(x);
    if (code == static_cast<int>(values_.size()) || values_[code] != x) return 0;
    for (int h = 0; h < height_; ++h) {
      const auto& level = levels_[h];
      const int ones_l = level.rank(l), ones_r = level.rank(r);
      if ((code >> (height_ - 1 - h)) & 1) {
        l = level.zeros + ones_l;
        r = level.zeros + ones_r;
      } else {
        l -= ones_l;
        r -= ones_r;
      }
    }
    return r - l;
  }

  int range_freq(int l, int r, const T& upper) const {
    check_range(l, r);
    return count_less(l, r, lower(upper));
  }

  int range_freq(int l, int r, const T& lower_value, const T& upper) const {
    check_range(l, r);
    assert(lower_value <= upper);
    return count_less(l, r, lower(upper)) - count_less(l, r, lower(lower_value));
  }

  std::optional<T> prev_value(int l, int r, const T& upper) const {
    const int k = range_freq(l, r, upper);
    if (k == 0) return std::nullopt;
    return kth_smallest(l, r, k - 1);
  }

  std::optional<T> next_value(int l, int r, const T& lower_value) const {
    const int k = range_freq(l, r, lower_value);
    if (k == r - l) return std::nullopt;
    return kth_smallest(l, r, k);
  }
};

}  // namespace blueberry

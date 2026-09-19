#pragma once
#include <algorithm>
#include <bit>
#include <cassert>
#include <functional>
#include <limits>
#include <utility>
#include <vector>

namespace blueberry {
// Word-RAM O(N) construction/space, O(1) RMQ; ties choose the leftmost index.
// Blocks grow with log N, rather than using a fixed-width sparse-table reduction.
template <class T, class Compare = std::less<T>>
class LinearRMQ {
  std::vector<T> values_;
  Compare compare_;
  int width_ = 1;
  std::vector<unsigned> masks_;
  std::vector<std::vector<int>> table_;
  int best(int a, int b) const {
    if (compare_(values_[a], values_[b])) return a;
    if (compare_(values_[b], values_[a])) return b;
    return std::min(a, b);
  }
  int small(int l, int r) const {
    const int start = (l / width_) * width_;
    return start + std::countr_zero(masks_[r - 1] & (~0u << (l - start)));
  }
 public:
  explicit LinearRMQ(const std::vector<T>& values = {}, Compare compare = Compare{})
      : values_(values), compare_(std::move(compare)), masks_(values.size()) {
    assert(values.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    const int n = size();
    width_ = std::max(1, static_cast<int>(std::bit_width(static_cast<unsigned>(n))) / 2);
    std::vector<int> minima;
    for (int start = 0; start < n;) {
      const int end = start + std::min(width_, n - start);
      unsigned mask = 0;
      for (int i = start; i < end; ++i) {
        while (mask && compare_(values_[i], values_[start + std::bit_width(mask) - 1]))
          mask ^= 1u << (std::bit_width(mask) - 1);
        mask |= 1u << (i - start);
        masks_[i] = mask;
      }
      minima.push_back(start + std::countr_zero(mask));
      start = end;
    }
    if (minima.empty()) return;
    table_.push_back(std::move(minima));
    const int m = static_cast<int>(table_[0].size());
    for (int k = 1; (1u << k) <= static_cast<unsigned>(m); ++k) {
      const int span = static_cast<int>(1u << k), half = span / 2;
      table_.emplace_back(m - span + 1);
      for (int i = 0; i <= m - span; ++i)
        table_[k][i] = best(table_[k - 1][i], table_[k - 1][i + half]);
    }
  }
  int size() const { return static_cast<int>(values_.size()); }
  T get(int p) const { assert(0 <= p && p < size()); return values_[p]; }
  int argmin(int l, int r) const {
    assert(0 <= l && l < r && r <= size());
    const int a = l / width_, b = (r - 1) / width_;
    if (a == b) return small(l, r);
    int answer = best(small(l, (a + 1) * width_), small(b * width_, r));
    if (a + 1 < b) {
      const int k = std::bit_width(static_cast<unsigned>(b - a - 1)) - 1;
      answer = best(answer, best(table_[k][a + 1], table_[k][b - (1 << k)]));
    }
    return answer;
  }
  T prod(int l, int r) const { return values_[argmin(l, r)]; }
};
}  // namespace blueberry

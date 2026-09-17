#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

namespace blueberry {

// Static range aggregation for associative, not necessarily commutative operations.
template <class T, class Op>
class DisjointSparseTable {
 public:
  DisjointSparseTable(const std::vector<T>& values, Op op)
      : values_(values), op_(std::move(op)) {
    assert(values.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    const std::size_t n = values.size();
    const int levels = n < 2 ? 0 : std::bit_width(n - 1);
    table_.reserve(levels);
    for (int level = 0; level < levels; ++level) {
      table_.push_back(values);
      auto& row = table_.back();
      const std::size_t half = std::size_t{1} << level;
      for (std::size_t left = 0; left < n; left += 2 * half) {
        const std::size_t middle = std::min(left + half, n);
        const std::size_t right = std::min(middle + half, n);
        for (std::size_t i = middle - 1; i > left; --i)
          row[i - 1] = op_(values[i - 1], row[i]);
        for (std::size_t i = middle + 1; i < right; ++i)
          row[i] = op_(row[i - 1], values[i]);
      }
    }
  }

  T prod(int left, int right) const {
    assert(0 <= left && left < right && right <= size());
    if (right - left == 1) return values_[left];
    const int level = std::bit_width(static_cast<unsigned>(left ^ (right - 1))) - 1;
    return op_(table_[level][left], table_[level][right - 1]);
  }

  int size() const { return static_cast<int>(values_.size()); }

 private:
  std::vector<T> values_;
  Op op_;
  std::vector<std::vector<T>> table_;
};

}  // namespace blueberry

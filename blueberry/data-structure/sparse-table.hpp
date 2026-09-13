#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

namespace blueberry {

/**
 * @brief Sparse Table
 */
template <class T, class Op>
class SparseTable {
 public:
  SparseTable(const std::vector<T>& values, Op op)
      : n_(static_cast<int>(values.size())), op_(std::move(op)) {
    assert(!values.empty());
    const int levels = std::bit_width(static_cast<unsigned>(n_));
    offsets_.resize(levels + 1);
    for (int level = 0; level < levels; ++level) {
      offsets_[level + 1] = offsets_[level] + n_ - (1 << level) + 1;
    }
    table_.resize(offsets_.back());
    std::copy(values.begin(), values.end(), table_.begin());
    for (int level = 1; level < levels; ++level) {
      const int length = 1 << level;
      for (int left = 0; left + length <= n_; ++left) {
        table_[offsets_[level] + left] =
            op_(table_[offsets_[level - 1] + left],
                table_[offsets_[level - 1] + left + length / 2]);
      }
    }
  }

  T product(int left, int right) const {
    assert(0 <= left && left < right && right <= n_);
    const int level = std::bit_width(static_cast<unsigned>(right - left)) - 1;
    return op_(table_[offsets_[level] + left],
               table_[offsets_[level] + right - (1 << level)]);
  }

  int size() const { return n_; }

 private:
  int n_;
  Op op_;
  std::vector<std::size_t> offsets_;
  std::vector<T> table_;
};

}  // namespace blueberry

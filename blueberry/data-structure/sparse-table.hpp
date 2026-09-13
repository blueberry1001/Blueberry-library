#pragma once

#include <cassert>
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
    log_.assign(n_ + 1, 0);
    for (int i = 2; i <= n_; ++i) log_[i] = log_[i / 2] + 1;
    table_.resize(log_[n_] + 1);
    table_[0] = values;
    for (int level = 1; level < static_cast<int>(table_.size()); ++level) {
      const int length = 1 << level;
      table_[level].resize(n_ - length + 1);
      for (int left = 0; left + length <= n_; ++left) {
        table_[level][left] =
            op_(table_[level - 1][left], table_[level - 1][left + length / 2]);
      }
    }
  }

  // Query a non-empty half-open interval [left, right).
  // `prod` is the short spelling used in new code.  Keep `product` below as
  // a source-compatible spelling for existing submissions.
  T prod(int left, int right) const {
    assert(0 <= left && left < right && right <= n_);
    const int level = log_[right - left];
    return op_(table_[level][left], table_[level][right - (1 << level)]);
  }

  T product(int left, int right) const { return prod(left, right); }

  int size() const { return n_; }

 private:
  int n_;
  Op op_;
  std::vector<int> log_;
  std::vector<std::vector<T>> table_;
};

}  // namespace blueberry

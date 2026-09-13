#pragma once

#include <bit>
#include <cassert>
#include <vector>

namespace blueberry {

/**
 * @brief Fenwick Tree (Binary Indexed Tree)
 */
template <class T>
class FenwickTree {
 public:
  explicit FenwickTree(int n) : n_(n) {
    assert(n >= 0);
    data_.resize(n);
  }

  explicit FenwickTree(const std::vector<T>& values)
      : n_(static_cast<int>(values.size())), data_(values) {
    for (int i = 1; i <= n_; ++i) {
      const int parent = i + (i & -i);
      if (parent <= n_) data_[parent - 1] += data_[i - 1];
    }
  }

  void add(int index, const T& value) {
    assert(0 <= index && index < n_);
    for (int i = index + 1; i <= n_; i += i & -i) data_[i - 1] += value;
  }

  T prefix_sum(int right) const {
    assert(0 <= right && right <= n_);
    T result{};
    for (int i = right; i > 0; i -= i & -i) result += data_[i - 1];
    return result;
  }

  T sum(int left, int right) const {
    assert(0 <= left && left <= right && right <= n_);
    return prefix_sum(right) - prefix_sum(left);
  }

  T get(int index) const {
    assert(0 <= index && index < n_);
    int right = index + 1;
    T result = data_[right - 1];
    const int left = right - (right & -right);
    for (--right; right != left; right -= right & -right) result -= data_[right - 1];
    return result;
  }

  int size() const { return n_; }

  // Returns the smallest r in [0, n] such that prefix_sum(r) >= target.
  // Every added value must be non-negative for this operation.
  int lower_bound(const T& target) const {
    if (!(T{} < target)) return 0;
    int index = 0;
    T current{};
    int step = static_cast<int>(std::bit_floor(static_cast<unsigned>(n_)));
    for (; step > 0; step >>= 1) {
      const int next = index + step;
      if (next <= n_ && current + data_[next - 1] < target) {
        index = next;
        current += data_[next - 1];
      }
    }
    return index == n_ ? n_ : index + 1;
  }

 private:
  int n_;
  std::vector<T> data_;
};

}  // namespace blueberry

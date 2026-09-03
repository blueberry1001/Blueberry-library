#pragma once

#include <cassert>
#include <utility>
#include <vector>

namespace blueberry {

/**
 * @brief Segment Tree
 */
template <class S, class Op>
class SegmentTree {
 public:
  SegmentTree(int n, Op op, S identity)
      : n_(n), op_(std::move(op)), identity_(std::move(identity)) {
    assert(n >= 0);
    initialize();
  }

  SegmentTree(const std::vector<S>& values, Op op, S identity)
      : n_(static_cast<int>(values.size())),
        op_(std::move(op)),
        identity_(std::move(identity)) {
    initialize();
    for (int i = 0; i < n_; ++i) data_[size_ + i] = values[i];
    for (int i = size_ - 1; i > 0; --i) update(i);
  }

  void set(int index, const S& value) {
    assert(0 <= index && index < n_);
    int node = index + size_;
    data_[node] = value;
    while ((node >>= 1) > 0) update(node);
  }

  const S& get(int index) const {
    assert(0 <= index && index < n_);
    return data_[index + size_];
  }

  S product(int left, int right) const {
    assert(0 <= left && left <= right && right <= n_);
    S left_result = identity_;
    S right_result = identity_;
    for (left += size_, right += size_; left < right; left >>= 1, right >>= 1) {
      if (left & 1) left_result = op_(left_result, data_[left++]);
      if (right & 1) right_result = op_(data_[--right], right_result);
    }
    return op_(left_result, right_result);
  }

  const S& all_product() const { return data_[1]; }

  int size() const { return n_; }

 private:
  void initialize() {
    size_ = 1;
    while (size_ < n_) size_ <<= 1;
    data_.assign(size_ * 2, identity_);
  }

  void update(int node) { data_[node] = op_(data_[node * 2], data_[node * 2 + 1]); }

  int n_;
  int size_ = 1;
  Op op_;
  S identity_;
  std::vector<S> data_;
};

}  // namespace blueberry

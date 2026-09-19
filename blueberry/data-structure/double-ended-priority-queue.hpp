#pragma once
#include <bit>
#include <cassert>
#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

namespace blueberry {
// Min-max heap: even-depth nodes are minima, odd-depth nodes are maxima.
template <class T, class Compare = std::less<T>>
class DoubleEndedPriorityQueue {
  std::vector<T> data_;
  Compare compare_;
  static bool min_level(std::size_t i) { return (std::bit_width(i + 1) & 1) != 0; }
  bool better(std::size_t a, std::size_t b, bool low) const {
    return low ? compare_(data_[a], data_[b]) : compare_(data_[b], data_[a]);
  }
  void climb(std::size_t i, bool low) {
    while (i >= 3) {
      std::size_t grandparent = (i - 3) / 4;
      if (!better(i, grandparent, low)) break;
      std::swap(data_[i], data_[grandparent]); i = grandparent;
    }
  }
  void descend(std::size_t i, bool low) {
    while (i < data_.size() / 2) {
      std::size_t best = i;
      for (std::size_t j = 2 * i + 1; j <= 2 * i + 2 && j < data_.size(); ++j)
        if (better(j, best, low)) best = j;
      for (std::size_t j = 4 * i + 3; j <= 4 * i + 6 && j < data_.size(); ++j)
        if (better(j, best, low)) best = j;
      if (best == i) break;
      std::swap(data_[i], data_[best]);
      if (best < 4 * i + 3) break;
      const std::size_t parent = (best - 1) / 2;
      if (better(parent, best, low)) std::swap(data_[parent], data_[best]);
      i = best;
    }
  }
  std::size_t max_index() const {
    return data_.size() == 1 ? 0 : data_.size() == 2 || compare_(data_[2], data_[1]) ? 1 : 2;
  }
  void erase_at(std::size_t i, bool low) {
    if (i + 1 != data_.size()) std::swap(data_[i], data_.back());
    data_.pop_back();
    if (i < data_.size()) descend(i, low);
  }
 public:
  explicit DoubleEndedPriorityQueue(Compare compare = {}) : compare_(std::move(compare)) {}
  explicit DoubleEndedPriorityQueue(const std::vector<T>& values, Compare compare = {})
      : data_(values), compare_(std::move(compare)) {
    for (std::size_t i = data_.size() / 2; i > 0; --i) descend(i - 1, min_level(i - 1));
  }
  DoubleEndedPriorityQueue(const DoubleEndedPriorityQueue&) = default;
  DoubleEndedPriorityQueue& operator=(const DoubleEndedPriorityQueue&) = default;
  DoubleEndedPriorityQueue(DoubleEndedPriorityQueue&& other)
      : data_(std::move(other.data_)), compare_(other.compare_) { other.data_.clear(); }
  DoubleEndedPriorityQueue& operator=(DoubleEndedPriorityQueue&& other) {
    if (this != &other) { compare_ = other.compare_; data_ = std::move(other.data_); other.data_.clear(); }
    return *this;
  }
  std::size_t size() const { return data_.size(); }
  bool empty() const { return data_.empty(); }
  const T& min() const { assert(!empty()); return data_[0]; }
  const T& max() const { assert(!empty()); return data_[max_index()]; }
  void push(const T& value) {
    data_.push_back(value);
    std::size_t i = data_.size() - 1;
    if (i == 0) return;
    const std::size_t parent = (i - 1) / 2;
    bool low = min_level(i);
    if (better(parent, i, low)) {
      std::swap(data_[parent], data_[i]); i = parent; low = !low;
    }
    climb(i, low);
  }
  void pop_min() { assert(!empty()); erase_at(0, true); }
  void pop_max() { assert(!empty()); erase_at(max_index(), false); }
  void clear() { data_.clear(); }
  void reserve(std::size_t capacity) { data_.reserve(capacity); }
};
}  // namespace blueberry

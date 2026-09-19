#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <optional>
#include <queue>
#include <utility>
#include <vector>

namespace blueberry {
// Unit-slope convex functions on the whole real line, initially constant.
template <class T = long long>
class SlopeTrick {
  std::priority_queue<T> left_;
  std::priority_queue<T, std::vector<T>, std::greater<T>> right_;
  T left_shift_ = T(0), right_shift_ = T(0), minimum_ = T(0);
 public:
  explicit SlopeTrick(T constant = T(0)) : minimum_(constant) {}
  SlopeTrick(const SlopeTrick&) = default;
  SlopeTrick& operator=(const SlopeTrick&) = default;
  SlopeTrick(SlopeTrick&& other) : SlopeTrick() { swap(other); }
  SlopeTrick& operator=(SlopeTrick&& other) {
    if (this != &other) { SlopeTrick moved(std::move(other)); swap(moved); }
    return *this;
  }
  std::size_t size() const { return left_.size() + right_.size(); }
  T min() const { return minimum_; }
  std::pair<std::optional<T>, std::optional<T>> argmin() const {
    return {left_.empty() ? std::nullopt : std::optional<T>(left_.top() + left_shift_),
            right_.empty() ? std::nullopt : std::optional<T>(right_.top() + right_shift_)};
  }
  void add_const(const T& value) { minimum_ += value; }
  void add_x_minus_a(const T& a) {
    if (!left_.empty() && a < left_.top() + left_shift_) {
      T x = left_.top() + left_shift_; left_.pop();
      minimum_ += x - a; left_.push(a - left_shift_); right_.push(x - right_shift_);
    } else right_.push(a - right_shift_);
  }
  void add_a_minus_x(const T& a) {
    if (!right_.empty() && right_.top() + right_shift_ < a) {
      T x = right_.top() + right_shift_; right_.pop();
      minimum_ += a - x; right_.push(a - right_shift_); left_.push(x - left_shift_);
    } else left_.push(a - left_shift_);
  }
  void add_abs(const T& a) { add_x_minus_a(a); add_a_minus_x(a); }
  void shift(const T& dx) { shift(dx, dx); }
  void shift(const T& l, const T& r) {
    assert(l <= r); left_shift_ += l; right_shift_ += r;
  }
  void prefix_min() { decltype(right_)().swap(right_); right_shift_ = T(0); }
  void suffix_min() { decltype(left_)().swap(left_); left_shift_ = T(0); }
  T eval(const T& x) const {
    T result = minimum_;
    auto left = left_; auto right = right_;
    while (!left.empty()) { result += std::max(T(0), left.top() + left_shift_ - x); left.pop(); }
    while (!right.empty()) { result += std::max(T(0), x - right.top() - right_shift_); right.pop(); }
    return result;
  }
  void merge(SlopeTrick& other) {
    assert(this != &other);
    if (size() < other.size()) swap(other);
    minimum_ += other.minimum_;
    while (!other.left_.empty()) {
      T x = other.left_.top() + other.left_shift_; other.left_.pop(); add_a_minus_x(x);
    }
    while (!other.right_.empty()) {
      T x = other.right_.top() + other.right_shift_; other.right_.pop(); add_x_minus_a(x);
    }
    other.clear();
  }
  void clear() { decltype(left_)().swap(left_); decltype(right_)().swap(right_); left_shift_ = right_shift_ = minimum_ = T(0); }
 private:
  void swap(SlopeTrick& other) {
    left_.swap(other.left_); right_.swap(other.right_);
    std::swap(left_shift_, other.left_shift_); std::swap(right_shift_, other.right_shift_);
    std::swap(minimum_, other.minimum_);
  }
};
}  // namespace blueberry

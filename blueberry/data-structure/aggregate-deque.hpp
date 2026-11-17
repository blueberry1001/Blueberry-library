#pragma once
#include <cassert>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

namespace blueberry {
template <class S, auto op, auto e>
class AggregateDeque {
  struct Entry { S value, aggregate; };
  std::vector<Entry> left_, right_;
  void rebalance(bool need_left) {
    if (need_left ? !left_.empty() : !right_.empty()) return;
    if constexpr (std::is_trivially_copyable_v<S> && std::is_copy_assignable_v<S> && std::is_move_assignable_v<S>) {
      if (static_cast<std::size_t>(size()) + (static_cast<std::size_t>(size()) + 1) / 2 <= static_cast<std::size_t>(std::numeric_limits<int>::max())) {
      auto& src = need_left ? right_ : left_;
      const auto count = (src.size() + 1) / 2;
      if (need_left) {
        for (auto i = count; i > 0; --i) push_front(src[i - 1].value);
        src.erase(src.begin(), src.begin() + count);
        if (!src.empty()) src[0].aggregate = src[0].value;
        for (std::size_t i = 1; i < src.size(); ++i) src[i].aggregate = op(src[i - 1].aggregate, src[i].value);
      } else {
        for (auto i = count; i > 0; --i) push_back(src[i - 1].value);
        src.erase(src.begin(), src.begin() + count);
        if (!src.empty()) src[0].aggregate = src[0].value;
        for (std::size_t i = 1; i < src.size(); ++i) src[i].aggregate = op(src[i].value, src[i - 1].aggregate);
      }
      return;
      }
    }
    std::vector<S> values;
    values.reserve(size());
    for (auto it = left_.rbegin(); it != left_.rend(); ++it) values.push_back(it->value);
    for (const auto& entry : right_) values.push_back(entry.value);
    left_.clear(); right_.clear();
    const auto split = need_left ? (values.size() + 1) / 2 : values.size() / 2;
    for (auto i = split; i > 0; --i) push_front(values[i - 1]);
    for (auto i = split; i < values.size(); ++i) push_back(values[i]);
  }
 public:
  AggregateDeque() = default;
  AggregateDeque(const AggregateDeque&) = default;
  AggregateDeque& operator=(const AggregateDeque&) = default;
  AggregateDeque(AggregateDeque&& other) noexcept
      : left_(std::move(other.left_)), right_(std::move(other.right_)) {
    other.left_.clear(); other.right_.clear();
  }
  AggregateDeque& operator=(AggregateDeque&& other) noexcept {
    if (this != &other) {
      left_ = std::move(other.left_); right_ = std::move(other.right_);
      other.left_.clear(); other.right_.clear();
    }
    return *this;
  }
  int size() const { return static_cast<int>(left_.size() + right_.size()); }
  bool empty() const { return left_.empty() && right_.empty(); }
  void push_front(const S& value) {
    assert(size() < std::numeric_limits<int>::max());
    left_.push_back({value, left_.empty() ? value : op(value, left_.back().aggregate)});
  }
  void push_back(const S& value) {
    assert(size() < std::numeric_limits<int>::max());
    right_.push_back({value, right_.empty() ? value : op(right_.back().aggregate, value)});
  }
  void pop_front() { assert(!empty()); rebalance(true); left_.pop_back(); }
  void pop_back() { assert(!empty()); rebalance(false); right_.pop_back(); }
  S front() const { assert(!empty()); return left_.empty() ? right_.front().value : left_.back().value; }
  S back() const { assert(!empty()); return right_.empty() ? left_.front().value : right_.back().value; }
  S prod() const {
    if (left_.empty()) return right_.empty() ? e() : right_.back().aggregate;
    if (right_.empty()) return left_.back().aggregate;
    return op(left_.back().aggregate, right_.back().aggregate);
  }
  void clear() { std::vector<Entry>().swap(left_); std::vector<Entry>().swap(right_); }
};
}

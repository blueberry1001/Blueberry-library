#pragma once
#include <cassert>
#include <limits>
#include <utility>
#include <vector>

namespace blueberry {
// Two stacks preserve left-to-right monoid order, including noncommutative op.
template <class S, auto op, auto e>
class AggregateQueue {
  struct Entry { S value, aggregate; };
  std::vector<Entry> input_, output_;
  void transfer() {
    if (!output_.empty()) return;
    while (!input_.empty()) {
      S value = input_.back().value;
      output_.push_back({value, output_.empty() ? value : op(value, output_.back().aggregate)});
      input_.pop_back();
    }
  }
 public:
  AggregateQueue() = default;
  AggregateQueue(const AggregateQueue&) = default;
  AggregateQueue& operator=(const AggregateQueue&) = default;
  AggregateQueue(AggregateQueue&& other) noexcept
      : input_(std::move(other.input_)), output_(std::move(other.output_)) {
    other.input_.clear(); other.output_.clear();
  }
  AggregateQueue& operator=(AggregateQueue&& other) noexcept {
    if (this != &other) {
      input_ = std::move(other.input_); output_ = std::move(other.output_);
      other.input_.clear(); other.output_.clear();
    }
    return *this;
  }
  int size() const { return static_cast<int>(input_.size() + output_.size()); }
  bool empty() const { return input_.empty() && output_.empty(); }
  void push(const S& value) {
    assert(size() < std::numeric_limits<int>::max());
    input_.push_back({value, input_.empty() ? value : op(input_.back().aggregate, value)});
  }
  void pop() { assert(!empty()); transfer(); output_.pop_back(); }
  S front() const { assert(!empty()); return output_.empty() ? input_.front().value : output_.back().value; }
  S back() const { assert(!empty()); return input_.empty() ? output_.front().value : input_.back().value; }
  S prod() const {
    return op(output_.empty() ? e() : output_.back().aggregate,
              input_.empty() ? e() : input_.back().aggregate);
  }
  void clear() { std::vector<Entry>().swap(input_); std::vector<Entry>().swap(output_); }
};
}  // namespace blueberry

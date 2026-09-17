#pragma once

#include <cassert>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

namespace blueberry {

// Sorted occurrences with stable order inside comparator-equivalent classes.
// Aggregates follow in-order traversal; op need not be commutative.
template <class T, auto op, auto e, class Compare = std::less<T>>
class OrderedMultiset {
 public:
  explicit OrderedMultiset(Compare compare = {}, std::uint64_t seed = 0x243f6a8885a308d3ULL)
      : compare_(std::move(compare)), state_(seed) {}
  OrderedMultiset(const OrderedMultiset&) = default;
  OrderedMultiset& operator=(const OrderedMultiset&) = default;
  OrderedMultiset(OrderedMultiset&& other)
      : compare_(other.compare_), nodes_(std::move(other.nodes_)), free_(std::move(other.free_)),
        root_(std::exchange(other.root_, -1)), state_(other.state_) {
    other.nodes_.clear(); other.free_.clear();
  }
  OrderedMultiset& operator=(OrderedMultiset&& other) {
    if (this == &other) return *this;
    compare_ = other.compare_; nodes_ = std::move(other.nodes_); free_ = std::move(other.free_);
    root_ = std::exchange(other.root_, -1); state_ = other.state_;
    other.nodes_.clear(); other.free_.clear();
    return *this;
  }
  int size() const { return count_node(root_); }
  bool empty() const { return root_ == -1; }
  T all_prod() const { return aggregate(root_); }
  void clear() {
    root_ = -1; std::vector<Node>().swap(nodes_); std::vector<int>().swap(free_);
  }
  void insert(const T& value) {
    assert(size() < std::numeric_limits<int>::max());
    root_ = insert_node(root_, value);
  }
  bool erase(const T& value) {
    const int position = rank(value);
    const auto found = kth(position);
    if (!found || compare_(value, *found) || compare_(*found, value)) return false;
    erase_kth(position); return true;
  }
  void erase_kth(int position) {
    assert(0 <= position && position < size());
    root_ = erase_node(root_, position);
  }
  int rank(const T& value) const { return rank_bound(value, false); }
  int count(const T& value) const { return rank_bound(value, true) - rank(value); }
  bool contains(const T& value) const { return count(value) != 0; }
  std::optional<T> kth(int position) const {
    if (position < 0 || position >= size()) return std::nullopt;
    int v = root_;
    for (;;) {
      const int left = count_node(nodes_[v].left);
      if (position == left) return nodes_[v].value;
      if (position < left) v = nodes_[v].left;
      else { position -= left + 1; v = nodes_[v].right; }
    }
  }
  std::optional<T> lower_bound(const T& value) const { return kth(rank(value)); }
  std::optional<T> upper_bound(const T& value) const { return kth(rank_bound(value, true)); }
  std::optional<T> floor(const T& value) const { return kth(rank_bound(value, true) - 1); }
  T prod(int l, int r) const {
    assert(0 <= l && l <= r && r <= size());
    return prod_node(root_, l, r);
  }

 private:
  struct Node {
    T value, aggregate;
    std::uint64_t priority;
    int left = -1, right = -1, size = 1;
    Node(const T& value_, std::uint64_t priority_)
        : value(value_), aggregate(value_), priority(priority_) {}
  };
  int count_node(int v) const { return v == -1 ? 0 : nodes_[v].size; }
  T aggregate(int v) const { return v == -1 ? e() : nodes_[v].aggregate; }
  void pull(int v) {
    nodes_[v].size = 1 + count_node(nodes_[v].left) + count_node(nodes_[v].right);
    nodes_[v].aggregate = op(op(aggregate(nodes_[v].left), nodes_[v].value), aggregate(nodes_[v].right));
  }
  std::uint64_t random() {
    std::uint64_t z = (state_ += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
  }
  int allocate(const T& value) {
    Node node(value, random());
    if (!free_.empty()) {
      const int v = free_.back(); free_.pop_back(); nodes_[v] = std::move(node); return v;
    }
    assert(nodes_.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
    nodes_.push_back(std::move(node)); return static_cast<int>(nodes_.size()) - 1;
  }
  int rotate(int v, bool right) {
    const int u = right ? nodes_[v].left : nodes_[v].right;
    if (right) { nodes_[v].left = nodes_[u].right; nodes_[u].right = v; }
    else { nodes_[v].right = nodes_[u].left; nodes_[u].left = v; }
    pull(v); pull(u); return u;
  }
  int insert_node(int v, const T& value) {
    if (v == -1) return allocate(value);
    const bool left = compare_(value, nodes_[v].value);
    // Save indices, not references: recursive allocation may move the pool.
    const int child = insert_node(left ? nodes_[v].left : nodes_[v].right, value);
    if (left) nodes_[v].left = child; else nodes_[v].right = child;
    if (nodes_[child].priority < nodes_[v].priority) v = rotate(v, left);
    pull(v); return v;
  }
  int merge(int a, int b) {
    if (a == -1) return b;
    if (b == -1) return a;
    if (nodes_[a].priority < nodes_[b].priority) {
      nodes_[a].right = merge(nodes_[a].right, b); pull(a); return a;
    }
    nodes_[b].left = merge(a, nodes_[b].left); pull(b); return b;
  }
  int erase_node(int v, int position) {
    const int left = count_node(nodes_[v].left);
    if (position < left) nodes_[v].left = erase_node(nodes_[v].left, position);
    else if (position > left) nodes_[v].right = erase_node(nodes_[v].right, position - left - 1);
    else {
      const int result = merge(nodes_[v].left, nodes_[v].right);
      nodes_[v] = Node(e(), 0); free_.push_back(v); return result;
    }
    pull(v); return v;
  }
  int rank_bound(const T& value, bool upper) const {
    int answer = 0, v = root_;
    while (v != -1) {
      const bool before = upper ? !compare_(value, nodes_[v].value) : compare_(nodes_[v].value, value);
      if (before) { answer += count_node(nodes_[v].left) + 1; v = nodes_[v].right; }
      else v = nodes_[v].left;
    }
    return answer;
  }
  T prod_node(int v, int l, int r) const {
    if (v == -1 || r <= 0 || l >= count_node(v) || l >= r) return e();
    if (l <= 0 && count_node(v) <= r) return nodes_[v].aggregate;
    const int left = count_node(nodes_[v].left);
    T result = prod_node(nodes_[v].left, l, r);
    if (l <= left && left < r) result = op(result, nodes_[v].value);
    return op(result, prod_node(nodes_[v].right, l - left - 1, r - left - 1));
  }
  Compare compare_;
  std::vector<Node> nodes_;
  std::vector<int> free_;
  int root_ = -1;
  std::uint64_t state_;
};

}  // namespace blueberry

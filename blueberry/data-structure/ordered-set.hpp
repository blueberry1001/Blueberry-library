#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

namespace blueberry {

// Online comparison-based set with order statistics. Expected O(log n).
template <class Key, class Compare = std::less<Key>>
class OrderedSet {
 public:
  explicit OrderedSet(Compare compare = {}) : comp_(std::move(compare)) {}
  OrderedSet(const OrderedSet&) = default;
  OrderedSet& operator=(const OrderedSet&) = default;
  OrderedSet(OrderedSet&& other)
      : comp_(other.comp_), nodes_(std::move(other.nodes_)), free_(std::move(other.free_)),
        root_(std::exchange(other.root_, -1)), state_(other.state_) {
    other.nodes_.clear();
    other.free_.clear();
  }
  OrderedSet& operator=(OrderedSet&& other) {
    if (this == &other) return *this;
    comp_ = other.comp_;
    nodes_ = std::move(other.nodes_);
    free_ = std::move(other.free_);
    root_ = std::exchange(other.root_, -1);
    state_ = other.state_;
    other.nodes_.clear();
    other.free_.clear();
    return *this;
  }

  int size() const { return count(root_); }
  bool empty() const { return root_ == -1; }
  bool contains(const Key& key) const {
    int v = root_;
    while (v != -1) {
      if (comp_(key, nodes_[v].key)) v = nodes_[v].left;
      else if (comp_(nodes_[v].key, key)) v = nodes_[v].right;
      else return true;
    }
    return false;
  }
  bool insert(const Key& key) {
    bool added = false;
    root_ = insert_node(root_, key, added);
    return added;
  }
  bool erase(const Key& key) {
    bool removed = false;
    root_ = erase_node(root_, key, removed);
    return removed;
  }
  int rank(const Key& key) const {
    int answer = 0, v = root_;
    while (v != -1) {
      if (comp_(nodes_[v].key, key)) {
        answer += count(nodes_[v].left) + 1;
        v = nodes_[v].right;
      } else v = nodes_[v].left;
    }
    return answer;
  }
  std::optional<Key> kth(int k) const {
    if (k < 0 || k >= size()) return std::nullopt;
    int v = root_;
    while (true) {
      int left = count(nodes_[v].left);
      if (k == left) return nodes_[v].key;
      if (k < left) v = nodes_[v].left;
      else { k -= left + 1; v = nodes_[v].right; }
    }
  }
  std::optional<Key> lower_bound(const Key& key) const { return bound(key, false); }
  std::optional<Key> upper_bound(const Key& key) const { return bound(key, true); }
  std::optional<Key> floor(const Key& key) const {
    int v = root_, answer = -1;
    while (v != -1) {
      if (!comp_(key, nodes_[v].key)) { answer = v; v = nodes_[v].right; }
      else v = nodes_[v].left;
    }
    return answer == -1 ? std::nullopt : std::optional<Key>(nodes_[answer].key);
  }

 private:
  struct Node {
    Key key;
    std::uint64_t priority;
    int left = -1, right = -1, size = 1;
  };
  int count(int v) const { return v == -1 ? 0 : nodes_[v].size; }
  void pull(int v) { nodes_[v].size = 1 + count(nodes_[v].left) + count(nodes_[v].right); }
  std::uint64_t random() {
    std::uint64_t z = (state_ += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
  }
  int allocate(const Key& key) {
    Node node{key, random()};
    if (free_.empty()) {
      nodes_.push_back(std::move(node));
      return static_cast<int>(nodes_.size()) - 1;
    }
    int v = free_.back();
    free_.pop_back();
    nodes_[v] = std::move(node);
    return v;
  }
  int rotate(int v, bool right) {
    int u = right ? nodes_[v].left : nodes_[v].right;
    if (right) { nodes_[v].left = nodes_[u].right; nodes_[u].right = v; }
    else { nodes_[v].right = nodes_[u].left; nodes_[u].left = v; }
    pull(v); pull(u);
    return u;
  }
  int insert_node(int v, const Key& key, bool& added) {
    if (v == -1) { added = true; return allocate(key); }
    if (comp_(key, nodes_[v].key)) {
      // Allocation may move the pool: never keep a Node reference here.
      int u = insert_node(nodes_[v].left, key, added);
      nodes_[v].left = u;
      if (nodes_[u].priority < nodes_[v].priority) v = rotate(v, true);
    } else if (comp_(nodes_[v].key, key)) {
      int u = insert_node(nodes_[v].right, key, added);
      nodes_[v].right = u;
      if (nodes_[u].priority < nodes_[v].priority) v = rotate(v, false);
    }
    pull(v);
    return v;
  }
  int join(int a, int b) {
    if (a == -1) return b;
    if (b == -1) return a;
    if (nodes_[a].priority < nodes_[b].priority) {
      nodes_[a].right = join(nodes_[a].right, b); pull(a); return a;
    }
    nodes_[b].left = join(a, nodes_[b].left); pull(b); return b;
  }
  int erase_node(int v, const Key& key, bool& removed) {
    if (v == -1) return -1;
    if (comp_(key, nodes_[v].key)) nodes_[v].left = erase_node(nodes_[v].left, key, removed);
    else if (comp_(nodes_[v].key, key)) nodes_[v].right = erase_node(nodes_[v].right, key, removed);
    else {
      removed = true;
      int result = join(nodes_[v].left, nodes_[v].right);
      free_.push_back(v);
      return result;
    }
    pull(v);
    return v;
  }
  std::optional<Key> bound(const Key& key, bool strict) const {
    int v = root_, answer = -1;
    while (v != -1) {
      bool skip = strict ? !comp_(key, nodes_[v].key) : comp_(nodes_[v].key, key);
      if (skip) v = nodes_[v].right;
      else { answer = v; v = nodes_[v].left; }
    }
    return answer == -1 ? std::nullopt : std::optional<Key>(nodes_[answer].key);
  }
  Compare comp_;
  std::vector<Node> nodes_;
  std::vector<int> free_;
  int root_ = -1;
  std::uint64_t state_ = 0x243f6a8885a308d3ULL;
};

}  // namespace blueberry

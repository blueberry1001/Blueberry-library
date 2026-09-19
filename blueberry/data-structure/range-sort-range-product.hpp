#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <limits>
#include <iterator>
#include <map>
#include <type_traits>
#include <utility>
#include <vector>

#include <atcoder/segtree>

namespace blueberry {

// Sorted blocks are Patricia tries; only branching bits occupy nodes.
// Keys of distinct live elements must be distinct.
template <class S, auto op, auto e, class Key = unsigned>
class RangeSortRangeProduct {
  static_assert(std::is_unsigned_v<Key> && !std::is_same_v<Key, bool>);
  struct Node {
    int child[2] = {-1, -1};
    int bit = -1, count = 1;
    Key key = 0;
    S forward = e(), backward = e();
  };
  struct Block { int root; bool reversed; };
  int n_ = 0;
  std::vector<Node> nodes_;
  std::vector<int> free_;
  std::map<int, Block> blocks_;
  atcoder::segtree<S, op, e> products_;

  static int checked_size(std::size_t n) {
    assert(n <= static_cast<std::size_t>(std::numeric_limits<int>::max() / 2));
    return static_cast<int>(n);
  }

  int allocate(Node node) {
    if (!free_.empty()) {
      const int id = free_.back();
      free_.pop_back();
      nodes_[id] = std::move(node);
      return id;
    }
    assert(nodes_.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
    nodes_.push_back(std::move(node));
    return static_cast<int>(nodes_.size()) - 1;
  }
  void release(int id) { free_.push_back(id); }
  void pull(int id) {
    auto& node = nodes_[id];
    const auto& a = nodes_[node.child[0]];
    const auto& b = nodes_[node.child[1]];
    node.count = a.count + b.count;
    node.key = a.key;
    node.forward = op(a.forward, b.forward);
    node.backward = op(b.backward, a.backward);
  }
  int branch(int a, int b, int bit) {
    Node node;
    node.bit = bit;
    node.child[0] = a;
    node.child[1] = b;
    const int id = allocate(std::move(node));
    pull(id);
    return id;
  }
  int unite(int a, int b) {
    const Key difference = nodes_[a].key ^ nodes_[b].key;
    const int differing_bit = difference ? int(std::bit_width(difference)) - 1 : -1;
    const int top = std::max(nodes_[a].bit, nodes_[b].bit);
    if (differing_bit > top) {
      if ((nodes_[a].key >> differing_bit) & Key{1}) std::swap(a, b);
      return branch(a, b, differing_bit);
    }
    assert(top >= 0);  // Duplicate live keys violate the precondition.
    if (nodes_[a].bit < nodes_[b].bit) std::swap(a, b);
    if (nodes_[a].bit == nodes_[b].bit) {
      const int left = unite(nodes_[a].child[0], nodes_[b].child[0]);
      const int right = unite(nodes_[a].child[1], nodes_[b].child[1]);
      nodes_[a].child[0] = left;
      nodes_[a].child[1] = right;
      release(b);
    } else {
      const int side = int((nodes_[b].key >> nodes_[a].bit) & Key{1});
      const int child = unite(nodes_[a].child[side], b);
      nodes_[a].child[side] = child;
    }
    pull(a);
    return a;
  }
  std::pair<int, int> split(int id, int count) {
    if (count == 0) return {-1, id};
    if (count == nodes_[id].count) return {id, -1};
    const int a = nodes_[id].child[0], b = nodes_[id].child[1];
    const int left_count = nodes_[a].count;
    if (count == left_count) {
      release(id);
      return {a, b};
    }
    if (count < left_count) {
      auto [x, y] = split(a, count);
      nodes_[id].child[0] = y;
      pull(id);
      return {x, id};
    }
    auto [x, y] = split(b, count - left_count);
    nodes_[id].child[1] = x;
    pull(id);
    return {id, y};
  }
  S aggregate(Block block) const {
    return block.reversed ? nodes_[block.root].backward : nodes_[block.root].forward;
  }
  auto boundary(int p) {
    if (p == n_) return blocks_.end();
    auto it = std::prev(blocks_.upper_bound(p));
    if (it->first == p) return it;
    const Block block = it->second;
    const int count = p - it->first;
    auto [a, b] = split(block.root, block.reversed ? nodes_[block.root].count - count : count);
    if (block.reversed) std::swap(a, b);
    it->second.root = a;
    products_.set(it->first, aggregate(it->second));
    auto inserted = blocks_.emplace(p, Block{b, block.reversed}).first;
    products_.set(p, aggregate(inserted->second));
    return inserted;
  }

 public:
  RangeSortRangeProduct() = default;
  explicit RangeSortRangeProduct(const std::vector<std::pair<Key, S>>& values)
      : products_(checked_size(values.size())) {
    n_ = static_cast<int>(values.size());
#ifndef NDEBUG
    std::vector<Key> keys;
    for (const auto& value : values) keys.push_back(value.first);
    std::sort(keys.begin(), keys.end());
    assert(std::adjacent_find(keys.begin(), keys.end()) == keys.end());
#endif
    nodes_.reserve(values.size() * 2);
    for (int i = 0; i < n_; ++i) {
      Node node;
      node.key = values[i].first;
      node.forward = node.backward = values[i].second;
      blocks_.emplace_hint(blocks_.end(), i, Block{allocate(std::move(node)), false});
      products_.set(i, values[i].second);
    }
  }
  int size() const { return n_; }
  std::pair<Key, S> get(int p) const {
    assert(0 <= p && p < n_);
    auto it = std::prev(blocks_.upper_bound(p));
    int k = p - it->first, id = it->second.root;
    if (it->second.reversed) k = nodes_[id].count - 1 - k;
    while (nodes_[id].bit >= 0) {
      const int count = nodes_[nodes_[id].child[0]].count;
      const int side = k >= count;
      if (side) k -= count;
      id = nodes_[id].child[side];
    }
    return {nodes_[id].key, nodes_[id].forward};
  }
  void set(int p, Key key, const S& value) {
    assert(0 <= p && p < n_);
    boundary(p + 1);
    const int id = boundary(p)->second.root;
    nodes_[id].key = key;
    nodes_[id].forward = nodes_[id].backward = value;
    products_.set(p, value);
  }
  S prod(int l, int r) {
    assert(0 <= l && l <= r && r <= n_);
    if (l == r) return e();
    boundary(l);
    boundary(r);
    return products_.prod(l, r);
  }
  S all_prod() const { return products_.all_prod(); }
  void sort(int l, int r, bool descending = false) {
    assert(0 <= l && l <= r && r <= n_);
    if (l == r) return;
    const auto end = boundary(r);
    auto first = boundary(l), it = std::next(first);
    int root = first->second.root;
    while (it != end) {
      root = unite(root, it->second.root);
      products_.set(it->first, e());
      it = blocks_.erase(it);
    }
    first->second = {root, descending};
    products_.set(l, aggregate(first->second));
  }
};

}  // namespace blueberry

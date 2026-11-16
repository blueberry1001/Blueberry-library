#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace blueberry {

template <class T, class Coord = long long>
class DynamicFenwickTree2D {
  static_assert(std::is_integral_v<Coord> && std::is_signed_v<Coord>);

 public:
  DynamicFenwickTree2D() = default;
  DynamicFenwickTree2D(Coord n, Coord m) : n_(n), m_(m) { assert(n >= 0 && m >= 0); }
  DynamicFenwickTree2D(const DynamicFenwickTree2D&) = default;
  DynamicFenwickTree2D& operator=(const DynamicFenwickTree2D&) = default;
  DynamicFenwickTree2D(DynamicFenwickTree2D&& other)
      : n_(std::exchange(other.n_, 0)), m_(std::exchange(other.m_, 0)),
        roots_(std::move(other.roots_)), nodes_(std::move(other.nodes_)) {
    other.roots_.clear(); other.nodes_.clear();
  }
  DynamicFenwickTree2D& operator=(DynamicFenwickTree2D&& other) {
    if (this == &other) return *this;
    roots_ = std::move(other.roots_); nodes_ = std::move(other.nodes_);
    n_ = std::exchange(other.n_, 0); m_ = std::exchange(other.m_, 0);
    other.roots_.clear(); other.nodes_.clear();
    return *this;
  }

  Coord height() const { return n_; }
  Coord width() const { return m_; }
  void add(Coord x, Coord y, const T& value) {
    assert(0 <= x && x < n_ && 0 <= y && y < m_);
    for (Coord i = x + 1;;) {
      auto [it, inserted] = roots_.try_emplace(i, -1);
      (void)inserted;
      it->second = add_node(it->second, y, value);
      const Coord step = i & -i;
      if (step > n_ - i) break;
      i += step;
    }
  }
  T pref(Coord x, Coord y) const {
    assert(0 <= x && x <= n_ && 0 <= y && y <= m_);
    T answer{};
    for (; x > 0; x -= x & -x) {
      auto it = roots_.find(x);
      if (it != roots_.end()) answer += prefix_node(it->second, y);
    }
    return answer;
  }
  T sum(Coord left, Coord down, Coord right, Coord up) const {
    assert(0 <= left && left <= right && right <= n_);
    assert(0 <= down && down <= up && up <= m_);
    if (left == right || down == up) return T{};
    if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
      using U = std::make_unsigned_t<T>;
      U a = 0, b = 0;
      while (left != right) {
        if (left < right) {
          auto it = roots_.find(right);
          if (it != roots_.end()) a += range_node<U>(it->second, down, up);
          right -= right & -right;
        } else {
          auto it = roots_.find(left);
          if (it != roots_.end()) b += range_node<U>(it->second, down, up);
          left -= left & -left;
        }
      }
      return static_cast<T>(a - b);
    } else return pref(right, up) - pref(left, up) - pref(right, down) + pref(left, down);
  }
  T get(Coord x, Coord y) const {
    assert(0 <= x && x < n_ && 0 <= y && y < m_);
    return sum(x, y, x + 1, y + 1);
  }

 private:
  struct Node {
    Coord key;
    T value, sum;
    int left = -1, right = -1, height = 1;
  };
  int node_height(int v) const { return v == -1 ? 0 : nodes_[v].height; }
  T node_sum(int v) const { return v == -1 ? T{} : nodes_[v].sum; }
  void pull(int v) {
    auto& node = nodes_[v];
    node.height = 1 + std::max(node_height(node.left), node_height(node.right));
    node.sum = node_sum(node.left) + node.value + node_sum(node.right);
  }
  int rotate(int v, bool right) {
    int u = right ? nodes_[v].left : nodes_[v].right;
    if (right) { nodes_[v].left = nodes_[u].right; nodes_[u].right = v; }
    else { nodes_[v].right = nodes_[u].left; nodes_[u].left = v; }
    pull(v); pull(u);
    return u;
  }
  int add_node(int v, Coord key, const T& value) {
    if (v == -1) {
      assert(nodes_.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
      nodes_.push_back({key, value, value});
      return static_cast<int>(nodes_.size()) - 1;
    }
    if (key < nodes_[v].key) {
      int child = add_node(nodes_[v].left, key, value);
      nodes_[v].left = child;
    } else if (nodes_[v].key < key) {
      int child = add_node(nodes_[v].right, key, value);
      nodes_[v].right = child;
    } else nodes_[v].value += value;
    pull(v);
    int balance = node_height(nodes_[v].left) - node_height(nodes_[v].right);
    if (balance > 1) {
      int child = nodes_[v].left;
      if (node_height(nodes_[child].left) < node_height(nodes_[child].right))
        nodes_[v].left = rotate(child, false);
      return rotate(v, true);
    }
    if (balance < -1) {
      int child = nodes_[v].right;
      if (node_height(nodes_[child].right) < node_height(nodes_[child].left))
        nodes_[v].right = rotate(child, true);
      return rotate(v, false);
    }
    return v;
  }
  template <class U>
  U range_node(int v, Coord low, Coord high) const {
    if (v != -1 && low == 0 && high == m_) return static_cast<U>(nodes_[v].sum);
    while (v != -1 && (nodes_[v].key < low || nodes_[v].key >= high))
      v = nodes_[v].key < low ? nodes_[v].right : nodes_[v].left;
    if (v == -1) return U{};
    U answer = static_cast<U>(nodes_[v].value);
    int left = nodes_[v].left, right = nodes_[v].right;
    while (left != -1) {
      const auto& node = nodes_[left];
      if (node.key >= low) {
        answer += static_cast<U>(node.value);
        if (node.right != -1) answer += static_cast<U>(nodes_[node.right].sum);
        left = node.left;
      } else left = node.right;
    }
    while (right != -1) {
      const auto& node = nodes_[right];
      if (node.key < high) {
        answer += static_cast<U>(node.value);
        if (node.left != -1) answer += static_cast<U>(nodes_[node.left].sum);
        right = node.right;
      } else right = node.left;
    }
    return answer;
  }
  T prefix_node(int v, Coord key) const {
    T answer{};
    while (v != -1) {
      const auto& node = nodes_[v];
      if (node.key < key) {
        answer += node_sum(node.left); answer += node.value; v = node.right;
      } else v = node.left;
    }
    return answer;
  }
  Coord n_ = 0, m_ = 0;
  std::unordered_map<Coord, int> roots_;
  std::vector<Node> nodes_;
};

}

#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace blueberry {

/**
 * @brief Dynamic Li Chao Tree
 */
template <class T = long long>
class DynamicLiChaoTree {
  static_assert(std::is_integral_v<T> && std::is_signed_v<T>);

 public:
  DynamicLiChaoTree(T low, T high) : low_(low), high_(high) {
    assert(low <= high);
  }

  DynamicLiChaoTree(const DynamicLiChaoTree&) = default;
  DynamicLiChaoTree& operator=(const DynamicLiChaoTree&) = default;

  DynamicLiChaoTree(DynamicLiChaoTree&& other) noexcept
      : low_(other.low_), high_(other.high_), nodes_(std::move(other.nodes_)),
        root_(std::exchange(other.root_, -1)) {
    other.nodes_.clear();
  }

  DynamicLiChaoTree& operator=(DynamicLiChaoTree&& other) noexcept {
    if (this != &other) {
      low_ = other.low_;
      high_ = other.high_;
      nodes_ = std::move(other.nodes_);
      root_ = std::exchange(other.root_, -1);
      other.nodes_.clear();
    }
    return *this;
  }

  void add_line(T a, T b) {
    if (low_ < high_) root_ = insert(root_, low_, high_, {a, b});
  }

  void add_segment(T l, T r, T a, T b) {
    assert(l <= r);
    l = std::max(l, low_);
    r = std::min(r, high_);
    if (l < r) root_ = segment(root_, low_, high_, l, r, {a, b});
  }

  std::optional<T> query(T x) const {
    assert(low_ <= x && x < high_);
    std::optional<T> answer;
    int node = root_;
    T l = low_, r = high_;
    while (node != -1) {
      const auto& current = nodes_[node];
      if (current.line) {
        const T value = current.line->eval(x);
        if (!answer || value < *answer) answer = value;
      }
      if (l == r - 1) break;
      const T m = midpoint(l, r);
      if (x < m) {
        node = current.left;
        r = m;
      } else {
        node = current.right;
        l = m;
      }
    }
    return answer;
  }

 private:
  struct Line {
    T a, b;
    T eval(T x) const { return a * x + b; }
  };
  struct Node {
    std::optional<Line> line;
    int left = -1, right = -1;
  };

  static T midpoint(T l, T r) {
    // l < r: the unsigned difference holds even a full signed-domain width.
    using U = std::make_unsigned_t<T>;
    const U width = static_cast<U>(r) - static_cast<U>(l);
    return l + static_cast<T>(width / 2);
  }

  int create(std::optional<Line> line = std::nullopt) {
    assert(nodes_.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
    const int index = static_cast<int>(nodes_.size());
    nodes_.push_back({line, -1, -1});
    return index;
  }

  int insert(int node, T l, T r, Line line) {
    if (node == -1) return create(line);
    const int root = node;
    while (true) {
      auto& current = nodes_[node].line;
      if (!current) {
        current = line;
        return root;
      }
      const bool better_left = line.eval(l) < current->eval(l);
      const bool better_right = line.eval(r - 1) < current->eval(r - 1);
      // Affine functions cross at most once; no child is needed for dominance.
      if (better_left == better_right) {
        if (better_left) current = line;
        return root;
      }
      const T m = midpoint(l, r);
      const bool better_middle = line.eval(m) < current->eval(m);
      if (better_middle) std::swap(line, *current);
      const bool go_left = better_left != better_middle;
      const int child = go_left ? nodes_[node].left : nodes_[node].right;
      if (child == -1) {
        const int added = create(line);
        // create() can reallocate: look up the parent by index afterwards.
        if (go_left) nodes_[node].left = added;
        else nodes_[node].right = added;
        return root;
      }
      node = child;
      if (go_left) r = m;
      else l = m;
    }
  }

  int segment(int node, T l, T r, T begin, T end, Line line) {
    if (end <= l || r <= begin) return node;
    if (begin <= l && r <= end) return insert(node, l, r, line);
    if (node == -1) node = create();
    const T m = midpoint(l, r);
    const int left = segment(nodes_[node].left, l, m, begin, end, line);
    nodes_[node].left = left;
    const int right = segment(nodes_[node].right, m, r, begin, end, line);
    nodes_[node].right = right;
    return node;
  }

  T low_, high_;
  std::vector<Node> nodes_;
  int root_ = -1;
};

}  // namespace blueberry

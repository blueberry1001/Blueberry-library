#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

namespace blueberry {
// General convex piecewise-linear functions on a finite closed domain.
// Each splay node is (breakpoint, positive change in slope).
template <class T = long long>
class SplaySlopeTrick {
  struct Node {
    T x, weight, sum, moment, lazy = T(0);
    Node* left = nullptr; Node* right = nullptr; Node* parent = nullptr;
    std::size_t count = 1;
    Node(const T& position, const T& change) : x(position), weight(change), sum(change), moment(position * change) {}
  };
  Node* root_ = nullptr;
  T left_ = T(0), right_ = T(0), slope_ = T(0), value_ = T(0);
  bool empty_ = true;
  static T sum(Node* node) { return node ? node->sum : T(0); }
  static T moment(Node* node) { return node ? node->moment : T(0); }
  static std::size_t count(Node* node) { return node ? node->count : 0; }
  static void pull(Node* node) {
    node->sum = sum(node->left) + node->weight + sum(node->right);
    node->moment = moment(node->left) + node->x * node->weight + moment(node->right);
    node->count = 1 + count(node->left) + count(node->right);
  }
  static void translate(Node* node, const T& dx) {
    if (node) { node->x += dx; node->moment += dx * node->sum; node->lazy += dx; }
  }
  static void push(Node* node) {
    if (node->lazy == T(0)) return;
    translate(node->left, node->lazy); translate(node->right, node->lazy); node->lazy = T(0);
  }
  void rotate(Node* node) {
    Node* parent = node->parent; Node* grandparent = parent->parent;
    if (parent->left == node) {
      parent->left = node->right;
      if (node->right) node->right->parent = parent;
      node->right = parent;
    } else {
      parent->right = node->left;
      if (node->left) node->left->parent = parent;
      node->left = parent;
    }
    parent->parent = node; node->parent = grandparent;
    if (!grandparent) root_ = node;
    else if (grandparent->left == parent) grandparent->left = node;
    else grandparent->right = node;
    pull(parent); pull(node);
  }
  // Search paths are pushed top-down before splaying their nodes.
  void splay(Node* node) {
    while (node->parent) {
      Node* parent = node->parent; Node* grandparent = parent->parent;
      if (grandparent)
        rotate((grandparent->left == parent) == (parent->left == node) ? parent : node);
      rotate(node);
    }
  }
  void add_break(const T& x, const T& weight) {
    if (weight == T(0)) return;
    assert(weight > T(0));
    if (!root_) { root_ = new Node(x, weight); return; }
    Node* node = root_;
    while (true) {
      push(node);
      if (node->x == x) { splay(node); node->weight += weight; pull(node); return; }
      Node*& child = x < node->x ? node->left : node->right;
      if (!child) { child = new Node(x, weight); child->parent = node; splay(child); return; }
      node = child;
    }
  }
  std::pair<T, T> prefix(const T& x) {
    T mass = T(0), weighted = T(0);
    Node* node = root_; Node* last = nullptr;
    while (node) {
      push(node); last = node;
      if (x < node->x) node = node->left;
      else {
        mass += sum(node->left) + node->weight;
        weighted += moment(node->left) + node->x * node->weight;
        node = node->right;
      }
    }
    if (last) splay(last);
    return {mass, weighted};
  }
  Node* at_mass(const T& target) {
    Node* node = root_;
    T before = T(0);
    while (node) {
      push(node);
      if (before + sum(node->left) >= target) node = node->left;
      else if (before + sum(node->left) + node->weight >= target) { splay(node); return node; }
      else { before += sum(node->left) + node->weight; node = node->right; }
    }
    return nullptr;
  }
  std::pair<Node*, Node*> split(const T& x, bool inclusive) {
    Node* node = root_; Node* selected = nullptr; Node* last = nullptr;
    while (node) {
      push(node); last = node;
      if (node->x < x || (inclusive && node->x == x)) { selected = node; node = node->right; }
      else node = node->left;
    }
    if (!selected) { if (last) splay(last); return {nullptr, root_}; }
    splay(selected);
    Node* right = root_->right; root_->right = nullptr;
    if (right) right->parent = nullptr;
    pull(root_); return {root_, right};
  }
  void join(Node* left, Node* right) {
    root_ = left;
    if (!left) { root_ = right; return; }
    Node* node = left;
    while (true) { push(node); if (!node->right) break; node = node->right; }
    splay(node); root_->right = right;
    if (right) right->parent = root_;
    pull(root_);
  }
  static void destroy(Node* node) {
    while (node) {
      if (node->left) {
        Node* child = node->left; node->left = child->right; child->right = node; node = child;
      } else { Node* next = node->right; delete node; node = next; }
    }
  }
  std::vector<std::pair<T, T>> breaks() {
    std::vector<std::pair<T, T>> result;
    result.reserve(size()); std::vector<Node*> stack; Node* node = root_;
    while (node || !stack.empty()) {
      while (node) { push(node); stack.push_back(node); node = node->left; }
      node = stack.back(); stack.pop_back(); result.emplace_back(node->x, node->weight); node = node->right;
    }
    return result;
  }
  std::vector<std::pair<T, T>> segments() {
    auto points = breaks();
    std::vector<std::pair<T, T>> result;
    T x = left_, slope = slope_;
    for (auto [position, weight] : points) { result.emplace_back(position - x, slope); x = position; slope += weight; }
    if (x < right_) result.emplace_back(right_ - x, slope);
    return result;
  }
  void swap(SplaySlopeTrick& other) {
    std::swap(root_, other.root_); std::swap(left_, other.left_); std::swap(right_, other.right_);
    std::swap(slope_, other.slope_); std::swap(value_, other.value_); std::swap(empty_, other.empty_);
  }
 public:
  SplaySlopeTrick() = default;
  SplaySlopeTrick(const T& l, const T& r, const T& a = T(0), const T& b = T(0))
      : left_(l), right_(r), slope_(a), value_(a * l + b), empty_(false) { assert(l <= r); }
  SplaySlopeTrick(const SplaySlopeTrick&) = delete;
  SplaySlopeTrick& operator=(const SplaySlopeTrick&) = delete;
  SplaySlopeTrick(SplaySlopeTrick&& other) : SplaySlopeTrick() { swap(other); }
  SplaySlopeTrick& operator=(SplaySlopeTrick&& other) {
    if (this != &other) { SplaySlopeTrick moved(std::move(other)); swap(moved); }
    return *this;
  }
  ~SplaySlopeTrick() { destroy(root_); }
  static SplaySlopeTrick from_points(const std::vector<std::pair<T, T>>& points) {
    if (points.empty()) return {};
    SplaySlopeTrick result(points.front().first, points.back().first, T(0), points.front().second);
    T previous = T(0);
    for (std::size_t i = 1; i < points.size(); ++i) {
      T dx = points[i].first - points[i - 1].first, dy = points[i].second - points[i - 1].second;
      assert(dx > T(0));
      T slope = dy / dx;
      assert(slope * dx == dy); // Integral T requires an exact quotient.
      if (i == 1) result.slope_ = slope;
      else { assert(previous <= slope); result.add_break(points[i - 1].first, slope - previous); }
      previous = slope;
    }
    return result;
  }
  bool empty() const { return empty_; }
  std::size_t size() const { return count(root_); }
  std::optional<std::pair<T, T>> domain() const {
    if (empty_) return std::nullopt;
    return std::pair<T, T>{left_, right_};
  }
  std::optional<T> eval(const T& x) {
    if (empty_ || x < left_ || right_ < x) return std::nullopt;
    auto [mass, weighted] = prefix(x);
    return value_ + slope_ * (x - left_) + mass * x - weighted;
  }
  std::optional<std::pair<T, T>> argmin() {
    if (empty_) return std::nullopt;
    if (slope_ > T(0) || left_ == right_) return std::pair<T, T>{left_, left_};
    if (slope_ + sum(root_) < T(0)) return std::pair<T, T>{right_, right_};
    if (slope_ == T(0)) {
      if (!root_) return std::pair<T, T>{left_, right_};
      Node* node = root_;
      while (true) { push(node); if (!node->left) break; node = node->left; }
      splay(node); return std::pair<T, T>{left_, node->x};
    }
    Node* node = at_mass(-slope_);
    T x = node->x;
    if (slope_ + sum(node->left) + node->weight > T(0)) return std::pair<T, T>{x, x};
    node = node->right;
    if (!node) return std::pair<T, T>{x, right_};
    while (true) { push(node); if (!node->left) break; node = node->left; }
    T y = node->x; splay(node); return std::pair<T, T>{x, y};
  }
  std::optional<T> min() { auto where = argmin(); return where ? eval(where->first) : std::nullopt; }
  void add_const(const T& value) { if (!empty_) value_ += value; }
  void add_linear(const T& a, const T& b) { if (!empty_) { value_ += a * left_ + b; slope_ += a; } }
  void add_x_minus_a(const T& a, const T& weight = T(1)) {
    assert(weight >= T(0));
    if (empty_ || weight == T(0) || a >= right_) return;
    if (a <= left_) add_linear(weight, -weight * a);
    else add_break(a, weight);
  }
  void add_a_minus_x(const T& a, const T& weight = T(1)) {
    assert(weight >= T(0)); add_linear(-weight, weight * a); add_x_minus_a(a, weight);
  }
  void add_abs(const T& a, const T& weight = T(1)) { add_a_minus_x(a, weight); add_x_minus_a(a, weight); }
  void shift(const T& dx, const T& dy = T(0)) {
    if (!empty_) { left_ += dx; right_ += dx; value_ += dy; translate(root_, dx); }
  }
  void restrict(const T& l, const T& r) {
    if (empty_) return;
    T left = std::max(left_, l), right = std::min(right_, r);
    if (right < left) { clear(); return; }
    T value = *eval(left), slope = slope_ + prefix(left).first;
    auto [discard_left, keep] = split(left, true); destroy(discard_left); root_ = keep;
    auto [middle, discard_right] = split(right, false); destroy(discard_right); root_ = middle;
    left_ = left; right_ = right; value_ = value; slope_ = slope;
  }
  void merge(SplaySlopeTrick& other) {
    assert(this != &other);
    if (empty_ || other.empty_) { clear(); other.clear(); return; }
    T l = std::max(left_, other.left_), r = std::min(right_, other.right_);
    restrict(l, r); other.restrict(l, r);
    if (!empty_) {
      value_ += other.value_; slope_ += other.slope_;
      for (auto [x, weight] : other.breaks()) add_break(x, weight);
    }
    other.clear();
  }
  void convolve_segment(const T& l, const T& r, const T& a, const T& b) {
    assert(l <= r);
    if (empty_) return;
    shift(l, a * l + b);
    T length = r - l;
    if (length == T(0)) return;
    if (left_ == right_) { right_ += length; slope_ = a; return; }
    if (a <= slope_) {
      translate(root_, length); add_break(left_ + length, slope_ - a); slope_ = a;
    } else if (a >= slope_ + sum(root_)) {
      add_break(right_, a - slope_ - sum(root_));
    } else {
      Node* node = at_mass(a - slope_);
      T x = node->x, lower = a - slope_ - sum(node->left), upper = node->weight - lower;
      Node* left = node->left; Node* right = node->right;
      if (left) left->parent = nullptr;
      if (right) right->parent = nullptr;
      delete node; root_ = nullptr;
      translate(right, length); join(left, right);
      add_break(x, lower); add_break(x + length, upper);
    }
    right_ += length;
  }
  void convolve(SplaySlopeTrick& other) {
    assert(this != &other);
    if (empty_ || other.empty_) { clear(); other.clear(); return; }
    auto a = segments(), b = other.segments();
    SplaySlopeTrick result;
    result.empty_ = false; result.left_ = left_ + other.left_; result.right_ = right_ + other.right_;
    result.value_ = value_ + other.value_;
    T x = result.left_, previous = T(0);
    bool first = true;
    std::size_t i = 0, j = 0;
    while (i < a.size() || j < b.size()) {
      auto segment = j == b.size() || (i < a.size() && a[i].second <= b[j].second) ? a[i++] : b[j++];
      if (first) { result.slope_ = segment.second; first = false; }
      else result.add_break(x, segment.second - previous);
      previous = segment.second; x += segment.first;
    }
    *this = std::move(result); other.clear();
  }
  void clear() { destroy(root_); root_ = nullptr; empty_ = true; left_ = right_ = slope_ = value_ = T(0); }
};
}  // namespace blueberry

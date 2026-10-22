#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace blueberry {
namespace implicit_treap_detail {
struct NoAction {};
template <class S> S no_mapping(NoAction, S value) { return value; }
inline NoAction no_composition(NoAction, NoAction) { return {}; }
inline NoAction no_id() { return {}; }
}
template <class S, auto op, auto e,
          class F = implicit_treap_detail::NoAction,
          auto mapping = implicit_treap_detail::no_mapping<S>,
          auto composition = implicit_treap_detail::no_composition,
          auto id = implicit_treap_detail::no_id>
class ImplicitTreap {
 public:
  explicit ImplicitTreap(const std::vector<S>& values = {},
                         std::uint64_t seed = 0x243f6a8885a308d3ULL) : state_(seed) {
    nodes_.reserve(values.size());
    std::vector<int> stack;
    stack.reserve(values.size());
    for (const S& value : values) {
      const int v = allocate(value);
      int left = -1;
      while (!stack.empty() && nodes_[stack.back()].priority > nodes_[v].priority) {
        left = stack.back(); pull(left); stack.pop_back();
      }
      nodes_[v].left = left;
      if (!stack.empty()) nodes_[stack.back()].right = v;
      stack.push_back(v);
    }
    if (stack.empty()) return;
    root_ = stack.front();
    while (!stack.empty()) { pull(stack.back()); stack.pop_back(); }
  }
  ImplicitTreap(const ImplicitTreap&) = default;
  ImplicitTreap& operator=(const ImplicitTreap&) = default;
  ImplicitTreap(ImplicitTreap&& other) noexcept
      : nodes_(std::move(other.nodes_)), free_(std::move(other.free_)),
        work_(std::move(other.work_)), root_(std::exchange(other.root_, -1)), state_(other.state_) {
    other.nodes_.clear(); other.free_.clear(); other.work_.clear();
  }
  ImplicitTreap& operator=(ImplicitTreap&& other) noexcept {
    if (this == &other) return *this;
    nodes_ = std::move(other.nodes_); free_ = std::move(other.free_); work_ = std::move(other.work_);
    root_ = std::exchange(other.root_, -1); state_ = other.state_;
    other.nodes_.clear(); other.free_.clear(); other.work_.clear();
    return *this;
  }

  int size() const { return count(root_); }
  bool empty() const { return root_ == -1; }
  S all_prod() const { return forward(root_); }
  void clear() {
    root_ = -1;
    std::vector<Node>().swap(nodes_);
    std::vector<int>().swap(free_);
    std::vector<std::pair<int, bool>>().swap(work_);
  }
  void insert(int position, const S& value) {
    const int added = allocate(value);
    auto [left, right] = split(root_, position);
    root_ = merge(merge(left, added), right);
  }
  void erase(int position) {
    auto [left, rest] = split(root_, position);
    auto [removed, right] = split(rest, 1);
    root_ = merge(left, right);
    nodes_[removed] = Node(e(), 0);
    free_.push_back(removed);
  }
  S get(int position) {
    int v = root_;
    for (;;) {
      push(v);
      const int left = count(nodes_[v].left);
      if (position == left) return nodes_[v].value;
      if (position < left) v = nodes_[v].left;
      else { position -= left + 1; v = nodes_[v].right; }
    }
  }
  void set(int position, const S& value) {
    work_.clear();
    int v = root_;
    for (;;) {
      push(v); work_.emplace_back(v, false);
      const int left = count(nodes_[v].left);
      if (position == left) { nodes_[v].value = value; break; }
      if (position < left) v = nodes_[v].left;
      else { position -= left + 1; v = nodes_[v].right; }
    }
    while (!work_.empty()) { pull(work_.back().first); work_.pop_back(); }
  }
  S prod(int l, int r) {
    if (l == r) return e();
    int v = root_;
    for (;;) {
      if (l == 0 && r == count(v)) return forward(v);
      push(v);
      const int left = count(nodes_[v].left);
      if (r <= left) v = nodes_[v].left;
      else if (l > left) { l -= left + 1; r -= left + 1; v = nodes_[v].right; }
      else {
        S result = nodes_[v].value;
        if (l < left) result = op(prod_suffix(nodes_[v].left, l), result);
        if (r > left + 1) result = op(result, prod_prefix(nodes_[v].right, r - left - 1));
        return result;
      }
    }
  }
  void reverse(int l, int r) {
    if (r - l <= 1) return;
    if (l == 0 && r == size()) { toggle(root_); return; }
    auto [left, rest] = split(root_, l);
    auto [middle, right] = split(rest, r - l);
    toggle(middle);
    root_ = merge(left, merge(middle, right));
  }
  void apply(int l, int r, const F& action) {
    if (l == r) return;
    if (l == 0 && r == size()) { apply_node(root_, action); return; }
    work_.clear();
    int v = root_;
    for (;;) {
      if (l == 0 && r == count(v)) { apply_node(v, action); break; }
      push(v); work_.emplace_back(v, false);
      const int left = count(nodes_[v].left);
      if (r <= left) v = nodes_[v].left;
      else if (l > left) { l -= left + 1; r -= left + 1; v = nodes_[v].right; }
      else {
        if (l < left) apply_suffix(nodes_[v].left, l, action);
        nodes_[v].value = mapping(action, nodes_[v].value);
        if (r > left + 1) apply_prefix(nodes_[v].right, r - left - 1, action);
        break;
      }
    }
    while (!work_.empty()) { pull(work_.back().first); work_.pop_back(); }
  }
  std::vector<S> to_vector() {
    std::vector<S> result;
    result.reserve(size());
    std::vector<int> stack;
    int v = root_;
    while (v != -1 || !stack.empty()) {
      while (v != -1) { push(v); stack.push_back(v); v = nodes_[v].left; }
      v = stack.back(); stack.pop_back();
      result.push_back(nodes_[v].value);
      v = nodes_[v].right;
    }
    return result;
  }

 private:
  struct Node {
    S value, forward, backward;
    F lazy;
    std::uint64_t priority;
    int left = -1, right = -1, size = 1;
    bool reversed = false, pending = false;
    Node(const S& value_, std::uint64_t priority_)
        : value(value_), forward(value_), backward(value_), lazy(id()), priority(priority_) {}
  };
  int count(int v) const { return v == -1 ? 0 : nodes_[v].size; }
  S forward(int v) const { return v == -1 ? e() : nodes_[v].forward; }
  S backward(int v) const { return v == -1 ? e() : nodes_[v].backward; }
  void pull(int v) {
    Node& node = nodes_[v];
    node.size = 1 + count(node.left) + count(node.right);
    node.forward = op(op(forward(node.left), node.value), forward(node.right));
    node.backward = op(op(backward(node.right), node.value), backward(node.left));
  }
  void toggle(int v) {
    if (count(v) <= 1) return;
    std::swap(nodes_[v].left, nodes_[v].right);
    using std::swap;
    swap(nodes_[v].forward, nodes_[v].backward);
    nodes_[v].reversed = !nodes_[v].reversed;
  }
  void apply_node(int v, const F& action) {
    if (v == -1) return;
    Node& node = nodes_[v];
    node.value = mapping(action, node.value);
    if (node.size == 1) { node.forward = node.value; node.backward = node.value; return; }
    node.forward = mapping(action, node.forward);
    node.backward = mapping(action, node.backward);
    node.lazy = node.pending ? composition(action, node.lazy) : action;
    node.pending = true;
  }
  void push(int v) {
    Node& node = nodes_[v];
    if (node.reversed) { toggle(node.left); toggle(node.right); node.reversed = false; }
    if (node.pending) {
      apply_node(node.left, node.lazy); apply_node(node.right, node.lazy);
      node.lazy = id(); node.pending = false;
    }
  }
  std::uint64_t random() {
    std::uint64_t z = (state_ += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
  }
  int allocate(const S& value) {
    Node node(value, random());
    if (!free_.empty()) {
      const int v = free_.back(); free_.pop_back(); nodes_[v] = std::move(node); return v;
    }
    nodes_.push_back(std::move(node));
    return static_cast<int>(nodes_.size()) - 1;
  }
  std::pair<int, int> split(int v, int prefix) {
    work_.clear();
    while (v != -1) {
      push(v);
      const int left = count(nodes_[v].left);
      const bool descend_right = prefix > left;
      work_.emplace_back(v, descend_right);
      if (descend_right) { prefix -= left + 1; v = nodes_[v].right; }
      else v = nodes_[v].left;
    }
    int a = -1, b = -1;
    while (!work_.empty()) {
      auto [node, right] = work_.back(); work_.pop_back();
      if (right) { nodes_[node].right = a; pull(node); a = node; }
      else { nodes_[node].left = b; pull(node); b = node; }
    }
    return {a, b};
  }
  int merge(int a, int b) {
    work_.clear();
    while (a != -1 && b != -1) {
      if (nodes_[a].priority < nodes_[b].priority) {
        push(a); work_.emplace_back(a, true); a = nodes_[a].right;
      } else {
        push(b); work_.emplace_back(b, false); b = nodes_[b].left;
      }
    }
    int result = a == -1 ? b : a;
    while (!work_.empty()) {
      auto [v, right] = work_.back(); work_.pop_back();
      if (right) nodes_[v].right = result; else nodes_[v].left = result;
      pull(v); result = v;
    }
    return result;
  }
  S prod_prefix(int v, int length) {
    S result = e();
    while (v != -1 && length) {
      if (length == count(v)) return op(result, forward(v));
      push(v);
      const int left = count(nodes_[v].left);
      if (length <= left) v = nodes_[v].left;
      else {
        result = op(result, op(forward(nodes_[v].left), nodes_[v].value));
        length -= left + 1; v = nodes_[v].right;
      }
    }
    return result;
  }
  S prod_suffix(int v, int skip) {
    S result = e();
    while (v != -1) {
      if (!skip) return op(forward(v), result);
      push(v);
      const int left = count(nodes_[v].left);
      if (skip <= left) {
        result = op(op(nodes_[v].value, forward(nodes_[v].right)), result);
        v = nodes_[v].left;
      } else { skip -= left + 1; v = nodes_[v].right; }
    }
    return result;
  }
  void apply_prefix(int v, int length, const F& action) {
    const auto mark = work_.size();
    while (v != -1 && length) {
      if (length == count(v)) { apply_node(v, action); break; }
      push(v); work_.emplace_back(v, false);
      const int left = count(nodes_[v].left);
      if (length <= left) v = nodes_[v].left;
      else {
        apply_node(nodes_[v].left, action);
        nodes_[v].value = mapping(action, nodes_[v].value);
        length -= left + 1; v = nodes_[v].right;
      }
    }
    while (work_.size() > mark) { pull(work_.back().first); work_.pop_back(); }
  }
  void apply_suffix(int v, int skip, const F& action) {
    const auto mark = work_.size();
    while (v != -1) {
      if (!skip) { apply_node(v, action); break; }
      push(v); work_.emplace_back(v, false);
      const int left = count(nodes_[v].left);
      if (skip <= left) {
        nodes_[v].value = mapping(action, nodes_[v].value);
        apply_node(nodes_[v].right, action);
        v = nodes_[v].left;
      } else { skip -= left + 1; v = nodes_[v].right; }
    }
    while (work_.size() > mark) { pull(work_.back().first); work_.pop_back(); }
  }
  std::vector<Node> nodes_;
  std::vector<int> free_;
  std::vector<std::pair<int, bool>> work_;
  int root_ = -1;
  std::uint64_t state_;
};

}

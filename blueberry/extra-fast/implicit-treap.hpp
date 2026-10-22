#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

namespace blueberry::extra_fast {

template <std::uint32_t Mod = 998244353>
class ImplicitTreap {
  static_assert(Mod <= (std::uint32_t(1) << 31));

 public:
  using value_type = std::conditional_t<Mod == 0, std::uint64_t, std::uint32_t>;

 private:
  static constexpr std::uint32_t reversed_ = std::uint32_t(1) << 31;
  static constexpr bool montgomery_ = (Mod & 1) != 0;
  static constexpr std::uint32_t inverse_ = [] {
    std::uint32_t x = Mod;
    for (int i = 0; i < 5; ++i) x *= 2 - Mod * x;
    return 0 - x;
  }();
  static constexpr value_type one_ = [] {
    if constexpr (montgomery_) return value_type((std::uint64_t(1) << 32) % Mod);
    else return value_type(1);
  }();

  struct alignas(16) Node {
    std::uint32_t left = 0, right = 0, size_rev = 0, priority = 0;
    value_type value = 0, sum = 0, mul = one_, add = 0;
  };
  static_assert(sizeof(Node) == (Mod == 0 ? 48 : 32));

 public:
  explicit ImplicitTreap(const std::vector<value_type>& values = {},
                         std::uint64_t seed = 0x243f6a8885a308d3ULL)
      : state_(seed) {
    reserve(static_cast<int>(values.size()));
    std::vector<std::uint32_t> stack;
    stack.reserve(values.size());
    for (auto x : values) {
      const auto v = allocate(encode(x));
      std::uint32_t last = 0;
      while (!stack.empty() && higher(v, stack.back())) {
        last = stack.back();
        pull(last);
        stack.pop_back();
      }
      nodes_[v].left = last;
      if (!stack.empty()) nodes_[stack.back()].right = v;
      stack.push_back(v);
    }
    if (!stack.empty()) root_ = stack.front();
    while (!stack.empty()) { pull(stack.back()); stack.pop_back(); }
  }
  ImplicitTreap(const ImplicitTreap&) = default;
  ImplicitTreap(ImplicitTreap&& other) noexcept
      : nodes_(std::move(other.nodes_)), root_(std::exchange(other.root_, 0)),
        free_(std::exchange(other.free_, 0)), state_(other.state_) {
    other.nodes_.clear();
  }
  ImplicitTreap& operator=(ImplicitTreap other) noexcept {
    nodes_.swap(other.nodes_);
    std::swap(root_, other.root_);
    std::swap(free_, other.free_);
    std::swap(state_, other.state_);
    return *this;
  }

  int size() const { return static_cast<int>(count(root_)); }
  bool empty() const { return root_ == 0; }
  value_type all_prod() const { return decode(sum(root_)); }
  void reserve(int capacity) {
    if (capacity) nodes_.reserve(static_cast<std::size_t>(capacity) + 1);
  }
  void clear() noexcept {
    std::vector<Node>().swap(nodes_);
    root_ = free_ = 0;
  }
  void insert(int p, value_type x) {
    const auto v = allocate(encode(x));
    root_ = insert_node(root_, static_cast<std::uint32_t>(p), v);
  }
  void erase(int p) {
    root_ = erase_node(root_, static_cast<std::uint32_t>(p));
  }
  value_type get(int p) {
    auto v = root_;
    for (;;) {
      push(v);
      const int left = static_cast<int>(count(nodes_[v].left));
      if (p == left) return decode(nodes_[v].value);
      if (p < left) v = nodes_[v].left;
      else { p -= left + 1; v = nodes_[v].right; }
    }
  }
  void set(int p, value_type x) {
    set_node(root_, static_cast<std::uint32_t>(p), encode(x));
  }
  value_type prod(int l, int r) {
    return l == r ? 0 : decode(prod_node(root_, l, r));
  }
  void reverse(int l, int r) {
    if (r - l <= 1) return;
    if (l == 0 && r == size()) { toggle(root_); return; }
    auto [a, rest] = split(root_, l);
    auto [b, c] = split(rest, r - l);
    toggle(b);
    root_ = merge(a, merge(b, c));
  }
  void apply(int l, int r, value_type a, value_type b) {
    a = encode(a); b = encode(b);
    if (l != r && (a != one_ || b != 0)) apply_range(root_, l, r, a, b);
  }
  std::vector<value_type> to_vector() {
    std::vector<value_type> result;
    result.reserve(size());
    collect(root_, result);
    return result;
  }

 private:
  static value_type add(value_type a, value_type b) {
    const value_type s = a + b;
    if constexpr (Mod == 0) return s;
    else return s >= Mod ? s - Mod : s;
  }
  static value_type multiply(value_type a, value_type b) {
    if constexpr (Mod == 0) return a * b;
    else if constexpr (montgomery_) {
      const std::uint64_t t = std::uint64_t(a) * b;
      const std::uint32_t q = static_cast<std::uint32_t>(t) * inverse_;
      const auto s = static_cast<std::uint32_t>((t + std::uint64_t(q) * Mod) >> 32);
      return s >= Mod ? s - Mod : s;
    } else return std::uint64_t(a) * b % Mod;
  }
  static value_type encode(value_type x) {
    if constexpr (Mod == 0) return x;
    else if constexpr (montgomery_) return std::uint64_t(x) * one_ % Mod;
    else return x % Mod;
  }
  static value_type decode(value_type x) {
    if constexpr (montgomery_) return multiply(x, 1);
    else return x;
  }
  std::uint32_t count(std::uint32_t v) const {
    return v ? nodes_[v].size_rev & ~reversed_ : 0;
  }
  value_type sum(std::uint32_t v) const { return v ? nodes_[v].sum : 0; }
  bool higher(std::uint32_t a, std::uint32_t b) const {
    return nodes_[a].priority < nodes_[b].priority ||
           (nodes_[a].priority == nodes_[b].priority && a < b);
  }
  void pull(std::uint32_t v) {
    auto& n = nodes_[v];
    n.size_rev = (n.size_rev & reversed_) | (1 + count(n.left) + count(n.right));
    n.sum = add(add(sum(n.left), n.value), sum(n.right));
  }
  void toggle(std::uint32_t v) {
    if (!v) return;
    std::swap(nodes_[v].left, nodes_[v].right);
    nodes_[v].size_rev ^= reversed_;
  }
  void apply_node(std::uint32_t v, value_type a, value_type b) {
    if (!v) return;
    auto& n = nodes_[v];
    n.value = add(multiply(a, n.value), b);
    value_type delta;
    if constexpr (Mod == 0) delta = b * count(v);
    else delta = std::uint64_t(b) * count(v) % Mod;
    n.sum = add(multiply(a, n.sum), delta);
    n.mul = multiply(a, n.mul);
    n.add = add(multiply(a, n.add), b);
  }
  void push(std::uint32_t v) {
    auto& n = nodes_[v];
    if (n.size_rev & reversed_) {
      toggle(n.left); toggle(n.right); n.size_rev ^= reversed_;
    }
    if (n.mul != one_ || n.add != 0) {
      apply_node(n.left, n.mul, n.add); apply_node(n.right, n.mul, n.add);
      n.mul = one_; n.add = 0;
    }
  }
  std::uint32_t allocate(value_type x) {
    std::uint64_t z = (state_ += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    Node n;
    n.value = n.sum = x; n.size_rev = 1;
    n.priority = static_cast<std::uint32_t>((z ^ (z >> 31)) >> 32);
    if (free_) {
      const auto v = free_; free_ = nodes_[v].right; nodes_[v] = n; return v;
    }
    if (nodes_.empty()) nodes_.emplace_back();
    nodes_.push_back(n);
    return static_cast<std::uint32_t>(nodes_.size() - 1);
  }
  std::pair<std::uint32_t, std::uint32_t> split(std::uint32_t v, int prefix) {
    if (!v || prefix == 0) return {0, v};
    if (prefix == static_cast<int>(count(v))) return {v, 0};
    push(v);
    const int left = static_cast<int>(count(nodes_[v].left));
    if (prefix <= left) {
      auto [a, b] = split(nodes_[v].left, prefix);
      nodes_[v].left = b; pull(v); return {a, v};
    }
    auto [a, b] = split(nodes_[v].right, prefix - left - 1);
    nodes_[v].right = a; pull(v); return {v, b};
  }
  std::uint32_t merge(std::uint32_t a, std::uint32_t b) {
    if (!a || !b) return a ? a : b;
    if (higher(a, b)) {
      push(a); nodes_[a].right = merge(nodes_[a].right, b); pull(a); return a;
    }
    push(b); nodes_[b].left = merge(a, nodes_[b].left); pull(b); return b;
  }
  std::uint32_t insert_node(std::uint32_t v, std::uint32_t p, std::uint32_t added) {
    if (!v) return added;
    if (higher(added, v)) {
      auto [a, b] = split(v, static_cast<int>(p));
      nodes_[added].left = a; nodes_[added].right = b; pull(added); return added;
    }
    push(v);
    const auto left = count(nodes_[v].left);
    if (p <= left) nodes_[v].left = insert_node(nodes_[v].left, p, added);
    else nodes_[v].right = insert_node(nodes_[v].right, p - left - 1, added);
    pull(v); return v;
  }
  std::uint32_t erase_node(std::uint32_t v, std::uint32_t p) {
    push(v);
    const auto left = count(nodes_[v].left);
    if (p == left) {
      const auto result = merge(nodes_[v].left, nodes_[v].right);
      nodes_[v] = Node{}; nodes_[v].right = free_; free_ = v; return result;
    }
    if (p < left) nodes_[v].left = erase_node(nodes_[v].left, p);
    else nodes_[v].right = erase_node(nodes_[v].right, p - left - 1);
    pull(v); return v;
  }
  void set_node(std::uint32_t v, std::uint32_t p, value_type x) {
    push(v);
    const auto left = count(nodes_[v].left);
    if (p == left) nodes_[v].value = x;
    else if (p < left) set_node(nodes_[v].left, p, x);
    else set_node(nodes_[v].right, p - left - 1, x);
    pull(v);
  }
  value_type prod_node(std::uint32_t v, int l, int r) {
    if (l == 0 && r == static_cast<int>(count(v))) return sum(v);
    push(v);
    const int left = static_cast<int>(count(nodes_[v].left));
    value_type result = 0;
    if (l < left) result = prod_node(nodes_[v].left, l, std::min(r, left));
    if (l <= left && left < r) result = add(result, nodes_[v].value);
    if (r > left + 1)
      result = add(result, prod_node(nodes_[v].right, std::max(0, l - left - 1), r - left - 1));
    return result;
  }
  void apply_range(std::uint32_t v, int l, int r, value_type a, value_type b) {
    if (l == 0 && r == static_cast<int>(count(v))) { apply_node(v, a, b); return; }
    push(v);
    const int left = static_cast<int>(count(nodes_[v].left));
    if (l < left) apply_range(nodes_[v].left, l, std::min(r, left), a, b);
    if (l <= left && left < r) nodes_[v].value = add(multiply(a, nodes_[v].value), b);
    if (r > left + 1)
      apply_range(nodes_[v].right, std::max(0, l - left - 1), r - left - 1, a, b);
    pull(v);
  }
  void collect(std::uint32_t v, std::vector<value_type>& out) {
    if (!v) return;
    push(v); collect(nodes_[v].left, out);
    out.push_back(decode(nodes_[v].value)); collect(nodes_[v].right, out);
  }

  std::vector<Node> nodes_;
  std::uint32_t root_ = 0, free_ = 0;
  std::uint64_t state_;
};

}

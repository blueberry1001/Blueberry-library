#pragma once
#include <algorithm>
#include <cassert>
#include <limits>
#include <vector>

namespace blueberry {
// Splay-based preferred paths. op may be noncommutative; both directions are stored.
template<class S, auto op, auto e>
class LinkCutTree {
  struct Node {
    int child[2]{-1, -1}, parent = -1;
    bool reversed = false;
    S value, forward, backward;
    explicit Node(const S& x) : value(x), forward(x), backward(x) {}
  };
  std::vector<Node> nodes_;
  std::vector<int> stack_;
  void check(int v) const { assert(0 <= v && v < size()); (void)v; }
  bool auxiliary_root(int v) const {
    const int p = nodes_[v].parent;
    return p == -1 || (nodes_[p].child[0] != v && nodes_[p].child[1] != v);
  }
  S aggregate(int v, bool backward) const {
    return v == -1 ? e() : (backward ? nodes_[v].backward : nodes_[v].forward);
  }
  void pull(int v) {
    auto& a = nodes_[v];
    a.forward = op(op(aggregate(a.child[0], false), a.value), aggregate(a.child[1], false));
    a.backward = op(op(aggregate(a.child[1], true), a.value), aggregate(a.child[0], true));
  }
  void reverse(int v) {
    if (v == -1) return;
    auto& a = nodes_[v];
    std::swap(a.child[0], a.child[1]); std::swap(a.forward, a.backward);
    a.reversed = !a.reversed;
  }
  void push(int v) {
    if (!nodes_[v].reversed) return;
    reverse(nodes_[v].child[0]); reverse(nodes_[v].child[1]); nodes_[v].reversed = false;
  }
  void rotate(int v) {
    const int p = nodes_[v].parent, g = nodes_[p].parent;
    const int d = nodes_[p].child[1] == v, middle = nodes_[v].child[d ^ 1];
    if (!auxiliary_root(p)) nodes_[g].child[nodes_[g].child[1] == p] = v;
    nodes_[v].parent = g; nodes_[v].child[d ^ 1] = p; nodes_[p].parent = v;
    nodes_[p].child[d] = middle;
    if (middle != -1) nodes_[middle].parent = p;
    pull(p);
  }
  void splay(int v) {
    stack_.clear();
    for (int x = v;; x = nodes_[x].parent) {
      stack_.push_back(x); if (auxiliary_root(x)) break;
    }
    for (auto it = stack_.rbegin(); it != stack_.rend(); ++it) push(*it);
    while (!auxiliary_root(v)) {
      const int p = nodes_[v].parent, g = nodes_[p].parent;
      if (!auxiliary_root(p)) {
        if ((nodes_[p].child[1] == v) == (nodes_[g].child[1] == p)) rotate(p);
        else rotate(v);
      }
      rotate(v);
    }
    pull(v);
  }
  void access(int v) {
    int last = -1;
    for (int x = v; x != -1; x = nodes_[x].parent) {
      splay(x); nodes_[x].child[1] = last; pull(x); last = x;
    }
    splay(v);
  }
  void make_root(int v) { access(v); reverse(v); }
  int root(int v) {
    access(v); push(v);
    while (nodes_[v].child[0] != -1) { v = nodes_[v].child[0]; push(v); }
    splay(v); return v;
  }
 public:
  explicit LinkCutTree(const std::vector<S>& values = {}) {
    assert(values.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    nodes_.reserve(values.size());
    for (const S& x : values) nodes_.emplace_back(x);
  }
  int size() const { return static_cast<int>(nodes_.size()); }
  void evert(int v) { check(v); make_root(v); }
  int leader(int v) { check(v); return root(v); }
  bool same(int u, int v) { check(u); check(v); return u == v || root(u) == root(v); }
  bool link(int u, int v) {
    check(u); check(v); make_root(u);
    if (root(v) == u) return false;
    nodes_[u].parent = v; return true;
  }
  bool cut(int u, int v) {
    check(u); check(v); make_root(u); access(v);
    if (nodes_[v].child[0] != u || nodes_[u].child[1] != -1) return false;
    nodes_[v].child[0] = -1; nodes_[u].parent = -1; pull(v); return true;
  }
  S get(int v) const { check(v); return nodes_[v].value; }
  void set(int v, const S& x) { check(v); access(v); nodes_[v].value = x; pull(v); }
  S prod(int u, int v) {
    check(u); check(v); assert(same(u, v)); make_root(u); access(v);
    return nodes_[v].forward;
  }
};
}  // namespace blueberry

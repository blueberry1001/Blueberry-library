#define PROBLEM "https://judge.yosupo.jp/problem/point_set_range_composite"
#include <iostream>
#include <atcoder/modint>
#include "blueberry/data-structure/ordered-multiset.hpp"
using mint = atcoder::modint998244353;
struct T { int key; mint a, b; };
T op(T f, T g) {
  if (f.key == -1) return g;
  if (g.key == -1) return f;
  return {f.key, g.a * f.a, g.a * f.b + g.b};
}
T e() { return {-1, 1, 0}; }
struct Compare { bool operator()(const T& a, const T& b) const { return a.key < b.key; } };
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n, q; std::cin >> n >> q;
  blueberry::OrderedMultiset<T, op, e, Compare> bag;
  for (int i = 0; i < n; ++i) { int a, b; std::cin >> a >> b; bag.insert(T{i, a, b}); }
  // Noncommutative rank aggregation; only leaf keys participate in ordering.
  while (q--) {
    int type; std::cin >> type;
    if (type == 0) {
      int p, a, b; std::cin >> p >> a >> b;
      bag.erase(T{p, 0, 0}); bag.insert(T{p, a, b});
    } else {
      int l, r, x; std::cin >> l >> r >> x;
      const T f = bag.prod(l, r);
      std::cout << (f.a * x + f.b).val() << '\n';
    }
  }
}

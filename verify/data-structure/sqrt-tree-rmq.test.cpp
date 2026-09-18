#define PROBLEM "https://judge.yosupo.jp/problem/staticrmq"
#include <algorithm>
#include <iostream>
#include <limits>
#include <vector>
#include "blueberry/data-structure/sqrt-tree.hpp"
int op(int a, int b) { return std::min(a,b); }
int e() { return std::numeric_limits<int>::max(); }
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n, q; std::cin >> n >> q;
  std::vector<int> a(n); for (auto& x : a) std::cin >> x;
  blueberry::SqrtTree<int, op, e> tree(a);
  // Static RMQ covers an idempotent monoid alongside the sum driver.
  while (q--) { int l, r; std::cin >> l >> r; std::cout << tree.prod(l, r) << '\n'; }
}

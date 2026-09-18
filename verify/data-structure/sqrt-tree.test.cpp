#define PROBLEM "https://judge.yosupo.jp/problem/static_range_sum"
#include <iostream>
#include <vector>
#include "blueberry/data-structure/sqrt-tree.hpp"
long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n, q; std::cin >> n >> q;
  std::vector<long long> a(n); for (auto& x : a) std::cin >> x;
  blueberry::SqrtTree<long long, op, e> tree(a);
  // Non-idempotent aggregation across square-root block boundaries.
  while (q--) { int l, r; std::cin >> l >> r; std::cout << tree.prod(l, r) << '\n'; }
}

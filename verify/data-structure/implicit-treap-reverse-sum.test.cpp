#define PROBLEM "https://judge.yosupo.jp/problem/range_reverse_range_sum"
#include <iostream>
#include <vector>
#include "blueberry/data-structure/implicit-treap.hpp"
long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n, q; std::cin >> n >> q;
  std::vector<long long> a(n);
  for (auto& x : a) std::cin >> x;
  // The default NoAction keeps the ordinary monoid interface short.
  blueberry::ImplicitTreap<long long, op, e> tree(a);
  while (q--) {
    int type, l, r; std::cin >> type >> l >> r;
    if (type == 0) tree.reverse(l, r);
    else std::cout << tree.prod(l, r) << '\n';
  }
}

#define PROBLEM "https://judge.yosupo.jp/problem/point_add_range_sum"
#include <iostream>
#include <vector>
#include "blueberry/data-structure/dynamic-sqrt-tree.hpp"
long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n, q; std::cin >> n >> q;
  std::vector<long long> a(n); for (auto& x : a) std::cin >> x;
  blueberry::DynamicSqrtTree<long long, op, e> tree(a);
  // Add is expressed through the point-assignment API.
  while (q--) {
    int t, l, r; std::cin >> t >> l >> r;
    if (t == 0) tree.set(l, tree.get(l) + r);
    else std::cout << tree.prod(l, r) << '\n';
  }
}

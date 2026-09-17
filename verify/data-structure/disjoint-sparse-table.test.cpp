#define PROBLEM "https://judge.yosupo.jp/problem/static_range_sum"
#include <iostream>
#include <vector>
#include "blueberry/data-structure/disjoint-sparse-table.hpp"
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n, q; std::cin >> n >> q;
  std::vector<long long> a(n); for (auto& x : a) std::cin >> x;
  blueberry::DisjointSparseTable table(a, [](long long x, long long y) { return x + y; });
  // Non-idempotent sum exercises disjoint aggregation.
  while (q--) { int l, r; std::cin >> l >> r; std::cout << (l == r ? 0 : table.prod(l, r)) << '\n'; }
}

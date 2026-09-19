#define PROBLEM "https://judge.yosupo.jp/problem/staticrmq"
// @title Disjoint Sparse Table - Static RMQ（区間最小値）
#include <algorithm>
#include <iostream>
#include <vector>
#include "blueberry/data-structure/disjoint-sparse-table.hpp"

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  int n, q;
  std::cin >> n >> q;
  std::vector<int> a(n);
  for (int& x : a) std::cin >> x;
  // Static RMQ asks for the minimum, unlike the separate range-sum verifier.
  blueberry::DisjointSparseTable table(a, [](int x, int y) { return std::min(x, y); });
  while (q--) {
    int l, r;
    std::cin >> l >> r;
    std::cout << table.prod(l, r) << '\n';
  }
}

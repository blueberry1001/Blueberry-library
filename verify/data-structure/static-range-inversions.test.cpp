#define PROBLEM "https://judge.yosupo.jp/problem/static_range_inversions_query"
#include <iostream>
#include "blueberry/data-structure/static-range-inversions.hpp"
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n, q; std::cin >> n >> q;
  std::vector<int> a(n); for (int& x : a) std::cin >> x;
  std::vector<std::pair<int, int>> queries(q);
  for (auto& [l, r] : queries) std::cin >> l >> r;
  for (long long x : blueberry::static_range_inversions(a, queries)) std::cout << x << '\n';
}

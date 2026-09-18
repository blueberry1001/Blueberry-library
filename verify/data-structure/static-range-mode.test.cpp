#define PROBLEM "https://judge.yosupo.jp/problem/static_range_mode_query"
#include <iostream>
#include "blueberry/data-structure/static-range-mode.hpp"
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n, q; std::cin >> n >> q;
  std::vector<int> a(n); for (int& x : a) std::cin >> x;
  std::vector<std::pair<int, int>> queries(q);
  for (auto& [l, r] : queries) std::cin >> l >> r;
  for (auto [i, f] : blueberry::static_range_mode(a, queries)) std::cout << a[i] << ' ' << f << '\n';
}

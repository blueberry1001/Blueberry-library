#define PROBLEM "https://judge.yosupo.jp/problem/static_range_lis_query"
#include <iostream>
#include <vector>
#include "blueberry/data-structure/static-range-lis.hpp"

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  int n, q;
  std::cin >> n >> q;
  std::vector<int> a(n);
  for (int& x : a) std::cin >> x;
  blueberry::StaticRangeLIS<int> ds(a);
  while (q--) {
    int l, r;
    std::cin >> l >> r;
    std::cout << ds.lis(l, r) << '\n';
  }
}

#define PROBLEM "https://judge.yosupo.jp/problem/predecessor_problem"
#include <iostream>
#include <string>
#include "blueberry/data-structure/ordered-set.hpp"

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  int n, q;
  std::string initial;
  std::cin >> n >> q >> initial;
  blueberry::OrderedSet<int> set;
  for (int i = 0; i < n; ++i) if (initial[i] == '1') set.insert(i);
  while (q--) {
    int type, k;
    std::cin >> type >> k;
    if (type == 0) set.insert(k);
    if (type == 1) set.erase(k);
    if (type == 2) std::cout << set.contains(k) << '\n';
    if (type == 3) std::cout << set.lower_bound(k).value_or(-1) << '\n';
    if (type == 4) std::cout << set.floor(k).value_or(-1) << '\n';
  }
}

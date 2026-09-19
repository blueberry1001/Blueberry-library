#define PROBLEM "https://judge.yosupo.jp/problem/predecessor_problem"
#include <iostream>
#include <string>
#include "blueberry/data-structure/fast-set.hpp"
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n, q; std::string bits; std::cin >> n >> q >> bits;
  blueberry::FastSet set(bits);
  while (q--) {
    int t, x; std::cin >> t >> x;
    if (t == 0) set.insert(x);
    if (t == 1) set.erase(x);
    if (t == 2) std::cout << set.contains(x) << '\n';
    if (t == 3) { int y = set.next(x); std::cout << (y == n ? -1 : y) << '\n'; }
    if (t == 4) std::cout << set.prev(x) << '\n';
  }
}

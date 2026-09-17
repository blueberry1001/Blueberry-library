#define PROBLEM "https://judge.yosupo.jp/problem/double_ended_priority_queue"
#include <iostream>
#include "blueberry/data-structure/ordered-multiset.hpp"
long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n, q; std::cin >> n >> q;
  blueberry::OrderedMultiset<long long, op, e> bag;
  for (int i = 0; i < n; ++i) { long long x; std::cin >> x; bag.insert(x); }
  // Duplicates remain separate occurrences when either end is removed.
  while (q--) {
    int type; std::cin >> type;
    if (type == 0) { long long x; std::cin >> x; bag.insert(x); }
    else {
      const int position = type == 1 ? 0 : bag.size() - 1;
      std::cout << bag.kth(position).value() << '\n'; bag.erase_kth(position);
    }
  }
}

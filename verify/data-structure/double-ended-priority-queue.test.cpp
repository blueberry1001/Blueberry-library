#define PROBLEM "https://judge.yosupo.jp/problem/double_ended_priority_queue"
#include <iostream>
#include <vector>
#include "blueberry/data-structure/double-ended-priority-queue.hpp"
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n, q; std::cin >> n >> q;
  std::vector<long long> a(n); for (auto& x : a) std::cin >> x;
  blueberry::DoubleEndedPriorityQueue<long long> queue(a);
  while (q--) {
    int t; std::cin >> t;
    if (t == 0) { long long x; std::cin >> x; queue.push(x); }
    else if (t == 1) { std::cout << queue.min() << '\n'; queue.pop_min(); }
    else { std::cout << queue.max() << '\n'; queue.pop_max(); }
  }
}

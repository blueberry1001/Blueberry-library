#define PROBLEM "https://judge.yosupo.jp/problem/deque_operate_all_composite"
#include <iostream>
#include <atcoder/modint>
#include "blueberry/data-structure/aggregate-deque.hpp"
using mint = atcoder::modint998244353;
struct F { mint a, b; };
F op(F left, F right) { return {left.a * right.a, left.b * right.a + right.b}; }
F e() { return {1, 0}; }
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int q; std::cin >> q;
  blueberry::AggregateDeque<F, op, e> deque;
  while (q--) {
    int type; std::cin >> type;
    if (type < 2) { int a, b; std::cin >> a >> b; if (type == 0) deque.push_front(F{a, b}); else deque.push_back(F{a, b}); }
    else if (type == 2) deque.pop_front();
    else if (type == 3) deque.pop_back();
    else { int x; std::cin >> x; F f = deque.prod(); std::cout << (f.a * x + f.b).val() << '\n'; }
  }
}

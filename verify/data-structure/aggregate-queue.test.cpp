#define PROBLEM "https://judge.yosupo.jp/problem/queue_operate_all_composite"
#include <iostream>
#include <atcoder/modint>
#include "blueberry/data-structure/aggregate-queue.hpp"
using mint = atcoder::modint998244353;
struct F { mint a, b; };
F op(F left, F right) { return {left.a * right.a, left.b * right.a + right.b}; }
F e() { return {1, 0}; }
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int q; std::cin >> q;
  blueberry::AggregateQueue<F, op, e> queue;
  while (q--) {
    int type; std::cin >> type;
    if (type == 0) { int a, b; std::cin >> a >> b; queue.push(F{a, b}); }
    else if (type == 1) queue.pop();
    else { int x; std::cin >> x; F f = queue.prod(); std::cout << (f.a * x + f.b).val() << '\n'; }
  }
}

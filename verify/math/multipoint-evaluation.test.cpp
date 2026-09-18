#define PROBLEM "https://judge.yosupo.jp/problem/multipoint_evaluation"
#include <iostream>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/multipoint-evaluation.hpp"
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  using mint = atcoder::modint998244353;
  int n, m; std::cin >> n >> m;
  std::vector<mint> f(n), x(m);
  for (auto& a : f) { int v; std::cin >> v; a = v; }
  for (auto& a : x) { int v; std::cin >> v; a = v; }
  // Evaluate at arbitrary, possibly repeated, points.
  auto y = blueberry::multipoint_evaluation(f, x);
  for (int i = 0; i < m; ++i) std::cout << y[i].val() << " \n"[i + 1 == m];
}

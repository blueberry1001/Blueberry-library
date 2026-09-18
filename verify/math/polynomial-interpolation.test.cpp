#define PROBLEM "https://judge.yosupo.jp/problem/polynomial_interpolation"
#include <iostream>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/polynomial-interpolation.hpp"
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  using mint = atcoder::modint998244353;
  int n; std::cin >> n;
  std::vector<mint> x(n), y(n);
  for (auto& a : x) { int v; std::cin >> v; a = v; }
  for (auto& a : y) { int v; std::cin >> v; a = v; }
  // Recover coefficients in increasing degree order.
  auto f = blueberry::polynomial_interpolation(x, y);
  for (int i = 0; i < n; ++i) std::cout << f[i].val() << " \n"[i + 1 == n];
}

#define PROBLEM "https://judge.yosupo.jp/problem/shift_of_sampling_points_of_polynomial"
#include <iostream>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/sample-point-shift.hpp"
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  using mint = atcoder::modint998244353;
  int n, m, c; std::cin >> n >> m >> c;
  std::vector<mint> y(n);
  for (auto& a : y) { int v; std::cin >> v; a = v; }
  // Shift consecutive samples, including a modular wrap.
  auto f = blueberry::sample_point_shift(y, mint(c), m);
  for (int i = 0; i < m; ++i) std::cout << f[i].val() << " \n"[i + 1 == m];
}

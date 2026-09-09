#define PROBLEM "https://judge.yosupo.jp/problem/inv_of_formal_power_series"

#include <iostream>

#include <atcoder/modint>

#include "blueberry/math/formal-power-series.hpp"

using mint = atcoder::modint998244353;
using fps = blueberry::FormalPowerSeries<mint>;

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  int n;
  std::cin >> n;
  fps f(n);
  for (mint& value : f) {
    int coefficient;
    std::cin >> coefficient;
    value = coefficient;
  }
  const fps answer = f.inv(n);
  for (int i = 0; i < n; ++i) {
    if (i) std::cout << ' ';
    std::cout << answer[i].val();
  }
  std::cout << '\n';
}

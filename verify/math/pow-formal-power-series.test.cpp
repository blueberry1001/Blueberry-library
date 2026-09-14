#define PROBLEM "https://judge.yosupo.jp/problem/pow_of_formal_power_series"

#include <cstdint>
#include <iostream>

#include <atcoder/modint>

#include "blueberry/math/formal-power-series.hpp"

using namespace std;

// 非負整数exponent乗をx^N未満まで求める。

using mint = atcoder::modint998244353;
using fps = blueberry::FormalPowerSeries<mint>;

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  int n;
  int64_t exponent;
  cin >> n >> exponent;
  fps f(n);
  for (mint& value : f) {
    int coefficient;
    cin >> coefficient;
    value = coefficient;
  }
  const fps answer = f.pow(exponent, n);
  for (int i = 0; i < n; ++i) {
    if (i) cout << ' ';
    cout << answer[i].val();
  }
  cout << '\n';
}

#define PROBLEM "https://judge.yosupo.jp/problem/exp_of_formal_power_series"

#include <iostream>

#include <atcoder/modint>

#include "blueberry/math/formal-power-series.hpp"

using namespace std;

// f(0)=0を満たす級数のexp(f)をx^N未満まで求める。

using mint = atcoder::modint998244353;
using fps = blueberry::FormalPowerSeries<mint>;

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  int n;
  cin >> n;
  fps f(n);
  for (mint& value : f) {
    int coefficient;
    cin >> coefficient;
    value = coefficient;
  }
  const fps answer = f.exp(n);
  for (int i = 0; i < n; ++i) {
    if (i) cout << ' ';
    cout << answer[i].val();
  }
  cout << '\n';
}

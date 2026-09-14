#define PROBLEM "https://judge.yosupo.jp/problem/log_of_formal_power_series"

#include <iostream>

#include <atcoder/modint>

#include "blueberry/math/formal-power-series.hpp"

using namespace std;

// f(0)=1を満たす級数のlog(f)をx^N未満まで求める。

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
  const fps answer = f.log(n);
  for (int i = 0; i < n; ++i) {
    if (i) cout << ' ';
    cout << answer[i].val();
  }
  cout << '\n';
}

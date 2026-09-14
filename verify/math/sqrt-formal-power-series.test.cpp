#define PROBLEM "https://judge.yosupo.jp/problem/sqrt_of_formal_power_series"

#include <iostream>

#include <atcoder/modint>

#include "blueberry/math/formal-power-series.hpp"

using namespace std;

// 級数の平方根をx^N未満まで求め、存在しない場合は-1を出力する。

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
  const fps answer = f.sqrt(n);
  if (answer.empty()) {
    cout << "-1\n";
    return 0;
  }
  for (int i = 0; i < n; ++i) {
    if (i) cout << ' ';
    cout << answer[i].val();
  }
  cout << '\n';
}

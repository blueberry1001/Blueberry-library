#define PROBLEM "https://judge.yosupo.jp/problem/division_of_polynomials"

#include <iostream>

#include "blueberry/math/formal-power-series.hpp"

using namespace std;
using mint = atcoder::modint998244353;
using fps = blueberry::FormalPowerSeries<mint>;

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  int n, m;
  cin >> n >> m;
  fps f(n), g(m);
  for (fps* polynomial : {&f, &g}) {
    for (mint& value : *polynomial) {
      int coefficient;
      cin >> coefficient;
      value = coefficient;
    }
  }
  // f = q * g + r, deg(r) < deg(g).
  auto [q, r] = f.div_mod(g);
  q.shrink();
  cout << q.size() << ' ' << r.size() << '\n';
  for (const fps* polynomial : {&q, &r}) {
    for (int i = 0; i < static_cast<int>(polynomial->size()); ++i) {
      if (i) cout << ' ';
      cout << (*polynomial)[i].val();
    }
    cout << '\n';
  }
}

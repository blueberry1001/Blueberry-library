#define PROBLEM "https://judge.yosupo.jp/problem/system_of_linear_equations"
#include <iostream>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/matrix.hpp"
using namespace std;
using Mint = atcoder::modint998244353;
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int n, m; cin >> n >> m; blueberry::Matrix<Mint> a(n, m);
  for (int i = 0; i < n; ++i) for (int j = 0; j < m; ++j) {
    int value; cin >> value; a(i, j) = value;
  }
  vector<Mint> b(n);
  for (auto& x : b) { int value; cin >> value; x = value; }
  auto solution = a.solve(b);
  if (!solution) { cout << -1 << '\n'; return 0; }
  cout << solution->basis.size() << '\n';
  for (int j = 0; j < m; ++j) cout << solution->particular[j].val() << (j + 1 == m ? '\n' : ' ');
  for (const auto& v : solution->basis)
    for (int j = 0; j < m; ++j) cout << v[j].val() << (j + 1 == m ? '\n' : ' ');
}

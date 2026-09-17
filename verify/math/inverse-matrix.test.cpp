#define PROBLEM "https://judge.yosupo.jp/problem/inverse_matrix"
#include <iostream>
#include <atcoder/modint>
#include "blueberry/math/matrix.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int n; cin >> n; blueberry::Matrix<atcoder::modint998244353> a(n, n);
  for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
    int value; cin >> value; a(i, j) = value;
  }
  auto inverse = a.inv();
  if (!inverse) { cout << -1 << '\n'; return 0; }
  for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j)
    cout << (*inverse)(i, j).val() << (j + 1 == n ? '\n' : ' ');
}

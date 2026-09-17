#define PROBLEM "https://judge.yosupo.jp/problem/matrix_rank"
#include <iostream>
#include <atcoder/modint>
#include "blueberry/math/matrix.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int n, m; cin >> n >> m; blueberry::Matrix<atcoder::modint998244353> a(n, m);
  for (int i = 0; i < n; ++i) for (int j = 0; j < m; ++j) {
    int value; cin >> value; a(i, j) = value;
  }
  cout << a.rank() << '\n';
}

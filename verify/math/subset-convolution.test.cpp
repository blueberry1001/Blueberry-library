#define PROBLEM "https://judge.yosupo.jp/problem/subset_convolution"
#include <iostream>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/subset-convolution.hpp"
using namespace std;
using Mint = atcoder::modint998244353;
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int bits; cin >> bits; const int n = 1 << bits;
  vector<Mint> a(n), b(n);
  for (auto& x : a) { int value; cin >> value; x = value; }
  for (auto& x : b) { int value; cin >> value; x = value; }
  auto c = blueberry::subset_convolution(a, b);
  for (int i = 0; i < n; ++i) cout << c[i].val() << (i + 1 == n ? '\n' : ' ');
}

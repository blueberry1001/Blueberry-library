#define PROBLEM "https://judge.yosupo.jp/problem/bitwise_and_convolution"
#include <iostream>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/bitwise-convolution.hpp"
using namespace std;
using Mint = atcoder::modint998244353;
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int bits; cin >> bits; const int n = 1 << bits;
  vector<Mint> a(n), b(n);
  // De Morgan complement converts official AND cases into OR convolution.
  for (int i = 0; i < n; ++i) { int value; cin >> value; a[n - 1 - i] = value; }
  for (int i = 0; i < n; ++i) { int value; cin >> value; b[n - 1 - i] = value; }
  auto c = blueberry::bitwise_convolution_or(std::move(a), std::move(b));
  for (int i = 0; i < n; ++i) cout << c[n - 1 - i].val() << (i + 1 == n ? '\n' : ' ');
}

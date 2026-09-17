#define PROBLEM "https://judge.yosupo.jp/problem/gcd_convolution"
#include <atcoder/modint>
#include <iostream>
#include <vector>
#include "blueberry/math/divisor-convolution.hpp"
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n; std::cin >> n;
  using mint = atcoder::modint998244353;
  std::vector<mint> a(n + 1), b(n + 1);
  for (int i = 1, x; i <= n; ++i) { std::cin >> x; a[i] = x; }
  for (int i = 1, x; i <= n; ++i) { std::cin >> x; b[i] = x; }
  auto c = blueberry::gcd_convolution(a, b);
  for (int i = 1; i <= n; ++i) std::cout << c[i].val() << (i == n ? '\n' : ' ');
}

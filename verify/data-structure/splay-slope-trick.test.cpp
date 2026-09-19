#define PROBLEM "https://judge.yosupo.jp/problem/min_plus_convolution_convex_convex"
#include <iostream>
#include <utility>
#include <vector>
#include "blueberry/data-structure/splay-slope-trick.hpp"
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n, m; std::cin >> n >> m;
  std::vector<std::pair<long long, long long>> a(n), b(m);
  for (int i = 0; i < n; ++i) { a[i].first = i; std::cin >> a[i].second; }
  for (int i = 0; i < m; ++i) { b[i].first = i; std::cin >> b[i].second; }
  auto f = blueberry::SplaySlopeTrick<>::from_points(a);
  auto g = blueberry::SplaySlopeTrick<>::from_points(b);
  // Integer breakpoints make the continuous convolution agree at integer x.
  f.convolve(g);
  for (int i = 0; i < n + m - 1; ++i) std::cout << *f.eval(i) << (i + 1 == n + m - 1 ? '\n' : ' ');
}

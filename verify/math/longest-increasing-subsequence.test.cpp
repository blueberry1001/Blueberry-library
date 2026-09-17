#define PROBLEM "https://judge.yosupo.jp/problem/longest_increasing_subsequence"
#include <iostream>
#include <vector>
#include "blueberry/math/longest-increasing-subsequence.hpp"
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n; std::cin >> n;
  std::vector<int> a(n); for (int& x : a) std::cin >> x;
  auto ids = blueberry::longest_increasing_subsequence(a);
  std::cout << ids.size() << '\n';
  for (int i : ids) std::cout << i << ' ';
  std::cout << '\n';
}

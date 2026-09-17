#define PROBLEM "https://judge.yosupo.jp/problem/cartesian_tree"
#include <iostream>
#include <vector>
#include "blueberry/graph/cartesian-tree.hpp"
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n; std::cin >> n;
  std::vector<int> a(n); for (int& x : a) std::cin >> x;
  for (int p : blueberry::cartesian_tree(a)) std::cout << p << ' ';
  std::cout << '\n';
}

#define PROBLEM "https://judge.yosupo.jp/problem/staticrmq"

#include <algorithm>
#include <iostream>
#include <vector>

#include "blueberry/data-structure/sparse-table.hpp"

struct Minimum {
  int operator()(int left, int right) const { return std::min(left, right); }
};

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  int n, q;
  std::cin >> n >> q;
  std::vector<int> values(n);
  for (int& value : values) std::cin >> value;
  blueberry::SparseTable<int, Minimum> sparse_table(values, Minimum{});
  while (q--) {
    int left, right;
    std::cin >> left >> right;
    std::cout << sparse_table.product(left, right) << '\n';
  }
}

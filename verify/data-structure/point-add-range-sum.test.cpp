#define PROBLEM "https://judge.yosupo.jp/problem/point_add_range_sum"

#include <iostream>
#include <vector>

#include "blueberry/data-structure/fenwick-tree.hpp"

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  int n, q;
  std::cin >> n >> q;
  std::vector<long long> values(n);
  for (auto& value : values) std::cin >> value;
  blueberry::FenwickTree<long long> fenwick(values);
  while (q--) {
    int type, x, y;
    std::cin >> type >> x >> y;
    if (type == 0) {
      fenwick.add(x, y);
    } else {
      std::cout << fenwick.sum(x, y) << '\n';
    }
  }
}

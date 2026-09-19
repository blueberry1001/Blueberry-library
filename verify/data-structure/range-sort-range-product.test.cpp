#define PROBLEM "https://judge.yosupo.jp/problem/point_set_range_sort_range_composite"
#include <iostream>
#include <utility>
#include <vector>
#include <atcoder/modint>
#include "blueberry/data-structure/range-sort-range-product.hpp"
using Mint = atcoder::modint998244353;
using Affine = std::pair<Mint, Mint>;
Affine op(Affine f, Affine g) { return {g.first * f.first, g.first * f.second + g.second}; }
Affine e() { return {1, 0}; }
int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  int n, q;
  std::cin >> n >> q;
  std::vector<std::pair<unsigned, Affine>> values(n);
  for (auto& [key, f] : values) {
    int a, b;
    std::cin >> key >> a >> b;
    f = {a, b};
  }
  blueberry::RangeSortRangeProduct<Affine, op, e> ds(values);
  while (q--) {
    int type;
    std::cin >> type;
    if (type == 0) {
      int p, a, b;
      unsigned key;
      std::cin >> p >> key >> a >> b;
      ds.set(p, key, {a, b});
    } else {
      int l, r;
      std::cin >> l >> r;
      if (type == 1) {
        int x;
        std::cin >> x;
        auto [a, b] = ds.prod(l, r);
        std::cout << (a * x + b).val() << '\n';
      } else ds.sort(l, r, type == 3);
    }
  }
}

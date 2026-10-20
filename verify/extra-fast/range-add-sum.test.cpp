#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/DSL_2_G"
#include <cstdint>
#include <iostream>
#include "blueberry/extra-fast/range-add-sum.hpp"

// Inclusive 1-indexed range updates exercise more than point updates.
int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  int n, q;
  std::cin >> n >> q;
  blueberry::extra_fast::RangeAddSum tree(n);
  while (q--) {
    int type, l, r;
    std::cin >> type >> l >> r;
    if (type == 0) {
      std::uint64_t x;
      std::cin >> x;
      tree.apply(l - 1, r, x);
    } else {
      std::cout << tree.prod(l - 1, r) << '\n';
    }
  }
}

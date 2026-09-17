#define PROBLEM "https://judge.yosupo.jp/problem/range_chmin_chmax_add_range_sum"
#include <iostream>
#include <vector>
#include "blueberry/data-structure/segment-tree-beats.hpp"
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n, q; std::cin >> n >> q;
  std::vector<long long> values(n); for (auto& x : values) std::cin >> x;
  blueberry::SegmentTreeBeats tree(values);
  while (q--) {
    int type, l, r; std::cin >> type >> l >> r;
    if (type == 3) std::cout << tree.prod(l, r) << '\n';
    else {
      long long x; std::cin >> x;
      if (type == 0) tree.chmin(l, r, x);
      if (type == 1) tree.chmax(l, r, x);
      if (type == 2) tree.add(l, r, x);
    }
  }
}

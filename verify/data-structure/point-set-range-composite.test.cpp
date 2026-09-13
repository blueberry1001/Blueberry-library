#define PROBLEM "https://judge.yosupo.jp/problem/point_set_range_composite"

#include <iostream>
#include <vector>

#include "blueberry/data-structure/segment-tree.hpp"

namespace {

constexpr long long kMod = 998244353;

struct Affine {
  long long a;
  long long b;
};

struct Compose {
  Affine operator()(const Affine& left, const Affine& right) const {
    return {right.a * left.a % kMod, (right.a * left.b + right.b) % kMod};
  }
};

}  // namespace

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  int n, q;
  std::cin >> n >> q;
  std::vector<Affine> functions(n);
  for (auto& [a, b] : functions) std::cin >> a >> b;
  blueberry::SegmentTree<Affine, Compose> segment_tree(functions, Compose{}, {1, 0});
  while (q--) {
    int type;
    std::cin >> type;
    if (type == 0) {
      int position;
      Affine function;
      std::cin >> position >> function.a >> function.b;
      segment_tree.set(position, function);
    } else {
      int left, right;
      long long x;
      std::cin >> left >> right >> x;
      const auto function = segment_tree.prod(left, right);
      std::cout << (function.a * x + function.b) % kMod << '\n';
    }
  }
}

#define PROBLEM "https://judge.yosupo.jp/problem/range_affine_range_sum"
#include <cstdint>
#include <vector>
#include "blueberry/extra-fast/affine-sum-segment-tree.hpp"
#include "blueberry/utility/fast-io.hpp"

// Dynamic SIMD lazy tree: update chronology and range sums modulo 998244353.
int main() {
  blueberry::FastInput in;
  blueberry::FastOutput out;
  int n, q;
  in.read(n, q);
  std::vector<std::uint32_t> a(n);
  for (auto& x : a) in.read(x);
  blueberry::extra_fast::AffineSumSegmentTree tree(a);
  while (q--) {
    int type, l, r;
    in.read(type, l, r);
    if (type == 0) {
      std::uint32_t multiplier, translation;
      in.read(multiplier, translation);
      tree.apply(l, r, multiplier, translation);
    } else {
      out.writeln(tree.prod(l, r));
    }
  }
}

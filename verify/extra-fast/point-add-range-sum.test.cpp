#define PROBLEM "https://judge.yosupo.jp/problem/point_add_range_sum"
#include <cstdint>
#include <vector>
#include "blueberry/extra-fast/segment-tree.hpp"
#include "blueberry/utility/fast-io.hpp"

int main() {
  blueberry::FastInput in;
  blueberry::FastOutput out;
  int n, q;
  in.read(n, q);
  std::vector<std::uint64_t> a(n);
  for (auto& x : a) in.read(x);
  blueberry::extra_fast::SumSegmentTree tree(a);
  while (q--) {
    int type, p;
    std::uint64_t x;
    in.read(type, p, x);
    if (type == 0) tree.add(p, x);
    else out.writeln(tree.prod(p, static_cast<int>(x)));
  }
}

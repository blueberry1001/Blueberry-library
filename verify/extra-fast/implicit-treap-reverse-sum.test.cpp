#define PROBLEM "https://judge.yosupo.jp/problem/range_reverse_range_sum"
#include <cstdint>
#include <vector>
#include "blueberry/extra-fast/implicit-treap.hpp"
#include "blueberry/utility/fast-io.hpp"
int main() {
  blueberry::FastInput in;
  blueberry::FastOutput out;
  int n, q; in.read(n, q);
  std::vector<std::uint64_t> a(n);
  for (auto& x : a) in.read(x);
  blueberry::extra_fast::ImplicitTreap<0> tree(a);
  while (q--) {
    int type, l, r; in.read(type, l, r);
    if (type == 0) tree.reverse(l, r);
    else out.writeln(tree.prod(l, r));
  }
}

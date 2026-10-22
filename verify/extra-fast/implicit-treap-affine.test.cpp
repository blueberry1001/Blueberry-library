#define PROBLEM "https://judge.yosupo.jp/problem/dynamic_sequence_range_affine_range_sum"
#include <cstdint>
#include <vector>
#include "blueberry/extra-fast/implicit-treap.hpp"
#include "blueberry/utility/fast-io.hpp"
int main() {
  blueberry::FastInput in;
  blueberry::FastOutput out;
  int n, q; in.read(n, q);
  std::vector<std::uint32_t> a(n);
  for (auto& x : a) in.read(x);
  blueberry::extra_fast::ImplicitTreap<> tree(a);
  tree.reserve(n + q);
  while (q--) {
    int type; in.read(type);
    if (type == 0) { int p; std::uint32_t x; in.read(p, x); tree.insert(p, x); }
    else if (type == 1) { int p; in.read(p); tree.erase(p); }
    else {
      int l, r; in.read(l, r);
      if (type == 2) tree.reverse(l, r);
      else if (type == 3) { std::uint32_t x, y; in.read(x, y); tree.apply(l, r, x, y); }
      else out.writeln(tree.prod(l, r));
    }
  }
}

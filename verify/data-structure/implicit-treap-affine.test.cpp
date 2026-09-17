#define PROBLEM "https://judge.yosupo.jp/problem/dynamic_sequence_range_affine_range_sum"
#include <iostream>
#include <vector>
#include <atcoder/modint>
#include "blueberry/data-structure/implicit-treap.hpp"
using mint = atcoder::modint998244353;
struct S { mint sum; int length; };
struct F { mint a, b; };
S op(S a, S b) { return {a.sum + b.sum, a.length + b.length}; }
S e() { return {0, 0}; }
S mapping(F f, S x) { return {f.a * x.sum + f.b * x.length, x.length}; }
F composition(F f, F g) { return {f.a * g.a, f.a * g.b + f.b}; }
F id() { return {1, 0}; }
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n, q; std::cin >> n >> q;
  std::vector<S> initial(n);
  for (auto& x : initial) { int value; std::cin >> value; x = {value, 1}; }
  blueberry::ImplicitTreap<S, op, e, F, mapping, composition, id> tree(initial);
  // All five dynamic sequence operations share the same node pool.
  while (q--) {
    int type; std::cin >> type;
    if (type == 0) { int p, x; std::cin >> p >> x; tree.insert(p, S{x, 1}); }
    else if (type == 1) { int p; std::cin >> p; tree.erase(p); }
    else {
      int l, r; std::cin >> l >> r;
      if (type == 2) tree.reverse(l, r);
      else if (type == 3) { int a, b; std::cin >> a >> b; tree.apply(l, r, F{a, b}); }
      else std::cout << tree.prod(l, r).sum.val() << '\n';
    }
  }
}

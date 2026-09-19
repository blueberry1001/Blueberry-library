#define PROBLEM "https://judge.yosupo.jp/problem/range_affine_point_get"
#include <iostream>
#include <vector>
#include <atcoder/modint>
#include "blueberry/data-structure/dual-segment-tree.hpp"
using Mint = atcoder::modint998244353;
struct F { Mint a, b; };
Mint mapping(F f, Mint x) { return f.a * x + f.b; }
F composition(F f, F g) { return {f.a * g.a, f.a * g.b + f.b}; }
F id() { return {1, 0}; }
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n, q; std::cin >> n >> q;
  std::vector<Mint> a(n); for (auto& x : a) { int v; std::cin >> v; x = v; }
  blueberry::DualSegmentTree<Mint, F, mapping, composition, id> tree(a);
  // Affine actions are noncommutative, exercising chronological composition.
  while (q--) {
    int t; std::cin >> t;
    if (t == 0) { int l, r, b, c; std::cin >> l >> r >> b >> c; tree.apply(l, r, {b, c}); }
    else { int p; std::cin >> p; std::cout << tree.get(p).val() << '\n'; }
  }
}

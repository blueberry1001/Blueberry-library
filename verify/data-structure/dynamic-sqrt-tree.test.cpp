#define PROBLEM "https://judge.yosupo.jp/problem/point_set_range_composite"
#include <iostream>
#include <vector>
#include <atcoder/modint>
#include "blueberry/data-structure/dynamic-sqrt-tree.hpp"
using Mint = atcoder::modint998244353;
struct Affine { Mint a, b; };
Affine op(Affine f, Affine g) { return {g.a * f.a, g.a * f.b + g.b}; }
Affine e() { return {1, 0}; }
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n, q; std::cin >> n >> q;
  std::vector<Affine> a(n);
  for (auto& f : a) { int x, y; std::cin >> x >> y; f = {x, y}; }
  blueberry::DynamicSqrtTree<Affine, op, e> tree(a);
  // Noncommutative composition checks the order after point assignment.
  while (q--) {
    int t, x, y, z; std::cin >> t >> x >> y >> z;
    if (t == 0) tree.set(x, {y, z});
    else { auto f = tree.prod(x, y); std::cout << (f.a * z + f.b).val() << '\n'; }
  }
}

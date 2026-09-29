#define PROBLEM "https://judge.yosupo.jp/problem/point_set_range_composite"
#include <iostream>
#include <vector>
#include <atcoder/segtree>
#include <atcoder/modint>
#include "blueberry/algebra/monoids.hpp"
using namespace std;
using Mint = atcoder::modint998244353;
using M = blueberry::monoid::AffineComposition<Mint>;
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int n, q; cin >> n >> q;
  vector<M::S> a(n);
  for (auto& s : a) { int b, c; cin >> b >> c; s = M::leaf(b, c); }
  atcoder::segtree<M::S, M::op, M::e> seg(a);
  // Noncommutative composition follows array order.
  while (q--) {
    int t; cin >> t;
    if (t == 0) { int p, c, d; cin >> p >> c >> d; seg.set(p, M::leaf(c, d)); }
    else { int l, r, x; cin >> l >> r >> x; auto f = seg.prod(l, r); cout << (f.a * x + f.b).val() << '\n'; }
  }
}

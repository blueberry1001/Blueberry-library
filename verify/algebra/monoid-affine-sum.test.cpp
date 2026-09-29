#define PROBLEM "https://judge.yosupo.jp/problem/range_affine_range_sum"
#include <iostream>
#include <vector>
#include <atcoder/lazysegtree>
#include <atcoder/modint>
#include "blueberry/algebra/monoids.hpp"
using namespace std;
using Mint = atcoder::modint998244353;
using M = blueberry::monoid::AffineSum<Mint>;
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int n, q; cin >> n >> q;
  vector<M::S> a(n);
  for (auto& s : a) { int x; cin >> x; s = M::leaf(x); }
  atcoder::lazy_segtree<M::S, M::op, M::e, M::F, M::mapping, M::composition, M::id> seg(a);
  // ACL composes a new affine map after the pending old map.
  while (q--) {
    int t, l, r; cin >> t >> l >> r;
    if (t == 0) { int b, c; cin >> b >> c; seg.apply(l, r, {b, c}); }
    else cout << seg.prod(l, r).sum.val() << '\n';
  }
}

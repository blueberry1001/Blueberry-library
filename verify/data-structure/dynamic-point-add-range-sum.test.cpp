#define PROBLEM "https://judge.yosupo.jp/problem/point_add_range_sum"

#include <iostream>
#include "blueberry/data-structure/dynamic-fenwick-tree.hpp"

using namespace std;

// Dense official cases exercise the same sparse online API as large-domain tests.
int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  int n, q; cin >> n >> q;
  blueberry::DynamicFenwickTree<long long, int> tree(n);
  for (int i = 0; i < n; ++i) {
    long long a; cin >> a; tree.add(i, a);
  }
  while (q--) {
    int type, l; long long r;
    cin >> type >> l >> r;
    if (type == 0) tree.add(l, r);
    else cout << tree.sum(l, static_cast<int>(r)) << '\n';
  }
}

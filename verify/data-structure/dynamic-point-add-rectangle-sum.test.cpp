#define PROBLEM "https://judge.yosupo.jp/problem/point_add_rectangle_sum"

#include <iostream>
#include "blueberry/data-structure/dynamic-fenwick-tree-2d.hpp"

using namespace std;

// Read and execute each update immediately: future positions are not registered.
int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  int n, q;
  cin >> n >> q;
  blueberry::DynamicFenwickTree2D<long long, int> tree(1000000000, 1000000000);
  for (int i = 0; i < n; ++i) {
    int x, y; long long w;
    cin >> x >> y >> w;
    tree.add(x, y, w);
  }
  while (q--) {
    int type; cin >> type;
    if (type == 0) {
      int x, y; long long w;
      cin >> x >> y >> w;
      tree.add(x, y, w);
    } else {
      int l, d, r, u; cin >> l >> d >> r >> u;
      cout << tree.sum(l, d, r, u) << '\n';
    }
  }
}

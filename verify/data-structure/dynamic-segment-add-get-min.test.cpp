#define PROBLEM "https://judge.yosupo.jp/problem/segment_add_get_min"

#include <iostream>

#include "blueberry/data-structure/dynamic-li-chao-tree.hpp"

using namespace std;

// Segment endpoints and query coordinates are consumed without collecting them.
int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  int n, q;
  cin >> n >> q;
  blueberry::DynamicLiChaoTree<> tree(-1000000000LL, 1000000001LL);
  for (int i = 0; i < n; ++i) {
    long long l, r, a, b;
    cin >> l >> r >> a >> b;
    tree.add_segment(l, r, a, b);
  }
  while (q--) {
    int type;
    long long x;
    cin >> type >> x;
    if (type == 0) {
      long long r, a, b;
      cin >> r >> a >> b;
      tree.add_segment(x, r, a, b);
    } else {
      const auto answer = tree.query(x);
      if (answer) cout << *answer << '\n';
      else cout << "INFINITY\n";
    }
  }
}

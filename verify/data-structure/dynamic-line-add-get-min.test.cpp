#define PROBLEM "https://judge.yosupo.jp/problem/line_add_get_min"

#include <iostream>

#include "blueberry/data-structure/dynamic-li-chao-tree.hpp"

using namespace std;

// Coordinates arrive online; the integer domain includes both +/- 10^9.
int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  int n, q;
  cin >> n >> q;
  blueberry::DynamicLiChaoTree<> tree(-1000000000LL, 1000000001LL);
  for (int i = 0; i < n; ++i) {
    long long a, b;
    cin >> a >> b;
    tree.add_line(a, b);
  }
  while (q--) {
    int type;
    long long x;
    cin >> type >> x;
    if (type == 0) {
      long long b;
      cin >> b;
      tree.add_line(x, b);
    } else {
      cout << *tree.query(x) << '\n';
    }
  }
}

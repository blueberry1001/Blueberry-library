#define PROBLEM "https://judge.yosupo.jp/problem/unionfind"

#include <iostream>

#include "blueberry/data-structure/disjoint-set-union.hpp"

using namespace std;

// type=0で併合し、type=1で同じ連結成分に属するかを判定する。

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  int n, q;
  cin >> n >> q;
  blueberry::DisjointSetUnion dsu(n);
  while (q--) {
    int type, u, v;
    cin >> type >> u >> v;
    if (type == 0) {
      dsu.merge(u, v);
    } else {
      cout << dsu.same(u, v) << '\n';
    }
  }
}

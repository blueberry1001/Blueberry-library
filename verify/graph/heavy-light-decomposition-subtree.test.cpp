#define PROBLEM "https://judge.yosupo.jp/problem/vertex_add_subtree_sum"

#include <atcoder/fenwicktree>

#include <iostream>
#include <vector>

#include "blueberry/graph/heavy-light-decomposition.hpp"

using namespace std;

// HLDの部分木区間を検証する。区間和の管理にはACLのFenwick Treeを使う。

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  int n, q;
  cin >> n >> q;
  vector<long long> value(n);
  for (long long& x : value) cin >> x;
  vector<vector<int>> tree(n);
  for (int v = 1; v < n; ++v) {
    int parent;
    cin >> parent;
    tree[parent].push_back(v);
  }

  const blueberry::HLD hld(tree);
  // 頂点vの値をHLDの添字in(v)へ並べ直し、ACLで区間和を管理する。
  atcoder::fenwick_tree<long long> fenwick(n);
  for (int v = 0; v < n; ++v) fenwick.add(hld.in(v), value[v]);
  while (q--) {
    int type;
    cin >> type;
    if (type == 0) {
      int v;
      long long delta;
      cin >> v >> delta;
      // 頂点番号ではなく、HLD上の添字を更新する。
      fenwick.add(hld.in(v), delta);
    } else {
      int v;
      cin >> v;
      // vを根とする部分木は、HLD上で連続した1区間になる。
      const auto [left, right] = hld.subtree_range(v);
      cout << fenwick.sum(left, right) << '\n';
    }
  }
}

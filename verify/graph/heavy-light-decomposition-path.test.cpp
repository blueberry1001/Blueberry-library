#define PROBLEM "https://judge.yosupo.jp/problem/vertex_add_path_sum"

#include <atcoder/fenwicktree>

#include <iostream>
#include <vector>

#include "blueberry/graph/heavy-light-decomposition.hpp"

using namespace std;

// HLDを検証する。区間和の補助データ構造にはACLのFenwick Treeを使う。

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  int n, q;
  cin >> n >> q;
  vector<long long> value(n);
  for (long long& x : value) cin >> x;
  vector<vector<int>> tree(n);
  for (int i = 0; i + 1 < n; ++i) {
    int u, v;
    cin >> u >> v;
    tree[u].push_back(v);
    tree[v].push_back(u);
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
      int u, v;
      cin >> u >> v;
      long long answer = 0;
      // パスをO(log N)個の半開区間に分解し、それぞれの和を足す。
      hld.vertex_query(u, v, [&](int left, int right) {
        answer += fenwick.sum(left, right);
      });
      cout << answer << '\n';
    }
  }
}

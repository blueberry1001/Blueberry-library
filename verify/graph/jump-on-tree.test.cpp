#define PROBLEM "https://judge.yosupo.jp/problem/jump_on_tree"

#include <iostream>
#include <vector>

#include "blueberry/graph/heavy-light-decomposition.hpp"

using namespace std;

// uからvへk辺進んだ頂点を求める。パスの長さを超える場合は-1を返す。

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  int n, q;
  cin >> n >> q;
  vector<vector<int>> tree(n);
  for (int i = 0; i + 1 < n; ++i) {
    int u, v;
    cin >> u >> v;
    tree[u].push_back(v);
    tree[v].push_back(u);
  }
  const blueberry::HLD hld(tree);
  while (q--) {
    int u, v, k;
    cin >> u >> v >> k;
    cout << (k <= hld.distance(u, v) ? hld.jump(u, v, k) : -1) << '\n';
  }
}

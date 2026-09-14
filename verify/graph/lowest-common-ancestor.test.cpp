#define PROBLEM "https://judge.yosupo.jp/problem/lca"

#include <iostream>
#include <vector>

#include "blueberry/graph/lowest-common-ancestor.hpp"

using namespace std;

// 根0の木をダブリングで前計算し、各頂点対のLCAをO(log N)で求める。

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  int n, q;
  cin >> n >> q;
  vector<vector<int>> tree(n);
  for (int v = 1; v < n; ++v) {
    int parent;
    cin >> parent;
    tree[parent].push_back(v);
    tree[v].push_back(parent);
  }
  const blueberry::LowestCommonAncestor lca(tree);
  while (q--) {
    int u, v;
    cin >> u >> v;
    cout << lca.lca(u, v) << '\n';
  }
}

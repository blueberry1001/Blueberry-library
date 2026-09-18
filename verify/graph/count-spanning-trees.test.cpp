#define PROBLEM "https://judge.yosupo.jp/problem/counting_spanning_tree_undirected"
#include <iostream>
#include <atcoder/modint>
#include "blueberry/graph/count-spanning-trees.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int n, m; cin >> n >> m;
  vector<pair<int, int>> edges(m);
  for (auto& [u, v] : edges) cin >> u >> v;
  cout << blueberry::count_spanning_trees<atcoder::modint998244353>(n, edges).val() << '\n';
}

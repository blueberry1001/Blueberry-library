#define PROBLEM "https://judge.yosupo.jp/problem/tree_diameter"
#include <iostream>
#include "blueberry/graph/tree-diameter.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int n; cin >> n;
  vector<vector<pair<int, long long>>> graph(n);
  for (int i = 1; i < n; ++i) {
    int u, v; long long w; cin >> u >> v >> w;
    graph[u].emplace_back(v, w); graph[v].emplace_back(u, w);
  }
  auto [length, path] = blueberry::tree_diameter(graph);
  cout << length << ' ' << path.size() << '\n';
  for (int v : path) cout << v << ' ';
  cout << '\n';
}

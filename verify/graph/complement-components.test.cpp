#define PROBLEM "https://judge.yosupo.jp/problem/connected_components_of_complement_graph"
#include <iostream>
#include "blueberry/graph/complement-components.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int n, m; cin >> n >> m;
  vector<vector<int>> graph(n);
  while (m--) { int u, v; cin >> u >> v; graph[u].push_back(v); graph[v].push_back(u); }
  auto groups = blueberry::complement_components(graph);
  cout << groups.size() << '\n';
  for (const auto& group : groups) { cout << group.size(); for (int v : group) cout << ' ' << v; cout << '\n'; }
}

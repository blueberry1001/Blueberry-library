#define PROBLEM "https://judge.yosupo.jp/problem/cycle_detection_undirected"
#include <iostream>
#include "blueberry/graph/cycle-detection.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int n, m; cin >> n >> m;
  vector<pair<int, int>> edges(m);
  for (auto& [u, v] : edges) cin >> u >> v;
  auto cycle = blueberry::find_cycle<false>(n, edges);
  if (cycle.empty()) { cout << -1 << '\n'; return 0; }
  // Recover vertex orientation by trying either endpoint of the first edge.
  vector<int> vertices;
  for (int start : {edges[cycle[0]].first, edges[cycle[0]].second}) {
    vertices.clear(); int v = start; bool valid = true;
    for (int id : cycle) {
      auto [a, b] = edges[id];
      if (v != a && v != b) { valid = false; break; }
      vertices.push_back(v); v ^= a ^ b;
    }
    if (valid && v == start) break;
  }
  cout << cycle.size() << '\n';
  for (int v : vertices) cout << v << ' ';
  cout << '\n';
  for (int id : cycle) cout << id << ' ';
  cout << '\n';
}

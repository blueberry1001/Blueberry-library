#define PROBLEM "https://judge.yosupo.jp/problem/cycle_detection"
#include <iostream>
#include "blueberry/graph/cycle-detection.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int n, m; cin >> n >> m;
  vector<pair<int, int>> edges(m);
  for (auto& [u, v] : edges) cin >> u >> v;
  // Original edge IDs in directed cycle order.
  auto cycle = blueberry::find_cycle<true>(n, edges);
  if (cycle.empty()) { cout << -1 << '\n'; return 0; }
  cout << cycle.size() << '\n';
  for (int id : cycle) cout << id << '\n';
}

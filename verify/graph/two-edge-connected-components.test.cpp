#define PROBLEM "https://judge.yosupo.jp/problem/two_edge_connected_components"

#include <iostream>
#include <utility>
#include <vector>

#include "blueberry/graph/low-link.hpp"

using namespace std;

// Parallel edges and loops retain their input edge IDs during bridge detection.
int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  int n, m;
  cin >> n >> m;
  vector<pair<int, int>> edges(m);
  for (auto& [u, v] : edges) cin >> u >> v;
  const auto groups = blueberry::LowLink(n, edges).groups();
  cout << groups.size() << '\n';
  for (const auto& group : groups) {
    cout << group.size();
    for (int v : group) cout << ' ' << v;
    cout << '\n';
  }
}

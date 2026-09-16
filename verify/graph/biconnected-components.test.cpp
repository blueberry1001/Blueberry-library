#define PROBLEM "https://judge.yosupo.jp/problem/biconnected_components"
#include <iostream>
#include <utility>
#include <vector>
#include "blueberry/graph/biconnected-components.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  int n, m;
  cin >> n >> m;
  vector<pair<int, int>> edges(m);
  for (auto& [u, v] : edges) cin >> u >> v;
  // Articulation vertices can occur in several vertex-biconnected blocks.
  blueberry::BiconnectedComponents blocks(n, edges);
  cout << blocks.groups().size() << '\n';
  for (const auto& group : blocks.groups()) {
    cout << group.size();
    for (int v : group) cout << ' ' << v;
    cout << '\n';
  }
}

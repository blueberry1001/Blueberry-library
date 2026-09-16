#define PROBLEM "https://judge.yosupo.jp/problem/eulerian_trail_undirected"
#include <iostream>
#include <utility>
#include <vector>
#include "blueberry/graph/eulerian-trail.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  int t;
  cin >> t;
  while (t--) {
    int n, m;
    cin >> n >> m;
    vector<pair<int, int>> edges(m);
    for (auto& [u, v] : edges) cin >> u >> v;
    // Each undirected edge is consumed once even though it has two incidences.
    blueberry::EulerianTrail<false> trail(n, edges);
    if (!trail.exists()) { cout << "No\n"; continue; }
    cout << "Yes\n";
    for (int v : trail.vertices()) cout << v << ' ';
    cout << '\n';
    for (int id : trail.edges()) cout << id << ' ';
    cout << '\n';
  }
}

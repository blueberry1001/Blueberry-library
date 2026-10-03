#define PROBLEM "https://judge.yosupo.jp/problem/general_matching"
#include <iostream>
#include <utility>
#include <vector>
#include "blueberry/graph/general-matching.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  int n, m;
  cin >> n >> m;
  vector<pair<int, int>> edges(m);
  for (auto& [u, v] : edges) cin >> u >> v;
  // Print each edge of the maximum matching once, including on graphs with odd cycles.
  const auto mate = blueberry::general_matching(n, edges);
  int size = 0;
  for (int v = 0; v < n; ++v) size += v < mate[v];
  cout << size << '\n';
  for (int v = 0; v < n; ++v) if (v < mate[v]) cout << v << ' ' << mate[v] << '\n';
}

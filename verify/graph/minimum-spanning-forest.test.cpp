#define PROBLEM "https://judge.yosupo.jp/problem/minimum_spanning_tree"
#include <iostream>
#include "blueberry/graph/minimum-spanning-forest.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int n, m; cin >> n >> m;
  vector<tuple<int, int, long long>> edges(m);
  for (auto& [u, v, w] : edges) cin >> u >> v >> w;
  auto [total, selected] = blueberry::minimum_spanning_forest(n, edges);
  cout << total << '\n';
  for (int id : selected) cout << id << ' ';
  cout << '\n';
}

#define PROBLEM "https://judge.yosupo.jp/problem/maximum_independent_set"
#include <iostream>
#include "blueberry/graph/maximum-independent-set.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int n, m; cin >> n >> m;
  vector<pair<int, int>> edges(m);
  for (auto& [u, v] : edges) cin >> u >> v;
  auto answer = blueberry::maximum_independent_set(n, edges);
  cout << answer.size() << '\n';
  for (int v : answer) cout << v << ' ';
  cout << '\n';
}

#define PROBLEM "https://judge.yosupo.jp/problem/bipartitematching"
#include <iostream>
#include <utility>
#include <vector>
#include "blueberry/graph/hopcroft-karp.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  int l, r, m;
  cin >> l >> r >> m;
  vector<pair<int, int>> edges(m);
  for (auto& [u, v] : edges) cin >> u >> v;
  // Return the maximum matching directly as left/right vertex pairs.
  blueberry::HopcroftKarp matching(l, r, edges);
  cout << matching.size() << '\n';
  for (auto [u, v] : matching.pairs()) cout << u << ' ' << v << '\n';
}

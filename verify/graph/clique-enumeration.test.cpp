#define PROBLEM "https://judge.yosupo.jp/problem/enumerate_cliques"
#include <iostream>
#include <atcoder/modint>
#include "blueberry/graph/clique-enumeration.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int n, m; cin >> n >> m;
  using Mint = atcoder::modint998244353;
  vector<Mint> x(n);
  for (auto& value : x) { int a; cin >> a; value = a; }
  vector<pair<int, int>> edges(m);
  for (auto& [u, v] : edges) cin >> u >> v;
  Mint answer = 0;
  blueberry::enumerate_cliques(n, edges, [&](const vector<int>& clique) {
    Mint product = 1;
    for (int v : clique) product *= x[v];
    answer += product;
  });
  cout << answer.val() << '\n';
}

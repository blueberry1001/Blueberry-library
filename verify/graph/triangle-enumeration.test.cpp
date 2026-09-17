#define PROBLEM "https://judge.yosupo.jp/problem/enumerate_triangles"
#include <iostream>
#include <atcoder/modint>
#include "blueberry/graph/triangle-enumeration.hpp"
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
  blueberry::enumerate_triangles(n, edges, [&](int a, int b, int c) { answer += x[a] * x[b] * x[c]; });
  cout << answer.val() << '\n';
}

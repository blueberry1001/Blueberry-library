#define PROBLEM "https://judge.yosupo.jp/problem/rooted_tree_isomorphism_classification"
#include <algorithm>
#include <iostream>
#include "blueberry/graph/rooted-tree-isomorphism.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int n; cin >> n;
  vector<int> parent(n, -1);
  for (int v = 1; v < n; ++v) cin >> parent[v];
  auto id = blueberry::rooted_tree_isomorphism(parent);
  cout << *max_element(id.begin(), id.end()) + 1 << '\n';
  for (int x : id) cout << x << ' ';
  cout << '\n';
}

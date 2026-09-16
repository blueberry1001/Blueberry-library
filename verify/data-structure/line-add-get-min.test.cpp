#define PROBLEM "https://judge.yosupo.jp/problem/line_add_get_min"

#include <iostream>
#include <utility>
#include <vector>

#include "blueberry/data-structure/li-chao-tree.hpp"

using namespace std;

// Query coordinates are collected first; operations retain their input order.
int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  int n, q;
  cin >> n >> q;
  vector<pair<long long, long long>> initial(n);
  for (auto& [a, b] : initial) cin >> a >> b;
  struct Query { int type; long long a, b = 0; };
  vector<Query> queries(q);
  vector<long long> xs;
  for (auto& query : queries) {
    cin >> query.type >> query.a;
    if (query.type == 0) cin >> query.b;
    else xs.push_back(query.a);
  }
  blueberry::LiChaoTree<long long> tree(xs);
  for (auto [a, b] : initial) tree.add_line(a, b);
  for (const auto& query : queries) {
    if (query.type == 0) tree.add_line(query.a, query.b);
    else cout << *tree.query(query.a) << '\n';
  }
}

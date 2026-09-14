#define PROBLEM "https://judge.yosupo.jp/problem/segment_add_get_min"

#include <iostream>
#include <vector>

#include "blueberry/data-structure/li-chao-tree.hpp"

using namespace std;

// Segment endpoints stay in the original coordinate space, with [l, r).
int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  int n, q;
  cin >> n >> q;
  struct Segment { long long l, r, a, b; };
  vector<Segment> initial(n);
  for (auto& s : initial) cin >> s.l >> s.r >> s.a >> s.b;
  struct Query { int type; long long l, r = 0, a = 0, b = 0; };
  vector<Query> queries(q);
  vector<long long> xs;
  for (auto& query : queries) {
    cin >> query.type >> query.l;
    if (query.type == 0) cin >> query.r >> query.a >> query.b;
    else xs.push_back(query.l);
  }
  blueberry::LiChaoTree<long long> tree(xs);
  for (const auto& s : initial) tree.add_segment(s.l, s.r, s.a, s.b);
  for (const auto& query : queries) {
    if (query.type == 0) {
      tree.add_segment(query.l, query.r, query.a, query.b);
    } else {
      const auto answer = tree.query(query.l);
      if (answer) cout << *answer << '\n';
      else cout << "INFINITY\n";
    }
  }
}

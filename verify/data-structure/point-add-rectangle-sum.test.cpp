#define PROBLEM "https://judge.yosupo.jp/problem/point_add_rectangle_sum"

#include <iostream>
#include <vector>

#include "blueberry/data-structure/offline-fenwick-tree-2d.hpp"

using namespace std;

// 更新予定座標を先に登録し、入力順に点加算と半開矩形和を処理する。
int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  int n, q;
  cin >> n >> q;
  struct Point { long long x, y, w; };
  struct Query { int type; long long l, d, r, u; };
  vector<Point> initial(n);
  vector<pair<long long, long long>> points;
  points.reserve(n + q);
  for (auto& p : initial) {
    cin >> p.x >> p.y >> p.w;
    points.emplace_back(p.x, p.y);
  }
  vector<Query> queries(q);
  for (auto& query : queries) {
    cin >> query.type >> query.l >> query.d >> query.r;
    if (query.type == 0) points.emplace_back(query.l, query.d);
    else cin >> query.u;
  }
  blueberry::OfflineFenwickTree2D<long long> tree(std::move(points));
  for (auto p : initial) tree.add(p.x, p.y, p.w);
  for (auto query : queries) {
    if (query.type == 0) tree.add(query.l, query.d, query.r);
    else cout << tree.sum(query.l, query.d, query.r, query.u) << '\n';
  }
}

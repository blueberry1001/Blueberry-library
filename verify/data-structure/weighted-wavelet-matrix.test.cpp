#define PROBLEM "https://judge.yosupo.jp/problem/point_add_rectangle_sum"
#include <algorithm>
#include <iostream>
#include <tuple>
#include <vector>
#include "blueberry/data-structure/weighted-wavelet-matrix.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int n, q; cin >> n >> q;
  vector<tuple<long long, long long, long long>> points(n);
  for (auto& [x, y, w] : points) cin >> x >> y >> w;
  struct Query { int type, point; long long l, d, r, u; };
  vector<Query> queries(q);
  for (auto& t : queries) {
    cin >> t.type;
    if (t.type == 0) {
      long long x, y, w; cin >> x >> y >> w;
      t.point = points.size(); points.emplace_back(x, y, w);
    } else cin >> t.l >> t.d >> t.r >> t.u;
  }
  // Register future points at zero weight; updates themselves remain online.
  vector<int> order(points.size()), position(points.size());
  for (int i = 0; i < (int)order.size(); ++i) order[i] = i;
  sort(order.begin(), order.end(), [&](int i, int j) { return get<0>(points[i]) < get<0>(points[j]); });
  vector<long long> xs, ys, weights;
  for (int i : order) {
    position[i] = xs.size();
    auto [x, y, w] = points[i]; xs.push_back(x); ys.push_back(y); weights.push_back(i < n ? w : 0);
  }
  blueberry::WeightedWaveletMatrix<long long> wm(ys, weights);
  for (const auto& t : queries) {
    if (t.type == 0) wm.add(position[t.point], get<2>(points[t.point]));
    else {
      int l = lower_bound(xs.begin(), xs.end(), t.l) - xs.begin();
      int r = lower_bound(xs.begin(), xs.end(), t.r) - xs.begin();
      cout << wm.sum(l, r, t.d, t.u) << '\n';
    }
  }
}

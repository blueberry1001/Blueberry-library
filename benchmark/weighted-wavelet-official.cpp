#include <algorithm>
#include <numeric>
#include <tuple>
#include <vector>
#include "blueberry/data-structure/weighted-wavelet-matrix.hpp"
#include "blueberry/utility/fast-io.hpp"
using namespace std;

int main() {
  blueberry::FastInput<> in;
  blueberry::FastOutput out;
  int n, q;
  if (!in.read(n, q)) return 1;
  vector<tuple<long long,long long,long long>> points(n);
  for (auto& [x,y,w] : points) if (!in.read(x,y,w)) return 1;
  struct Query { int type, point; long long l,d,r,u; };
  vector<Query> queries(q);
  for (auto& t : queries) {
    if (!in.read(t.type)) return 1;
    if (t.type == 0) {
      long long x,y,w;
      if (!in.read(x,y,w)) return 1;
      t.point = static_cast<int>(points.size());
      points.emplace_back(x,y,w);
    } else if (!in.read(t.l,t.d,t.r,t.u)) return 1;
  }
  vector<int> order(points.size()), position(points.size());
  iota(order.begin(), order.end(), 0);
  sort(order.begin(), order.end(), [&](int i,int j) { return get<0>(points[i]) < get<0>(points[j]); });
  vector<long long> xs, ys, weights;
  for (int i : order) {
    position[i] = static_cast<int>(xs.size());
    auto [x,y,w] = points[i];
    xs.push_back(x); ys.push_back(y); weights.push_back(i < n ? w : 0);
  }
  blueberry::WeightedWaveletMatrix<long long> wm(ys,weights);
  for (const auto& t : queries) {
    if (t.type == 0) wm.add(position[t.point],get<2>(points[t.point]));
    else {
      int l = lower_bound(xs.begin(),xs.end(),t.l)-xs.begin();
      int r = lower_bound(xs.begin(),xs.end(),t.r)-xs.begin();
      if (!out.writeln(wm.sum(l,r,t.d,t.u))) return 1;
    }
  }
}

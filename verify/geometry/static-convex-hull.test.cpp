#define PROBLEM "https://judge.yosupo.jp/problem/static_convex_hull"
#include <utility>
#include <vector>
#include "blueberry/geometry/convex-hull.hpp"
#include "blueberry/utility/fast-io.hpp"

int main() {
  blueberry::FastInput in;
  blueberry::FastOutput out;
  int t;
  in.read(t);
  while (t--) {
    int n;
    in.read(n);
    std::vector<std::pair<long long, long long>> points(n);
    for (auto& [x, y] : points) in.read(x, y);
    // Return each distinct extreme vertex once, in counterclockwise order.
    auto hull = blueberry::convex_hull(std::move(points));
    out.writeln(hull.size());
    for (auto [x, y] : hull) out.writeln(x, y);
  }
}

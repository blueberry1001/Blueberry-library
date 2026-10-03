#define PROBLEM "https://judge.yosupo.jp/problem/closest_pair"
#include <utility>
#include <vector>
#include "blueberry/geometry/closest-pair.hpp"
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
    // Return distinct original indices attaining the minimum distance.
    const auto [i, j] = blueberry::closest_pair(points);
    out.writeln(i, j);
  }
}

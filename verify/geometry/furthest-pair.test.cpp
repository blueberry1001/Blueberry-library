#define PROBLEM "https://judge.yosupo.jp/problem/furthest_pair"
#include <utility>
#include <vector>
#include "blueberry/geometry/furthest-pair.hpp"
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
    // Return distinct original indices attaining the maximum distance.
    auto [i, j] = blueberry::furthest_pair(points);
    out.writeln(i, j);
  }
}

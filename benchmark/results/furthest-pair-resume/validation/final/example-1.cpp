#include <cassert>
#include <utility>
#include <vector>
#include "blueberry/geometry/furthest-pair.hpp"
int main() {
  using Point = std::pair<long long, long long>;
  const std::vector<Point> points{{1, 1}, {-2, -3}, {4, 5}, {0, 0}};
  const auto before = points;
  auto [i, j] = blueberry::furthest_pair(points);
  assert(i == 1 && j == 2);  // 距離の二乗は6*6+8*8=100。
  assert(points == before);
  assert((blueberry::furthest_pair(std::vector<Point>{}) == std::pair<int, int>{-1, -1}));
  assert((blueberry::furthest_pair(std::vector<Point>{{7, -2}}) == std::pair<int, int>{-1, -1}));
  const std::vector<Point> same{{7, -2}, {7, -2}};
  assert((blueberry::furthest_pair(same) == std::pair<int, int>{0, 1}));
}

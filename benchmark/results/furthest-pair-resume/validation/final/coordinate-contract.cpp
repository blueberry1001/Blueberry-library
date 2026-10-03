#include <cstdlib>
#include "blueberry/geometry/furthest-pair.hpp"
int main(int argc, char** argv) {
  if (argc != 2) return 4;
  const int scenario = std::atoi(argv[1]);
  const long long limit = (1LL << 62) - 1;
  if (scenario == 0) {
    const std::vector<std::pair<long long, long long>> points{{-limit,-limit},{limit,limit}};
    return blueberry::furthest_pair(points) == std::pair<int,int>{0,1} ? 0 : 5;
  }
  if (scenario == 13) {
    const std::vector<std::pair<long long, long long>> points(3, {limit,-limit});
    return blueberry::furthest_pair(points) == std::pair<int,int>{0,1} ? 0 : 6;
  }
  std::vector<std::pair<long long, long long>> points(scenario <= 4 ? 1 : scenario <= 8 ? 2 : 3, {0,0});
  const int index = (scenario - 1) % 4;
  auto& coordinate = index < 2 ? points.back().first : points.back().second;
  coordinate = index % 2 == 0 ? limit + 1 : -limit - 1;
  if (scenario >= 9) points[0] = points[1] = points[2];
  (void)blueberry::furthest_pair(points);
  return 0;
}

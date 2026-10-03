#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <boost/multiprecision/cpp_int.hpp>
#include "blueberry/geometry/convex-hull.hpp"

using Point = std::pair<long long, long long>;
using Points = std::vector<Point>;
using Wide = boost::multiprecision::int256_t;
std::uint64_t seed;

static_assert(std::is_same_v<decltype(blueberry::convex_hull(
                                 std::vector<std::pair<int, int>>{})),
                             std::vector<std::pair<int, int>>>);
static_assert(std::is_same_v<decltype(blueberry::convex_hull(Points{})), Points>);

template <class Coord>
Points widened(const std::vector<std::pair<Coord, Coord>>& points) {
  return Points(points.begin(), points.end());
}

[[noreturn]] void fail(const std::string& label, const Points& input,
                      const Points& expected, const Points& actual) {
  std::cerr << "seed=" << seed << " case=" << label << '\n';
  auto print = [](const char* name, const Points& points) {
    std::cerr << name << " size=" << points.size() << '\n';
    for (const auto& [x, y] : points) std::cerr << x << ' ' << y << '\n';
  };
  print("input", input);
  print("expected", expected);
  print("actual", actual);
  std::exit(1);
}

// Wider arithmetic and supporting lines, independent of the production scan.
Wide cross(const Point& a, const Point& b, const Point& c) {
  const Wide dx1 = Wide(b.first) - Wide(a.first);
  const Wide dy1 = Wide(b.second) - Wide(a.second);
  const Wide dx2 = Wide(c.first) - Wide(a.first);
  const Wide dy2 = Wide(c.second) - Wide(a.second);
  return dx1 * dy2 - dy1 * dx2;
}

Points oracle(Points points) {
  std::sort(points.begin(), points.end());
  points.erase(std::unique(points.begin(), points.end()), points.end());
  if (points.size() <= 2) return points;

  bool collinear = true;
  for (const auto& p : points)
    if (cross(points.front(), points.back(), p) != 0) collinear = false;
  if (collinear) return {points.front(), points.back()};

  const int n = static_cast<int>(points.size());
  std::vector<int> next(n, -1);
  int edge_count = 0;
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      if (i == j) continue;
      bool edge = true;
      const auto& low = std::min(points[i], points[j]);
      const auto& high = std::max(points[i], points[j]);
      for (const auto& p : points) {
        const Wide side = cross(points[i], points[j], p);
        // Require the full boundary edge, not part of a collinear edge.
        if (side < 0 || (side == 0 && (p < low || high < p))) {
          edge = false;
          break;
        }
      }
      if (!edge) continue;
      if (next[i] != -1) fail("oracle: ambiguous successor", points, {}, {});
      next[i] = j;
      ++edge_count;
    }
  }

  Points hull;
  int current = 0;
  do {
    if (current < 0 || static_cast<int>(hull.size()) >= n)
      fail("oracle: broken boundary cycle", points, {}, hull);
    hull.push_back(points[current]);
    current = next[current];
  } while (current != 0);
  if (edge_count != static_cast<int>(hull.size()))
    fail("oracle: disconnected boundary", points, {}, hull);
  return hull;
}

template <class Coord>
void check_expected(const std::vector<std::pair<Coord, Coord>>& points,
                    const Points& expected, const std::string& label,
                    std::mt19937_64& rng) {
  auto input = points;
  auto actual = blueberry::convex_hull(input);
  if (widened(actual) != expected)
    fail(label, widened(points), expected, widened(actual));
  if (input != points)
    fail(label + ": input changed", widened(points), widened(points), widened(input));

  actual = blueberry::convex_hull(std::move(input));
  if (widened(actual) != expected)
    fail(label + ": move", widened(points), expected, widened(actual));

  input = points;
  std::shuffle(input.begin(), input.end(), rng);
  actual = blueberry::convex_hull(input);
  if (widened(actual) != expected)
    fail(label + ": permutation", widened(input), expected, widened(actual));

  input.insert(input.end(), points.begin(), points.end());
  std::shuffle(input.begin(), input.end(), rng);
  actual = blueberry::convex_hull(input);
  if (widened(actual) != expected)
    fail(label + ": duplicates", widened(input), expected, widened(actual));
}

template <class Coord>
void check(const std::vector<std::pair<Coord, Coord>>& points,
           const std::string& label, std::mt19937_64& rng) {
  check_expected(points, oracle(widened(points)), label, rng);
}

int main(int argc, char** argv) {
  const char* env = std::getenv("BLUEBERRY_RANDOM_SEED");
  seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10)
                 : env ? std::strtoull(env, nullptr, 10) : 1;
  std::mt19937_64 rng(seed);
  std::cerr << "seed=" << seed << '\n';
  constexpr long long bound = (1LL << 62) - 1;
  const std::vector<Points> boundaries{
      {},
      {{0, 0}},
      {{bound, -bound}},
      {{2, 3}, {2, 3}, {2, 3}},
      {{2, 3}, {-1, 4}},
      {{-bound, 5}, {bound, 5}, {0, 5}, {bound, 5}},
      {{5, bound}, {5, -bound}, {5, 0}, {5, -bound}},
      {{-bound, -bound}, {bound, bound}, {0, 0}, {1, 1}},
      {{-bound, bound}, {bound, -bound}, {0, 0}, {-1, 1}},
      {{-bound, -bound}, {bound, -bound}, {bound, bound}, {-bound, bound},
       {0, 0}, {0, -bound}, {bound, 0}, {0, bound}, {-bound, 0}, {-bound, -bound}},
      // Huge products cancel to a determinant of exactly -1 and +1.
      {{-bound, -bound}, {bound, bound - 1}, {bound - 1, bound - 2}},
      {{-bound, bound}, {bound, -bound + 1}, {bound - 1, -bound + 2}},
      {{-2, -2}, {-1, -2}, {0, -2}, {2, -2}, {2, 0}, {2, 2},
       {0, 2}, {-2, 2}, {-2, 0}, {0, 0}},
      {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {0, 3}, {1, 1}, {2, 1}}};
  for (std::size_t i = 0; i < boundaries.size(); ++i)
    check(boundaries[i], "boundary " + std::to_string(i), rng);

  const int low = std::numeric_limits<int>::min();
  const int high = std::numeric_limits<int>::max();
  check<int>({}, "empty int", rng);
  check<int>({{low, low}, {high, low}, {high, high}, {low, high}, {0, 0}},
             "full int range", rng);
  check<int>({{low, low}, {high, high - 1}, {high - 1, high - 2}},
             "int cancellation", rng);

  // Exhaust all subsets of a small grid, including collinear boundary points.
  for (unsigned mask = 0; mask < (1U << 9); ++mask) {
    std::vector<std::pair<int, int>> points;
    for (unsigned i = 0; i < 9; ++i)
      if ((mask >> i) & 1U)
        points.emplace_back(static_cast<int>(i / 3) - 1, static_cast<int>(i % 3) - 1);
    check(points, "grid subset " + std::to_string(mask), rng);
  }

  const std::vector<long long> endpoints{
      -bound, -bound + 1, -bound + 2, -1000000000, -1,
      0, 1, 1000000000, bound - 2, bound - 1, bound};
  std::uniform_int_distribution<long long> wide_coordinate(-bound, bound);
  std::uniform_int_distribution<int> small_coordinate(-8, 8);
  for (int trial = 0; trial < 900; ++trial) {
    const int n = static_cast<int>(rng() % 19);
    Points points;
    for (int i = 0; i < n; ++i) {
      if (trial % 3 == 0) {
        points.emplace_back(small_coordinate(rng), small_coordinate(rng));
      } else if (trial % 3 == 1) {
        points.emplace_back(endpoints[rng() % endpoints.size()],
                            endpoints[rng() % endpoints.size()]);
      } else {
        points.emplace_back(wide_coordinate(rng), wide_coordinate(rng));
      }
    }
    check(points, "random " + std::to_string(trial), rng);
    if (trial % 3 == 0) {
      const std::vector<std::pair<int, int>> narrow(points.begin(), points.end());
      check(narrow, "random int " + std::to_string(trial), rng);
    }
  }

  // Every point on this parabola is extreme; no quadratic oracle is needed.
  Points parabola;
  for (long long x = -10000; x < 10000; ++x) parabola.emplace_back(x, x * x);
  const Points expected = parabola;
  std::shuffle(parabola.begin(), parabola.end(), rng);
  check_expected(parabola, expected, "large all-extreme parabola", rng);

  Points collinear;
  for (long long x = -10000; x < 10000; ++x) collinear.emplace_back(x, -3 * x + 1);
  const Points extremes{collinear.front(), collinear.back()};
  std::reverse(collinear.begin(), collinear.end());
  check_expected(collinear, extremes, "large collinear", rng);
}

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
#include "blueberry/geometry/furthest-pair.hpp"

using Point = std::pair<long long, long long>;
using Points = std::vector<Point>;
using Wide = boost::multiprecision::int256_t;
constexpr long long bound = (1LL << 62) - 1;
std::uint64_t seed;
int cases = 0;

static_assert(std::is_same_v<decltype(blueberry::furthest_pair(
                                 std::vector<std::pair<int, int>>{})),
                             std::pair<int, int>>);
static_assert(std::is_same_v<decltype(blueberry::furthest_pair(Points{})),
                             std::pair<int, int>>);

template <class Coord>
Wide squared_distance(const std::pair<Coord, Coord>& a,
                      const std::pair<Coord, Coord>& b) {
  const Wide dx = Wide(a.first) - Wide(b.first);
  const Wide dy = Wide(a.second) - Wide(b.second);
  return dx * dx + dy * dy;
}

// Enumerate original index pairs; no hull, calipers, or production predicate.
template <class Coord>
Wide oracle(const std::vector<std::pair<Coord, Coord>>& points) {
  Wide best = -1;
  for (std::size_t i = 0; i < points.size(); ++i)
    for (std::size_t j = i + 1; j < points.size(); ++j)
      best = std::max(best, squared_distance(points[i], points[j]));
  return best;
}

template <class Coord>
[[noreturn]] void fail(const std::string& label,
                      const std::vector<std::pair<Coord, Coord>>& points,
                      const Wide& expected, std::pair<int, int> actual) {
  std::cerr << "seed=" << seed << " case=" << cases << " label=" << label
            << " expected squared distance=" << expected
            << " actual indices=" << actual.first << ',' << actual.second << '\n';
  if (actual.first >= 0 && actual.second >= 0 &&
      static_cast<std::size_t>(actual.first) < points.size() &&
      static_cast<std::size_t>(actual.second) < points.size())
    std::cerr << "actual squared distance="
              << squared_distance(points[actual.first], points[actual.second]) << '\n';
  std::cerr << "input n=" << points.size() << '\n';
  for (const auto& [x, y] : points)
    std::cerr << static_cast<long long>(x) << ' ' << static_cast<long long>(y) << '\n';
  std::exit(1);
}

template <class Coord>
void check_expected(const std::vector<std::pair<Coord, Coord>>& points,
                    const Wide& expected, const std::string& label) {
  ++cases;
  const auto before = points;
  const auto actual = blueberry::furthest_pair(points);
  if (points != before) fail(label + ": input changed", before, expected, actual);
  if (points.size() < 2) {
    if (actual != std::pair<int, int>{-1, -1})
      fail(label + ": missing sentinel", points, expected, actual);
    return;
  }
  const auto [i, j] = actual;
  if (i < 0 || i >= j || static_cast<std::size_t>(j) >= points.size())
    fail(label + ": invalid or unnormalized original indices", points, expected, actual);
  if (squared_distance(points[i], points[j]) != expected)
    fail(label + ": nonmaximum distance", points, expected, actual);
}

template <class Coord>
void check(const std::vector<std::pair<Coord, Coord>>& points,
           const std::string& label, std::mt19937_64& rng) {
  const Wide expected = oracle(points);
  check_expected(points, expected, label);

  auto shuffled = points;
  std::shuffle(shuffled.begin(), shuffled.end(), rng);
  check_expected(shuffled, expected, label + ": permutation");
  shuffled.insert(shuffled.end(), points.begin(), points.end());
  std::shuffle(shuffled.begin(), shuffled.end(), rng);
  // Duplicating a singleton creates a valid pair at distance zero.
  const Wide duplicated_expected = points.size() == 1 ? Wide(0) : expected;
  check_expected(shuffled, duplicated_expected, label + ": duplicates");

  Points reflected;
  for (const auto& [x, y] : points)
    reflected.emplace_back(-static_cast<long long>(y), -static_cast<long long>(x));
  check_expected(reflected, expected, label + ": reflection");

  // Large translations stress subtraction while retaining the same distances.
  Points translated;
  bool within_bounds = true;
  for (const auto& [x, y] : points) {
    const Wide shifted_x = Wide(x) + bound / 2;
    const Wide shifted_y = Wide(y) - bound / 2;
    if (shifted_x < -Wide(bound) || shifted_x > Wide(bound) ||
        shifted_y < -Wide(bound) || shifted_y > Wide(bound)) {
      within_bounds = false;
      break;
    }
    translated.emplace_back(shifted_x.convert_to<long long>(), shifted_y.convert_to<long long>());
  }
  if (within_bounds) check_expected(translated, expected, label + ": translation");
}

int main(int argc, char** argv) {
  const char* env = std::getenv("BLUEBERRY_RANDOM_SEED");
  seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10)
                 : env ? std::strtoull(env, nullptr, 10) : 1;
  std::mt19937_64 rng(seed);
  std::cerr << "seed=" << seed << '\n';

  const std::vector<Points> boundaries{
      {},
      {{0, 0}},
      {{bound, -bound}},
      {{4, 7}, {4, 7}},
      {{bound, bound}, {bound, bound}, {bound, bound}},
      {{4, 7}, {-1, 3}},
      {{0, 5}, {bound, 5}, {-bound, 5}, {bound, 5}},
      {{5, 0}, {5, -bound}, {5, bound}, {5, -bound}},
      {{0, 0}, {bound, bound}, {-bound, -bound}, {1, 1}, {bound, bound}},
      {{0, 0}, {-bound, bound}, {bound, -bound}, {-1, 1}},
      {{0, 0}, {bound, bound}, {-bound, bound}, {-bound, -bound}, {bound, -bound},
       {0, bound}, {bound, 0}, {0, -bound}, {-bound, 0}},
      {{-bound, -bound}, {bound, bound - 1}, {bound - 1, bound - 2}},
      {{-bound, bound}, {bound, -bound + 1}, {bound - 1, -bound + 2}},
      {{2, 3}, {-2, -3}, {2, -3}, {-2, 3}, {0, 0}, {2, 0}, {0, 3}},
      {{5, 0}, {4, 3}, {3, 4}, {0, 5}, {-3, 4}, {-4, 3},
       {-5, 0}, {-4, -3}, {-3, -4}, {0, -5}, {3, -4}, {4, -3}},
      {{1, 1}, {1, 1}, {0, 0}, {4, 4}, {1, 1}, {4, 4}, {0, 0}}};
  for (std::size_t i = 0; i < boundaries.size(); ++i)
    check(boundaries[i], "boundary " + std::to_string(i), rng);

  const int low = std::numeric_limits<int>::min();
  const int high = std::numeric_limits<int>::max();
  check<int>({}, "empty int", rng);
  check<int>({{low, low}}, "singleton int", rng);
  check<int>({{0, 0}, {low, low}, {high, high}, {low, high}, {high, low}},
             "full int range", rng);
  check<int>({{low, low}, {high, high - 1}, {high - 1, high - 2}},
             "int cancellation", rng);
  check<signed char>({{-128, -128}, {127, 127}, {127, -128}, {-128, 127}},
                     "signed char", rng);
  check<short>({{-32768, 32767}, {32767, -32768}, {0, 0}}, "short", rng);

  for (unsigned mask = 0; mask < (1U << 9); ++mask) {
    std::vector<std::pair<int, int>> points;
    for (unsigned i = 0; i < 9; ++i)
      if ((mask >> i) & 1U)
        points.emplace_back(static_cast<int>(i / 3) - 1, static_cast<int>(i % 3) - 1);
    check(points, "grid subset " + std::to_string(mask), rng);
  }
  const std::vector<std::pair<int, int>> corners{{-1, -1}, {1, -1}, {1, 1}, {-1, 1}};
  for (unsigned length = 0; length <= 4; ++length) {
    for (unsigned code = 0; code < (1U << (2 * length)); ++code) {
      std::vector<std::pair<int, int>> points;
      for (unsigned i = 0; i < length; ++i) points.push_back(corners[(code >> (2 * i)) & 3U]);
      check(points, "corner sequence " + std::to_string(length) + "/" + std::to_string(code), rng);
    }
  }

  // Official hack_uso_caliper_00.in: distance around a hull is not unimodal.
  // https://github.com/yosupo06/library-checker-problems/blob/1814c4e5205517e368bb57a8d1127eb961cfeaae/geo/furthest_pair/gen/hack_uso_caliper_00.in
  const Points trap{{0, 0}, {-18767, -43052}, {-21874, -41502},
                    {-100000, 0}, {-280, 6810}, {-189, 4855}};
  for (int transform = 0; transform < 8; ++transform) {
    for (long long scale : {1LL, bound / 100000}) {
      Points points;
      for (auto [x, y] : trap) {
        if (transform & 1) std::swap(x, y);
        if (transform & 2) x = -x;
        if (transform & 4) y = -y;
        points.emplace_back(x * scale, y * scale);
      }
      check(points, "calipers trap " + std::to_string(transform) + "/" + std::to_string(scale), rng);
    }
  }

  const std::vector<long long> endpoints{
      -bound, -bound + 1, -bound + 2, -1000000000, -1,
      0, 1, 1000000000, bound - 2, bound - 1, bound};
  std::uniform_int_distribution<long long> wide_coordinate(-bound, bound);
  std::uniform_int_distribution<int> small_coordinate(-5, 5);
  for (int trial = 0; trial < 600; ++trial) {
    const int n = static_cast<int>(rng() % 41);
    Points points;
    for (int i = 0; i < n; ++i) {
      switch (trial % 6) {
        case 0:
          points.emplace_back(small_coordinate(rng), small_coordinate(rng));
          break;
        case 1:
          points.emplace_back(endpoints[rng() % endpoints.size()],
                              endpoints[rng() % endpoints.size()]);
          break;
        case 2:
          points.emplace_back(wide_coordinate(rng), wide_coordinate(rng));
          break;
        case 3: {
          const long long t = static_cast<long long>(rng() % 2000001) - 1000000;
          points.emplace_back(3 * t + 11, -7 * t + 19);
          break;
        }
        case 4:
          points.emplace_back(bound / 3 + (i % 5) * 1000000 + small_coordinate(rng),
                              -bound / 3 + (i / 5) * 1000000 + small_coordinate(rng));
          break;
        default: {
          const long long x = static_cast<long long>(rng() % 61) - 30;
          points.emplace_back(x, rng() % 2 ? x * x : 1800 - x * x);
          break;
        }
      }
    }
    check(points, "random " + std::to_string(trial), rng);
    if (trial % 6 == 0) {
      const std::vector<std::pair<int, int>> narrow(points.begin(), points.end());
      check(narrow, "random int " + std::to_string(trial), rng);
    }
  }

  // All points lie in the disk centered at (0,K^2) with radius K^2:
  // x^2 + (x^2-K^2)^2 - K^4 = x^2(x^2+1-2K^2) <= 0.
  // Both vertical poles occur, so their squared distance 4K^4 is optimal.
  constexpr long long k = 5000;
  Points lens;
  for (long long x = -k; x <= k; ++x) {
    lens.emplace_back(x, x * x);
    lens.emplace_back(x, 2 * k * k - x * x);
  }
  const Wide diameter = Wide(4) * k * k * k * k;
  check_expected(lens, diameter, "large double parabola");
  std::shuffle(lens.begin(), lens.end(), rng);
  check_expected(lens, diameter, "large double parabola shuffled");
  lens.emplace_back(0, 0);
  lens.emplace_back(0, 2 * k * k);
  check_expected(lens, diameter, "large double parabola duplicated poles");
  std::cerr << "furthest-pair: " << cases << " cases passed\n";
}

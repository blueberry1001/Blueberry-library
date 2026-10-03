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
#include "blueberry/geometry/closest-pair.hpp"

using Point = std::pair<long long, long long>;
using Points = std::vector<Point>;
using Indices = std::pair<int, int>;
using Wide = boost::multiprecision::int256_t;
constexpr long long bound = (1LL << 62) - 1;
std::uint64_t seed;
int cases = 0;

static_assert(std::is_same_v<decltype(blueberry::closest_pair(Points{})), Indices>);
static_assert(std::is_same_v<decltype(blueberry::closest_pair(
                                 std::vector<std::pair<int, int>>{})), Indices>);

struct Expected {
  Indices indices{-1, -1};
  Wide distance = -1;
};

template <class Coord>
Wide squared_distance(const std::pair<Coord, Coord>& a,
                      const std::pair<Coord, Coord>& b) {
  const Wide dx = Wide(a.first) - Wide(b.first);
  const Wide dy = Wide(a.second) - Wide(b.second);
  return dx * dx + dy * dy;
}

// Enumerate original index pairs with wider arithmetic. No sorting, strip,
// production distance helper, or geometric pruning is shared with the library.
template <class Coord>
Expected oracle(const std::vector<std::pair<Coord, Coord>>& points) {
  Expected result;
  for (std::size_t i = 0; i < points.size(); ++i) {
    for (std::size_t j = i + 1; j < points.size(); ++j) {
      const Wide distance = squared_distance(points[i], points[j]);
      // Original-index enumeration is lexicographic, so retaining the first
      // equal distance independently implements the required tie contract.
      if (result.indices.first < 0 || distance < result.distance)
        result = {{static_cast<int>(i), static_cast<int>(j)}, distance};
    }
  }
  return result;
}

template <class Coord>
[[noreturn]] void fail(const std::string& label,
                      const std::vector<std::pair<Coord, Coord>>& points,
                      const Expected& expected, Indices actual) {
  std::cerr << "seed=" << seed << " case=" << cases << " label=" << label
            << " expected=" << expected.indices.first << ',' << expected.indices.second
            << " distance=" << expected.distance
            << " actual=" << actual.first << ',' << actual.second << '\n';
  if (actual.first >= 0 && actual.second >= 0 &&
      static_cast<std::size_t>(actual.first) < points.size() &&
      static_cast<std::size_t>(actual.second) < points.size())
    std::cerr << "actual distance="
              << squared_distance(points[actual.first], points[actual.second]) << '\n';
  std::cerr << "n=" << points.size() << '\n';
  for (const auto& [x, y] : points)
    std::cerr << static_cast<long long>(x) << ' ' << static_cast<long long>(y) << '\n';
  std::exit(1);
}

template <class Coord>
void check_expected(const std::vector<std::pair<Coord, Coord>>& points,
                    const Expected& expected, const std::string& label) {
  ++cases;
  const auto before = points;
  const auto* storage = points.data();
  const auto capacity = points.capacity();
  const auto actual = blueberry::closest_pair(points);
  if (points != before || points.data() != storage || points.capacity() != capacity)
    fail(label + ": input or storage changed", before, expected, actual);
  if (actual != expected.indices)
    fail(label + ": incorrect distance or original-index tie", points, expected, actual);
  if (points.size() < 2) return;
  if (actual.first < 0 || actual.first >= actual.second ||
      static_cast<std::size_t>(actual.second) >= points.size())
    fail(label + ": invalid index pair", points, expected, actual);
  if (squared_distance(points[actual.first], points[actual.second]) != expected.distance)
    fail(label + ": incorrect squared distance", points, expected, actual);
}

template <class Coord>
void check(const std::vector<std::pair<Coord, Coord>>& points,
           const std::string& label) {
  check_expected(points, oracle(points), label);
}

void metamorphic(const Points& points, const std::string& label,
                 std::mt19937_64& rng) {
  const auto expected = oracle(points);
  check_expected(points, expected, label);
  auto shuffled = points;
  std::shuffle(shuffled.begin(), shuffled.end(), rng);
  // A permutation preserves distance, but changes the required original indices.
  check(shuffled, label + ": permutation");
  Points reflected;
  for (const auto& [x, y] : points) reflected.emplace_back(-y, -x);
  check_expected(reflected, expected, label + ": reflection");

  for (int transform = 0; transform < 3; ++transform) {
    Points changed;
    bool valid = true;
    for (const auto& [x, y] : points) {
      Wide nx, ny;
      if (transform == 0) {
        nx = Wide(x) + bound / 2;
        ny = Wide(y) - bound / 2;
      } else if (transform == 1) {
        nx = Wide(x) - Wide(y);
        ny = Wide(x) + Wide(y);
      } else {
        nx = Wide(3) * x;
        ny = Wide(3) * y;
      }
      if (nx < -Wide(bound) || nx > Wide(bound) ||
          ny < -Wide(bound) || ny > Wide(bound)) {
        valid = false;
        break;
      }
      changed.emplace_back(nx.convert_to<long long>(), ny.convert_to<long long>());
    }
    if (valid) {
      Expected transformed = expected;
      if (points.size() >= 2 && transform != 0)
        transformed.distance *= transform == 1 ? 2 : 9;
      check_expected(changed, transformed, label + ": transform " + std::to_string(transform));
    }
  }
}

template <class Coord>
void coordinate_type_cases() {
  const Coord low = [] {
    if constexpr (std::numeric_limits<Coord>::digits <= 31)
      return std::numeric_limits<Coord>::min();
    else return static_cast<Coord>(-bound);
  }();
  const Coord high = [] {
    if constexpr (std::numeric_limits<Coord>::digits <= 31)
      return std::numeric_limits<Coord>::max();
    else return static_cast<Coord>(bound);
  }();
  check<Coord>({}, "typed empty");
  check<Coord>({{low, high}}, "typed singleton");
  check<Coord>({{low, low}, {high, high}}, "typed opposite corners");
  check<Coord>({{low, high}, {high, low}, {low, low}, {high, high}}, "typed full range");
  check<Coord>({{low, high}, {high, low}, {low, high}, {high, low}}, "typed duplicate ties");
  check<Coord>({{1, 0}, {2, 0}, {0, 0}, {3, 0}}, "typed boundary ties");
}

void exhaustive_cases(std::mt19937_64& rng) {
  Points lattice;
  for (long long x = -1; x <= 1; ++x)
    for (long long y = -1; y <= 1; ++y) lattice.emplace_back(x, y);
  // Every ordered sequence of length <=4 from the 3x3 lattice: all multisets
  // and all their distinct permutations, including duplicate groups and ties.
  unsigned count = 1;
  for (unsigned length = 0; length <= 4; ++length, count *= 9) {
    for (unsigned code = 0; code < count; ++code) {
      Points points;
      unsigned digits = code;
      for (unsigned i = 0; i < length; ++i, digits /= 9)
        points.push_back(lattice[digits % 9]);
      check(points, "ordered lattice " + std::to_string(length) + "/" + std::to_string(code));
    }
  }
  // Every multiset of length5 or6; reverse and a reproducible permutation make
  // coordinate order disagree with original-index tie order.
  for (int length : {5, 6}) {
    Points points;
    const auto enumerate = [&](auto&& self, std::size_t first) -> void {
      if (static_cast<int>(points.size()) == length) {
        check(points, "lattice multiset");
        auto permuted = points;
        std::reverse(permuted.begin(), permuted.end());
        check(permuted, "lattice multiset reversed");
        std::shuffle(permuted.begin(), permuted.end(), rng);
        check(permuted, "lattice multiset permuted");
        return;
      }
      for (std::size_t i = first; i < lattice.size(); ++i) {
        points.push_back(lattice[i]);
        self(self, i);
        points.pop_back();
      }
    };
    enumerate(enumerate, 0);
  }
}

int main(int argc, char** argv) {
  const char* env = std::getenv("BLUEBERRY_RANDOM_SEED");
  seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10)
                 : env ? std::strtoull(env, nullptr, 10) : 1;
  std::mt19937_64 rng(seed);
  std::cerr << "closest-pair seed=" << seed << '\n';

  const std::vector<Points> boundaries{
      {}, {{0, 0}}, {{bound, -bound}}, {{bound, -bound}, {bound, -bound}},
      {{-bound, -bound}, {bound, bound}},
      {{-bound, -bound}, {bound, bound}, {bound, -bound}},
      {{-bound, bound}, {bound, -bound}, {-bound, -bound}},
      // Distances differing by only one near 2^126 must not become floating ties.
      {{-bound, -bound}, {bound, -bound + 1}, {bound, bound}, {-bound, bound}},
      {{-bound, -bound}, {bound, bound}, {bound - 1, bound}},
      {{-bound, -bound}, {-bound + 1, -bound}, {bound, bound}, {bound - 1, bound}},
      {{4, 4}, {9, 9}, {9, 9}, {-5, -5}, {-5, -5}, {4, 4}},
      {{10, 10}, {0, 0}, {0, 0}, {10, 10}, {0, 0}, {10, 10}},
      // The index-minimal pair crosses the x split and lies on its closed strip
      // boundary; using strict '< best' here can return (0,2) instead of (0,1).
      {{1, 0}, {2, 0}, {0, 0}, {3, 0}},
      {{0, 1}, {0, 2}, {0, 0}, {0, 3}},
      {{0, 0}, {2, 0}, {-2, 0}, {4, 0}, {0, 2}, {2, 2}, {-2, 2}, {4, 2}},
      {{0, 0}, {3, 4}, {-3, 4}, {0, 8}, {-6, 0}, {6, 0}},
      {{1, 1}, {0, 0}, {1, 0}, {2, 2}, {0, 2}, {2, 0}, {0, 1}, {2, 1}, {1, 2}},
      {{0, 0}, {1, bound}, {2, -bound}, {3, 1}, {4, bound - 1}, {5, -bound + 1}}};
  for (std::size_t i = 0; i < boundaries.size(); ++i)
    metamorphic(boundaries[i], "boundary " + std::to_string(i), rng);
  coordinate_type_cases<signed char>();
  coordinate_type_cases<short>();
  coordinate_type_cases<int>();
  coordinate_type_cases<long>();
  coordinate_type_cases<long long>();
  exhaustive_cases(rng);

  for (int shape = 0; shape < 6; ++shape) {
    Points points;
    for (long long i = 0; i < 192; ++i) {
      if (shape == 0) points.emplace_back(0, 7 * i);
      if (shape == 1) points.emplace_back(7 * i, 0);
      if (shape == 2) points.emplace_back(3 * (i % 16), 3 * (i / 16));
      if (shape == 3) points.emplace_back(11 * i, 33 * i + (i % 3 == 0));
      if (shape == 4) points.emplace_back(bound - 13 * i, -bound + 17 * i);
      if (shape == 5) points.emplace_back((i % 2) * 5, 9 * (i / 2));
    }
    std::shuffle(points.begin(), points.end(), rng);
    metamorphic(points, "structured " + std::to_string(shape), rng);
  }

  for (int trial = 0; trial < 900; ++trial) {
    const int n = static_cast<int>(rng() % 65);
    Points points;
    for (int i = 0; i < n; ++i) {
      const auto small = [&] { return static_cast<long long>(rng() % 101) - 50; };
      long long x = small(), y = small();
      switch (trial % 7) {
        case 0: x %= 4; y %= 4; break;
        case 1: x = 0; break;
        case 2: y = 37 * x + static_cast<long long>(rng() % 3); break;
        case 3: x = bound - static_cast<long long>(rng() % 1000);
                y = -bound + static_cast<long long>(rng() % 1000); break;
        case 4: x = static_cast<long long>(rng() % (2ULL * bound + 1)) - bound;
                y = static_cast<long long>(rng() % (2ULL * bound + 1)) - bound; break;
        case 5: x = 5 * static_cast<long long>(rng() % 13);
                y = 5 * static_cast<long long>(rng() % 13); break;
        default: break;
      }
      points.emplace_back(x, y);
      if (i > 0 && trial % 7 == 6 && rng() % 3 == 0)
        points.back() = points[static_cast<std::size_t>(rng() % static_cast<unsigned>(i))];
    }
    if (trial % 17 == 0) metamorphic(points, "random " + std::to_string(trial), rng);
    else check(points, "random " + std::to_string(trial));
  }

  // Known-answer cases avoid quadratic work while exercising duplicate groups.
  check_expected(Points(4096, {bound, -bound}), {{0, 1}, Wide(0)}, "all identical");
  Points late{{bound, bound}};
  for (long long i = 0; i < 1024; ++i) late.emplace_back(i, -i);
  late.push_back(late[0]);
  check_expected(late, {{0, 1025}, Wide(0)}, "late duplicate of first original point");

  Points moved{{1, 0}, {2, 0}, {0, 0}, {3, 0}};
  const auto before = moved;
  const auto* storage = moved.data();
  const auto capacity = moved.capacity();
  const auto expected = oracle(moved);
  ++cases;
  const auto actual = blueberry::closest_pair(std::move(moved));
  if (actual != expected.indices || moved != before ||
      moved.data() != storage || moved.capacity() != capacity)
    fail("rvalue must bind without consuming caller storage", before, expected, actual);
  ++cases;
  const auto temporary = blueberry::closest_pair(Points{{1, 0}, {2, 0}, {0, 0}, {3, 0}});
  if (temporary != expected.indices) fail("temporary input", before, expected, temporary);
  std::cout << "closest-pair cases=" << cases << " seed=" << seed << '\n';
}

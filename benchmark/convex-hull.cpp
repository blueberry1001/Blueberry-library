// Independent same-contract layout comparisons; see docs/development/convex-hull.md.
#include "blueberry/geometry/convex-hull.hpp"
#include "blueberry/utility/fast-io.hpp"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <sys/resource.h>
#include <utility>
#include <vector>

using Point = std::pair<long long, long long>;
using Points = std::vector<Point>;
using Clock = std::chrono::steady_clock;

static long long ns(Clock::time_point begin, Clock::time_point end) {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
}

static __int128 cross(const Point& a, const Point& b, const Point& c) {
  const __int128 x1 = __int128(b.first) - a.first;
  const __int128 y1 = __int128(b.second) - a.second;
  const __int128 x2 = __int128(c.first) - a.first;
  const __int128 y2 = __int128(c.second) - a.second;
  return x1 * y2 - y1 * x2;
}

static void check_coords(const Points& points) {
#ifndef NDEBUG
  constexpr __int128 bound = (__int128(1) << 62) - 1;
  for (const auto& [x, y] : points) {
    assert(-bound <= __int128(x) && __int128(x) <= bound);
    assert(-bound <= __int128(y) && __int128(y) <= bound);
  }
#else
  (void)points;
#endif
}

// Coordinates are sorted in place; the output buffer is initialized once.
static void value_sort(Points& points) {
  check_coords(points);
  std::sort(points.begin(), points.end());
  points.erase(std::unique(points.begin(), points.end()), points.end());
}

static Points value_scan(const Points& points) {
  if (points.size() <= 2) return points;
  Points hull(points.size() + 1);
  std::size_t used = 0;
  for (const auto& p : points) {
    while (used >= 2 && cross(hull[used - 2], hull[used - 1], p) <= 0) --used;
    hull[used++] = p;
  }
  const auto lower = used;
  for (std::size_t i = points.size() - 1; i-- > 0;) {
    while (used > lower && cross(hull[used - 2], hull[used - 1], points[i]) <= 0)
      --used;
    hull[used++] = points[i];
  }
  hull.resize(used - 1);
  return hull;
}

static std::vector<std::size_t> index_sort(const Points& points) {
  check_coords(points);
  std::vector<std::size_t> indices(points.size());
  std::iota(indices.begin(), indices.end(), 0);
  std::sort(indices.begin(), indices.end(), [&](auto a, auto b) {
    return points[a] < points[b];
  });
  indices.erase(std::unique(indices.begin(), indices.end(), [&](auto a, auto b) {
    return points[a] == points[b];
  }), indices.end());
  return indices;
}

static Points index_scan(const Points& points, const std::vector<std::size_t>& ids) {
  if (ids.size() <= 2) {
    Points result;
    for (auto i : ids) result.push_back(points[i]);
    return result;
  }
  std::vector<std::size_t> lower, upper;
  lower.reserve(ids.size());
  upper.reserve(ids.size());
  const auto append = [&](auto& chain, auto i) {
    while (chain.size() >= 2 &&
           cross(points[chain[chain.size() - 2]], points[chain.back()], points[i]) <= 0)
      chain.pop_back();
    chain.push_back(i);
  };
  for (auto i : ids) append(lower, i);
  for (auto it = ids.rbegin(); it != ids.rend(); ++it) append(upper, *it);
  Points result;
  result.reserve(lower.size() + upper.size() - 2);
  for (std::size_t j = 0; j + 1 < lower.size(); ++j) result.push_back(points[lower[j]]);
  for (std::size_t j = 0; j + 1 < upper.size(); ++j) result.push_back(points[upper[j]]);
  return result;
}

static Points run(const std::string& variant, Points points) {
  if (variant == "blueberry") return blueberry::convex_hull(std::move(points));
  if (variant == "value-buffer") {
    value_sort(points);
    return value_scan(points);
  }
  if (variant == "index-chains") {
    auto ids = index_sort(points);
    return index_scan(points, ids);
  }
  throw std::invalid_argument("unknown variant");
}

static std::uint64_t digest(const Points& points) {
  std::uint64_t result = 1469598103934665603ULL;
  for (const auto& [x, y] : points) {
    result = (result ^ std::uint64_t(x)) * 1099511628211ULL;
    result = (result ^ std::uint64_t(y)) * 1099511628211ULL;
  }
  return result;
}

static Points generate(const std::string& workload, std::size_t n) {
  std::mt19937_64 rng(0x434f4e564558ULL);
  Points points;
  points.reserve(n);
  for (std::size_t i = 0; i < n; ++i) {
    long long x, y;
    if (workload == "duplicates") {
      x = rng() % 32; y = rng() % 32;
    } else if (workload == "all-same") {
      x = 7; y = -11;
    } else if (workload == "collinear") {
      x = static_cast<long long>(rng() % 1000000001); y = 2 * x + 3;
    } else if (workload == "parabola") {
      x = static_cast<long long>(i) - static_cast<long long>(n / 2); y = x * x;
    } else if (workload == "wide") {
      constexpr long long bound = (1LL << 62) - 1;
      x = static_cast<long long>(rng() % ((1ULL << 63) - 1)) - bound;
      y = static_cast<long long>(rng() % ((1ULL << 63) - 1)) - bound;
    } else {
      x = static_cast<long long>(rng() % 2000000001) - 1000000000;
      y = static_cast<long long>(rng() % 2000000001) - 1000000000;
    }
    points.emplace_back(x, y);
  }
  if (workload == "parabola") std::shuffle(points.begin(), points.end(), rng);
  if (workload == "sorted" || workload == "reversed") std::sort(points.begin(), points.end());
  if (workload == "reversed") std::reverse(points.begin(), points.end());
  return points;
}

static long rss_kib() {
  rusage usage{};
  if (getrusage(RUSAGE_SELF, &usage) != 0) throw std::runtime_error("getrusage failed");
  return usage.ru_maxrss;
}

static void io(const std::string& variant, const std::string& kind) {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  blueberry::FastInput input;
  blueberry::FastOutput output;
  const auto t0 = Clock::now();
  std::size_t n;
  if (kind == "fast") {
    if (!input.read(n)) throw std::runtime_error("missing size");
  } else if (!(std::cin >> n)) throw std::runtime_error("missing size");
  Points points(n);
  for (auto& [x, y] : points) {
    if (kind == "fast") {
      if (!input.read(x, y)) throw std::runtime_error("invalid point");
    } else if (!(std::cin >> x >> y)) throw std::runtime_error("invalid point");
  }
  const auto t1 = Clock::now();
  const auto hull = run(variant, std::move(points));
  const auto t2 = Clock::now();
  if (kind == "fast") {
    output.writeln(hull.size());
    for (const auto& [x, y] : hull) output.writeln(x, y);
    if (!output.flush()) throw std::runtime_error("output failed");
  } else {
    std::cout << hull.size() << '\n';
    for (const auto& [x, y] : hull) std::cout << x << ' ' << y << '\n';
    std::cout.flush();
    if (!std::cout) throw std::runtime_error("output failed");
  }
  const auto t3 = Clock::now();
  std::cerr << "{\"parse_ns\":" << ns(t0, t1) << ",\"hull_ns\":" << ns(t1, t2)
            << ",\"format_ns\":" << ns(t2, t3) << ",\"rss_kib\":" << rss_kib()
            << ",\"hull_size\":" << hull.size() << ",\"digest\":" << digest(hull) << "}\n";
}

int main(int argc, char** argv) {
  if (argc >= 2 && std::string(argv[1]) == "io" && argc == 4) {
    io(argv[2], argv[3]);
    return 0;
  }
  if (argc >= 2 && std::string(argv[1]) == "generate" && argc == 4) {
    const auto points = generate(argv[2], std::stoull(argv[3]));
    blueberry::FastOutput output;
    output.writeln(points.size());
    for (const auto& [x, y] : points) output.writeln(x, y);
    return output.flush() ? 0 : 1;
  }
  if (argc != 5) throw std::invalid_argument("variant workload n repeats, io variant fast|iostream, or generate workload n");
  const std::string variant = argv[1], workload = argv[2];
  const auto input = generate(workload, std::stoull(argv[3]));
  const int repeats = std::stoi(argv[4]);
  std::cout << "{\"input_digest\":" << digest(input) << ",\"samples\":[";
  for (int repeat = -1; repeat < repeats; ++repeat) {
    const auto t0 = Clock::now();
    const auto hull = run(variant, input);
    const auto t1 = Clock::now();
    const auto s0 = Clock::now();
    auto copied = input;
    const auto s1 = Clock::now();
    Points staged;
    auto s2 = s1;
    if (variant == "blueberry") {
      staged = blueberry::convex_hull(std::move(copied));
    } else if (variant == "value-buffer") {
      value_sort(copied);
      s2 = Clock::now();
      staged = value_scan(copied);
    } else {
      auto indices = index_sort(copied);
      s2 = Clock::now();
      staged = index_scan(copied, indices);
    }
    const auto s3 = Clock::now();
    if (hull != staged) throw std::runtime_error("staged/full mismatch");
    if (repeat >= 0) {
      if (repeat) std::cout << ',';
      std::cout << "{\"full_ns\":" << ns(t0, t1) << ",\"copy_ns\":" << ns(s0, s1)
                << ",\"preprocess_sort_ns\":";
      if (variant == "blueberry") std::cout << "null";
      else std::cout << ns(s1, s2);
      std::cout << ",\"scan_ns\":";
      if (variant == "blueberry") std::cout << "null";
      else std::cout << ns(s2, s3);
      std::cout << ",\"after_copy_ns\":" << ns(s1, s3)
                << ",\"hull_size\":" << hull.size() << ",\"digest\":" << digest(hull) << '}';
    }
  }
  std::cout << "],\"rss_kib\":" << rss_kib() << "}\n";
}

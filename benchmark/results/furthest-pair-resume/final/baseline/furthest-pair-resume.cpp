// Resumed independent comparisons. Original WIP harness/evidence remain unchanged.
// See benchmark/results/furthest-pair-resume/plan.md for stage scopes and contracts.
#include "blueberry/geometry/convex-hull.hpp"
#include "blueberry/geometry/furthest-pair.hpp"
#include "blueberry/utility/fast-io.hpp"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <limits>
#include <cstdlib>
#include <new>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <sys/resource.h>
#include <utility>
#include <vector>

#ifdef FP_ALLOCATION_DIAGNOSTIC
static bool allocation_enabled = false;
static std::uint64_t allocation_calls = 0, allocation_bytes = 0;
void* operator new(std::size_t n) {
  if (void* p = std::malloc(n ? n : 1)) {
    if (allocation_enabled) { ++allocation_calls; allocation_bytes += n; }
    return p;
  }
  throw std::bad_alloc();
}
void* operator new[](std::size_t n) { return ::operator new(n); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }
#endif

using Point = std::pair<long long, long long>;
using Points = std::vector<Point>;
using Answer = std::pair<int, int>;
using Clock = std::chrono::steady_clock;
using Wide = __int128;
struct Record { Point point; int index; };
static_assert(sizeof(Record) == 24);

static long long ns(Clock::time_point a, Clock::time_point b) {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(b - a).count();
}
static Wide cross(const Point& a, const Point& b, const Point& c) {
  const Wide x = Wide(b.first) - a.first, y = Wide(b.second) - a.second;
  const Wide u = Wide(c.first) - a.first, v = Wide(c.second) - a.second;
  return x * v - y * u;
}
static Wide distance(const Point& a, const Point& b) {
  const Wide x = Wide(b.first) - a.first, y = Wide(b.second) - a.second;
  return x * x + y * y;
}
static Answer normalize(int a, int b) { return {std::min(a, b), std::max(a, b)}; }
static void validate_input(const Points& points) {
#ifndef NDEBUG
  assert(points.size() <= std::size_t(std::numeric_limits<int>::max()));
  constexpr Wide bound = (Wide(1) << 62) - 1;
  for (const auto& [x, y] : points) {
    assert(-bound <= Wide(x) && Wide(x) <= bound);
    assert(-bound <= Wide(y) && Wide(y) <= bound);
  }
#else
  (void)points;
#endif
}

// Monotone area calipers, checking both edge endpoints and any area plateau.
template<class GetPoint>
static Answer calipers(int count, GetPoint get) {
  if (count <= 1) return {0, 0};
  if (count == 2) return {0, 1};
  Wide best = -1;
  Answer answer{0, 1};
  const auto update = [&](int a, int b) {
    const Wide d = distance(get(a), get(b));
    if (d > best) { best = d; answer = {a, b}; }
  };
  int j = 1;
  for (int i = 0; i < count; ++i) {
    const int next_i = i + 1 == count ? 0 : i + 1;
    const Wide edge_x = Wide(get(next_i).first) - get(i).first;
    const Wide edge_y = Wide(get(next_i).second) - get(i).second;
    const auto change = [&](int k) {
      const int next_k = k + 1 == count ? 0 : k + 1;
      const Wide x = Wide(get(next_k).first) - get(k).first;
      const Wide y = Wide(get(next_k).second) - get(k).second;
      return edge_x * y - edge_y * x;
    };
    while (change(j) > 0) j = j + 1 == count ? 0 : j + 1;
    update(i, j);
    update(next_i, j);
    if (change(j) == 0) {
      const int next_j = j + 1 == count ? 0 : j + 1;
      update(i, next_j);
      update(next_i, next_j);
    }
  }
  return answer;
}

template<class Value, class GetPoint>
static std::vector<Value> strict_hull(std::vector<Value> sorted, GetPoint get) {
  std::sort(sorted.begin(), sorted.end(), [&](const auto& a, const auto& b) {
    return get(a) < get(b);
  });
  sorted.erase(std::unique(sorted.begin(), sorted.end(), [&](const auto& a, const auto& b) {
    return get(a) == get(b);
  }), sorted.end());
  if (sorted.size() <= 2) return sorted;
  std::vector<Value> hull;
  hull.reserve(sorted.size() + 1);
  const auto append = [&](const Value& p, std::size_t base) {
    while (hull.size() > base &&
           cross(get(hull[hull.size() - 2]), get(hull.back()), get(p)) <= 0)
      hull.pop_back();
    hull.push_back(p);
  };
  for (const auto& p : sorted) append(p, 1);
  const auto lower = hull.size();
  for (auto it = sorted.rbegin() + 1; it != sorted.rend(); ++it) append(*it, lower);
  hull.pop_back();
  return hull;
}

struct Phases { long long construct = 0, calipers = 0, recovery = -1; };

template<bool Timed>
static Answer candidate(const std::string& variant, const Points& points, Phases* phases = nullptr) {
  // Validate even an empty/singleton call, matching the public debug contract.
  if (points.size() < 2) { validate_input(points); return {-1, -1}; }
  Clock::time_point t0{};
  if constexpr (Timed) t0 = Clock::now();
  if (variant == "blueberry") {
    // Decomposition only. Full calls below execute the intact public header.
    const auto hull = blueberry::convex_hull(points);
    Clock::time_point t1{};
    if constexpr (Timed) t1 = Clock::now();
    const auto pair = calipers(int(hull.size()), [&](int i) -> const Point& { return hull[i]; });
    Clock::time_point t2{};
    if constexpr (Timed) t2 = Clock::now();
    Answer answer{-1, -1};
    if (hull.size() == 1) answer = {0, 1};
    else {
      for (int i = 0; i < int(points.size()); ++i) {
        if (answer.first == -1 && points[i] == hull[pair.first]) answer.first = i;
        if (answer.second == -1 && points[i] == hull[pair.second]) answer.second = i;
        if (answer.first != -1 && answer.second != -1) break;
      }
      answer = normalize(answer.first, answer.second);
    }
    if constexpr (Timed) {
      const auto t3 = Clock::now();
      *phases = {ns(t0, t1), ns(t1, t2), ns(t2, t3)};
    }
    return answer;
  }
  validate_input(points);
  if (variant == "indices") {
    std::vector<int> ids(points.size());
    std::iota(ids.begin(), ids.end(), 0);
    const auto hull = strict_hull(std::move(ids), [&](int i) -> const Point& { return points[i]; });
    Clock::time_point t1{};
    if constexpr (Timed) t1 = Clock::now();
    const auto pair = calipers(int(hull.size()), [&](int i) -> const Point& { return points[hull[i]]; });
    if constexpr (Timed) *phases = {ns(t0, t1), ns(t1, Clock::now()), -1};
    return hull.size() == 1 ? Answer{0, 1} : normalize(hull[pair.first], hull[pair.second]);
  }
  if (variant == "records") {
    std::vector<Record> records;
    records.reserve(points.size());
    for (int i = 0; i < int(points.size()); ++i) records.push_back({points[i], i});
    const auto hull = strict_hull(std::move(records), [](const Record& p) -> const Point& { return p.point; });
    Clock::time_point t1{};
    if constexpr (Timed) t1 = Clock::now();
    const auto pair = calipers(int(hull.size()), [&](int i) -> const Point& { return hull[i].point; });
    if constexpr (Timed) *phases = {ns(t0, t1), ns(t1, Clock::now()), -1};
    return hull.size() == 1 ? Answer{0, 1} : normalize(hull[pair.first].index, hull[pair.second].index);
  }
  throw std::invalid_argument("unknown variant");
}

static Answer full(const std::string& variant, const Points& points) {
  if (variant == "fast-cases") {
    if (points.size() <= 2) {
      validate_input(points);
      return points.size() < 2 ? Answer{-1, -1} : Answer{0, 1};
    }
    if (std::all_of(points.begin() + 1, points.end(), [&](const auto& p) { return p == points[0]; })) {
      validate_input(points);
      return {0, 1};
    }
    return blueberry::furthest_pair(points);
  }
  return variant == "blueberry" ? blueberry::furthest_pair(points) : candidate<false>(variant, points);
}

static Wide objective(const Points& points, Answer result) {
  if (points.size() < 2) {
    if (result != Answer{-1, -1}) throw std::runtime_error("invalid empty/singleton answer");
    return 0;
  }
  if (result.first < 0 || result.first >= result.second || std::size_t(result.second) >= points.size())
    throw std::runtime_error("invalid or non-normalized original indices");
  return distance(points[result.first], points[result.second]);
}
static std::uint64_t hash_distance(std::uint64_t hash, Wide distance) {
  hash = (hash ^ std::uint64_t(distance)) * 1099511628211ULL;
  return (hash ^ std::uint64_t(distance >> 64)) * 1099511628211ULL;
}

static std::vector<Points> generate(const std::string& workload, int n) {
  std::mt19937_64 rng(0x4655525448455354ULL);
  if (workload == "tiny" || workload == "tiny0" || workload == "tiny1" || workload == "tiny2") {
    std::vector<Points> batch;
    for (int i = 0; i < n / 2; ++i) {
      const long long d = rng() % 1000000;
      const int kind = workload == "tiny" ? i % 6 : workload.back() - '0';
      switch (kind) {
        case 0: batch.emplace_back(); break;
        case 1: batch.push_back({{d, -d}}); break;
        case 2: batch.push_back({{d, 0}, {-d, 1}}); break;
        case 3: batch.push_back({{0, 0}, {d, 0}, {d, d}, {0, d}}); break;
        case 4: batch.push_back({{d, d}, {d, d}, {d, d}}); break;
        default: batch.push_back({{d, 0}, {0, 0}, {-d, 0}}); break;
      }
    }
    return batch;
  }
  Points points;
  points.reserve(n);
  for (int i = 0; i < n; ++i) {
    long long x, y;
    if (workload == "duplicates") { x = rng() % 32; y = rng() % 32; }
    else if (workload == "all-same" || workload == "late-different") { x = 17; y = -19; }
    else if (workload == "collinear") { x = rng() % 1000000001; y = 2 * x + 3; }
    else if (workload == "parabola") { x = i - n / 2; y = x * x; }
    else if (workload == "wide") {
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
  if (workload == "late-different" && !points.empty()) points.back() = {1000000000, -1000000000};
  return {std::move(points)};
}

static long rss_kib() {
  rusage usage{};
  if (getrusage(RUSAGE_SELF, &usage)) throw std::runtime_error("getrusage failed");
  return usage.ru_maxrss;
}

static void io(const std::string& variant, const std::string& kind) {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  blueberry::FastInput input;
  blueberry::FastOutput output;
  const auto start = Clock::now();
  int tests;
  if (kind == "fast") { if (!input.read(tests)) throw std::runtime_error("missing count"); }
  else if (!(std::cin >> tests)) throw std::runtime_error("missing count");
  std::vector<Points> cases(tests);
  for (auto& points : cases) {
    int n;
    if (kind == "fast") { if (!input.read(n)) throw std::runtime_error("missing size"); }
    else if (!(std::cin >> n)) throw std::runtime_error("missing size");
    points.resize(n);
    for (auto& [x, y] : points) {
      if (kind == "fast") { if (!input.read(x, y)) throw std::runtime_error("invalid point"); }
      else if (!(std::cin >> x >> y)) throw std::runtime_error("invalid point");
    }
  }
  const auto parsed = Clock::now();
  std::vector<Answer> answers;
  answers.reserve(cases.size());
  for (const auto& points : cases) answers.push_back(full(variant, points));
  const auto solved = Clock::now();
  for (const auto& [a, b] : answers) {
    if (kind == "fast") output.writeln(a, b);
    else std::cout << a << ' ' << b << '\n';
  }
  if (kind == "fast") { if (!output.flush()) throw std::runtime_error("output failed"); }
  else { std::cout.flush(); if (!std::cout) throw std::runtime_error("output failed"); }
  const auto formatted = Clock::now();
  std::uint64_t digest = 1469598103934665603ULL;
  for (std::size_t i = 0; i < cases.size(); ++i) digest = hash_distance(digest, objective(cases[i], answers[i]));
  std::cerr << "{\"parse_ns\":" << ns(start, parsed) << ",\"solve_ns\":" << ns(parsed, solved)
            << ",\"format_ns\":" << ns(solved, formatted) << ",\"rss_kib\":" << rss_kib()
            << ",\"cases\":" << cases.size() << ",\"objective_digest\":" << digest << "}\n";
}

int main(int argc, char** argv) {
  if (argc == 2 && std::string(argv[1]) == "check") {
    int checked = 0;
    for (const auto& workload : {"random", "duplicates", "all-same", "collinear", "parabola", "sorted", "reversed", "wide", "late-different", "tiny", "tiny0", "tiny1", "tiny2"}) {
      for (int n : {0, 1, 2, 5, 32, 64}) {
        for (const auto& points : generate(workload, n)) {
          Wide expected = 0;
          for (std::size_t i = 0; i < points.size(); ++i)
            for (std::size_t j = i + 1; j < points.size(); ++j)
              expected = std::max(expected, distance(points[i], points[j]));
          for (const auto& variant : {"blueberry", "indices", "records", "fast-cases"})
            if (objective(points, full(variant, points)) != expected) return 1;
          ++checked;
        }
      }
    }
    std::mt19937_64 fuzz(0x46505f46555a5a31ULL);
    for (int rep = 0; rep < 1000; ++rep) {
      Points points;
      const int n = int(fuzz() % 65);
      for (int i = 0; i < n; ++i)
        points.emplace_back(static_cast<long long>(fuzz() % 101) - 50,
                            static_cast<long long>(fuzz() % 101) - 50);
      Wide expected = 0;
      for (std::size_t i = 0; i < points.size(); ++i)
        for (std::size_t j = i + 1; j < points.size(); ++j)
          expected = std::max(expected, distance(points[i], points[j]));
      for (const auto& variant : {"blueberry", "indices", "records", "fast-cases"})
        if (objective(points, full(variant, points)) != expected) return 1;
      ++checked;
    }
    std::cout << "Checked " << checked << " cases against all-pairs distance.\n";
    return 0;
  }
  if (argc == 4 && std::string(argv[1]) == "io") { io(argv[2], argv[3]); return 0; }
  if (argc == 4 && std::string(argv[1]) == "generate") {
    const auto cases = generate(argv[2], std::stoi(argv[3]));
    blueberry::FastOutput output;
    output.writeln(cases.size());
    for (const auto& points : cases) {
      output.writeln(points.size());
      for (const auto& [x, y] : points) output.writeln(x, y);
    }
    return output.flush() ? 0 : 1;
  }
  if (argc == 3 && std::string(argv[1]) == "invalid-singleton") {
    (void)full(argv[2], {{std::numeric_limits<long long>::max(), 0}});
    return 0;
  }
  if (argc != 5 || std::string(argv[1]) != "sample")
    throw std::invalid_argument("sample variant workload n, io variant fast|iostream, or generate workload n");
  const std::string variant = argv[2], workload = argv[3];
  const auto cases = generate(workload, std::stoi(argv[4]));
  std::vector<Answer> answers(cases.size());
  std::uint64_t input_digest = 1469598103934665603ULL;
  std::size_t points_count = 0;
  for (const auto& points : cases) {
    points_count += points.size();
    input_digest = hash_distance(input_digest, points.size());
    for (const auto& [x, y] : points) {
      input_digest = (input_digest ^ std::uint64_t(x)) * 1099511628211ULL;
      input_digest = (input_digest ^ std::uint64_t(y)) * 1099511628211ULL;
    }
  }
#ifdef FP_ALLOCATION_DIAGNOSTIC
  allocation_calls = allocation_bytes = 0;
  allocation_enabled = true;
#endif
  const auto start = Clock::now();
  for (std::size_t i = 0; i < cases.size(); ++i) answers[i] = full(variant, cases[i]);
  const auto done = Clock::now();
#ifdef FP_ALLOCATION_DIAGNOSTIC
  allocation_enabled = false;
#endif
  std::uint64_t digest = 1469598103934665603ULL;
  for (std::size_t i = 0; i < cases.size(); ++i)
    digest = hash_distance(digest, objective(cases[i], answers[i]));
  Phases phases;
  const bool staged_available = workload.rfind("tiny", 0) != 0 && variant != "fast-cases";
  if (staged_available) {
    const auto staged = candidate<true>(variant, cases[0], &phases);
    if (objective(cases[0], staged) != objective(cases[0], answers[0]))
      throw std::runtime_error("staged/full objective mismatch");
  }
  // Metadata is computed outside every timed region and after full/staged calls.
  std::size_t hull_sum = 0, hull_min = std::numeric_limits<std::size_t>::max(), hull_max = 0;
  for (const auto& points : cases) {
    const auto h = blueberry::convex_hull(points).size();
    hull_sum += h; hull_min = std::min(hull_min, h); hull_max = std::max(hull_max, h);
  }
  if (cases.empty()) hull_min = 0;
  std::cout << "{\"input_digest\":" << input_digest << ",\"cases\":" << cases.size()
            << ",\"points\":" << points_count << ",\"hull_sum\":" << hull_sum
            << ",\"hull_min\":" << hull_min << ",\"hull_max\":" << hull_max
            << ",\"full_ns\":" << ns(start, done) << ",\"construct_ns\":";
  if (!staged_available) std::cout << "null"; else std::cout << phases.construct;
  std::cout << ",\"calipers_ns\":";
  if (!staged_available) std::cout << "null"; else std::cout << phases.calipers;
  std::cout << ",\"recovery_ns\":";
  if (!staged_available || phases.recovery < 0) std::cout << "null"; else std::cout << phases.recovery;
  std::cout << ",\"objective_digest\":" << digest << ",\"rss_kib\":" << rss_kib();
#ifdef FP_ALLOCATION_DIAGNOSTIC
  std::cout << ",\"allocation_calls\":" << allocation_calls << ",\"allocation_bytes\":" << allocation_bytes;
#endif
  std::cout << "}\n";
}

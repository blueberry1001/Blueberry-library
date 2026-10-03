// Independent equal-contract controls; primary-source research is recorded by the runner.
// Public API calls are intact. Instrumented control phases are separate, nonadditive runs.
#include "blueberry/geometry/closest-pair.hpp"
#include "blueberry/utility/fast-io.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <numeric>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <sys/resource.h>
#include <tuple>
#include <utility>
#include <vector>

#ifdef CP_ALLOCATIONS
static bool count_allocations = false;
static std::uint64_t allocation_calls = 0, allocation_bytes = 0;
void* operator new(std::size_t n) {
  if (void* p = std::malloc(n ? n : 1)) {
    if (count_allocations) { ++allocation_calls; allocation_bytes += n; }
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

using Coord = long long;
using Wide = __int128;
using UWide = unsigned __int128;
using Points = std::vector<std::pair<Coord, Coord>>;
using Answer = std::pair<int, int>;
using Clock = std::chrono::steady_clock;
struct Record { Coord x, y; int id; };
struct Phases { long long prepare = 0, search = 0; };
static long long elapsed(Clock::time_point a, Clock::time_point b) {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(b - a).count();
}
static Answer ids(int a, int b) { return {std::min(a, b), std::max(a, b)}; }
static Wide distance(const Record& a, const Record& b) {
  const Wide x = Wide(a.x) - b.x, y = Wide(a.y) - b.y;
  return x * x + y * y;
}
static void validate(const Points& p) {
#ifndef NDEBUG
  assert(p.size() <= std::size_t(std::numeric_limits<int>::max()));
  constexpr Wide bound = (Wide(1) << 62) - 1;
  for (const auto& [x, y] : p) {
    assert(-bound <= Wide(x) && Wide(x) <= bound);
    assert(-bound <= Wide(y) && Wide(y) <= bound);
  }
#else
  (void)p;
#endif
}
static std::vector<Record> prepare_points(const Points& p) {
  std::vector<Record> a;
  a.reserve(p.size());
  for (int i = 0; i < int(p.size()); ++i) a.push_back({p[i].first, p[i].second, i});
  std::sort(a.begin(), a.end(), [](const Record& x, const Record& y) {
    return std::tie(x.x, x.y, x.id) < std::tie(y.x, y.y, y.id);
  });
  return a;
}
struct Best {
  Wide value;
  Answer pair;
  Best(const Record& a, const Record& b) : value(distance(a, b)), pair(ids(a.id, b.id)) {}
  void consider(const Record& a, const Record& b) {
    const auto d = distance(a, b);
    const auto candidate = ids(a.id, b.id);
    if (d < value || (d == value && candidate < pair)) { value = d; pair = candidate; }
  }
};

// A direct independent algorithm implementation using per-node y-index/strip vectors.
// Original records remain x-sorted, giving a layout/control distinct from the public API.
static Answer recursive_vectors(const std::vector<Record>& a) {
  Best best(a[0], a[1]);
  const auto by_y = [&](int x, int y) {
    return std::tie(a[x].y, a[x].x, a[x].id) < std::tie(a[y].y, a[y].x, a[y].id);
  };
  const auto visit = [&](auto&& self, int l, int r) -> std::vector<int> {
    if (r - l <= 3) {
      std::vector<int> order(r - l);
      std::iota(order.begin(), order.end(), l);
      for (int i = l; i < r; ++i) for (int j = i + 1; j < r; ++j) best.consider(a[i], a[j]);
      std::sort(order.begin(), order.end(), by_y);
      return order;
    }
    const int m = l + (r - l) / 2;
    auto left = self(self, l, m), right = self(self, m, r);
    std::vector<int> merged(r - l), strip;
    strip.reserve(r - l);
    std::merge(left.begin(), left.end(), right.begin(), right.end(), merged.begin(), by_y);
    for (const int i : merged) {
      const Wide dx = Wide(a[i].x) - a[m].x;
      if (dx * dx > best.value) continue;
      for (auto it = strip.rbegin(); it != strip.rend(); ++it) {
        const Wide dy = Wide(a[i].y) - a[*it].y;
        if (dy * dy > best.value) break;
        best.consider(a[i], a[*it]);
      }
      strip.push_back(i);
    }
    return merged;
  };
  (void)visit(visit, 0, int(a.size()));
  return best.pair;
}
static Wide exact_radius(Wide squared) {
  auto r = static_cast<unsigned long long>(std::sqrt(static_cast<long double>(squared)));
  while (UWide(r) * r > UWide(squared)) --r;
  while (UWide(r + 1) * (r + 1) <= UWide(squared)) ++r;
  return Wide(r);
}
static Coord clamp_coord(Wide x) {
  return static_cast<Coord>(std::max(Wide(std::numeric_limits<Coord>::min()),
                                   std::min(Wide(std::numeric_limits<Coord>::max()), x)));
}
// Sweep x; the balanced tree orders y,x,index. Exact corrected radius includes ties.
static Answer sweep_tree(const std::vector<Record>& a) {
  Best best(a[0], a[1]);
  std::set<std::tuple<Coord, Coord, int>> active;
  std::size_t first = 0;
  for (std::size_t i = 0; i < a.size(); ++i) {
    const auto& p = a[i];
    while (first < i) {
      const Wide dx = Wide(p.x) - a[first].x;
      if (dx * dx <= best.value) break;
      active.erase({a[first].y, a[first].x, a[first].id});
      ++first;
    }
    const Wide radius = exact_radius(best.value);
    auto it = active.lower_bound({clamp_coord(Wide(p.y) - radius), std::numeric_limits<Coord>::min(), -1});
    const Wide upper = Wide(p.y) + radius;
    for (; it != active.end() && Wide(std::get<0>(*it)) <= upper; ++it) {
      const auto [y, x, id] = *it;
      best.consider(p, {x, y, id});
    }
    active.emplace(p.y, p.x, p.id);
  }
  return best.pair;
}

template<bool Timed = false>
static Answer control(const std::string& variant, const Points& p, Phases* phases = nullptr) {
  const auto begin = Timed ? Clock::now() : Clock::time_point{};
  validate(p);
  if (p.size() <= 2) return p.size() < 2 ? Answer{-1, -1} : Answer{0, 1};
  auto sorted = prepare_points(p);
  Answer repeated{int(p.size()), int(p.size())};
  for (std::size_t i = 1; i < sorted.size(); ++i)
    if (sorted[i - 1].x == sorted[i].x && sorted[i - 1].y == sorted[i].y)
      repeated = std::min(repeated, ids(sorted[i - 1].id, sorted[i].id));
  const auto prepared = Timed ? Clock::now() : Clock::time_point{};
  const Answer result = repeated.first != int(p.size()) ? repeated :
                        variant == "vectors" ? recursive_vectors(sorted) : sweep_tree(sorted);
  if constexpr (Timed) {
    phases->prepare = elapsed(begin, prepared);
    phases->search = elapsed(prepared, Clock::now());
  }
  return result;
}
static Answer full(const std::string& variant, const Points& p) {
  return variant == "blueberry" ? blueberry::closest_pair(p) : control(variant, p);
}
static Answer brute(const Points& p) {
  Answer result{-1, -1};
  UWide best = ~UWide(0);
  for (int i = 0; i < int(p.size()); ++i) for (int j = i + 1; j < int(p.size()); ++j) {
    const Wide dx = Wide(p[i].first) - p[j].first, dy = Wide(p[i].second) - p[j].second;
    const UWide x = UWide(dx < 0 ? -dx : dx), y = UWide(dy < 0 ? -dy : dy), d = x * x + y * y;
    if (d < best) { best = d; result = {i, j}; }
  }
  return result;
}
static Points generate(const std::string& kind, int n) {
  std::mt19937_64 rng(0x43504c4f53455354ULL);
  Points p;
  p.reserve(n);
  constexpr Coord bound = (1LL << 62) - 1;
  for (int i = 0; i < n; ++i) {
    Coord x, y;
    if (kind == "duplicates") { x = Coord(rng() % 128); y = Coord(rng() % 128); }
    else if (kind == "grid") { x = (i % 317) * 1000LL; y = (i / 317) * 1000LL; }
    else if (kind == "nearline") { x = i * 1000LL; y = 3 * x + Coord(rng() % 7); }
    else if (kind == "extreme") {
      x = Coord(rng() % ((1ULL << 63) - 1)) - bound;
      y = Coord(rng() % ((1ULL << 63) - 1)) - bound;
    } else { x = Coord(rng() % 2000000001) - 1000000000; y = Coord(rng() % 2000000001) - 1000000000; }
    p.emplace_back(x, y);
  }
  if (kind == "extreme" && n >= 4) { p[0] = {-bound, -bound}; p[1] = {bound, bound}; p[2] = {-bound, bound}; p[3] = {bound, -bound}; }
  std::shuffle(p.begin(), p.end(), rng);
  return p;
}
static std::uint64_t digest(const Points& p) {
  std::uint64_t h = 1469598103934665603ULL;
  for (const auto& [x, y] : p) { h = (h ^ std::uint64_t(x)) * 1099511628211ULL; h = (h ^ std::uint64_t(y)) * 1099511628211ULL; }
  return h;
}
static long rss() { rusage u{}; if (getrusage(RUSAGE_SELF, &u)) throw std::runtime_error("getrusage"); return u.ru_maxrss; }
static void sample(const std::string& variant, const std::string& kind, int n, bool diagnostic) {
  const auto p = generate(kind, n);
#ifdef CP_ALLOCATIONS
  allocation_calls = allocation_bytes = 0;
  count_allocations = true;
#endif
  const auto begin = Clock::now();
  const auto answer = full(variant, p);
  const auto finish = Clock::now();
#ifdef CP_ALLOCATIONS
  count_allocations = false;
#endif
  Phases phase;
  if (!diagnostic && variant != "blueberry" && control<true>(variant, p, &phase) != answer)
    throw std::runtime_error("staged pair mismatch");
  std::cout << "{\"input_digest\":" << digest(p) << ",\"n\":" << n << ",\"pair\":[" << answer.first << ',' << answer.second
            << "],\"full_ns\":" << elapsed(begin, finish) << ",\"prepare_ns\":";
  if (diagnostic || variant == "blueberry") std::cout << "null"; else std::cout << phase.prepare;
  std::cout << ",\"search_ns\":";
  if (diagnostic || variant == "blueberry") std::cout << "null"; else std::cout << phase.search;
  std::cout << ",\"rss_kib\":" << rss();
#ifdef CP_ALLOCATIONS
  std::cout << ",\"allocation_calls\":" << allocation_calls << ",\"allocation_bytes\":" << allocation_bytes;
#endif
  std::cout << "}\n";
}
static void io(const std::string& method) {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  blueberry::FastInput input;
  blueberry::FastOutput output;
  const auto begin = Clock::now();
  int tests = 0;
  if (method == "fast") { if (!input.read(tests)) throw std::runtime_error("count"); }
  else if (!(std::cin >> tests)) throw std::runtime_error("count");
  std::vector<Points> cases(tests);
  for (auto& p : cases) {
    int n = 0;
    if (method == "fast") { if (!input.read(n)) throw std::runtime_error("n"); }
    else if (!(std::cin >> n)) throw std::runtime_error("n");
    p.resize(n);
    for (auto& [x, y] : p) {
      if (method == "fast") { if (!input.read(x, y)) throw std::runtime_error("point"); }
      else if (!(std::cin >> x >> y)) throw std::runtime_error("point");
    }
  }
  const auto parsed = Clock::now();
  std::vector<Answer> answers;
  answers.reserve(cases.size());
  for (const auto& p : cases) answers.push_back(blueberry::closest_pair(p));
  const auto solved = Clock::now();
  for (const auto& [a, b] : answers) {
    if (method == "fast") output.writeln(a, b); else std::cout << a << ' ' << b << '\n';
  }
  if (method == "fast") { if (!output.flush()) throw std::runtime_error("output"); }
  else { std::cout.flush(); if (!std::cout) throw std::runtime_error("output"); }
  const auto formatted = Clock::now();
  std::cerr << "{\"parse_ns\":" << elapsed(begin, parsed) << ",\"solve_ns\":" << elapsed(parsed, solved)
            << ",\"format_ns\":" << elapsed(solved, formatted) << ",\"end_to_end_ns\":" << elapsed(begin, formatted)
            << ",\"cases\":" << cases.size() << "}\n";
}
int main(int argc, char** argv) {
  if (argc == 2 && std::string(argv[1]) == "check") {
    int count = 0;
    const auto check = [&](const Points& p) {
      const auto want = brute(p);
      for (const auto& variant : {"blueberry", "vectors", "sweep"}) {
        const auto got = full(variant, p);
        if (got != want) { std::cerr << variant << " case=" << count << " got=" << got.first << ',' << got.second << " want=" << want.first << ',' << want.second << '\n'; std::abort(); }
      }
      ++count;
    };
    for (int mask = 0; mask < 512; ++mask) {
      Points p;
      for (int i = 0; i < 9; ++i) if ((mask >> i) & 1) p.emplace_back(i / 3, i % 3);
      check(p); std::reverse(p.begin(), p.end()); check(p);
    }
    for (const auto& kind : {"random", "duplicates", "grid", "nearline", "extreme"})
      for (int n : {0, 1, 2, 3, 4, 7, 31, 64, 127}) check(generate(kind, n));
    constexpr Coord b = (1LL << 62) - 1;
    check({{-b, -b}, {b, b}, {-b, b}, {b, -b}});
    check({{5, 5}, {0, 0}, {5, 5}, {0, 0}});
    std::mt19937_64 rng(0x43504f5241434c45ULL);
    for (int rep = 0; rep < 1000; ++rep) {
      Points p;
      const int n = int(rng() % 65);
      for (int i = 0; i < n; ++i) p.emplace_back(Coord(rng() % 21) - 10, Coord(rng() % 21) - 10);
      std::shuffle(p.begin(), p.end(), rng); check(p);
    }
    std::cout << "{\"cases\":" << count << ",\"variants\":3,\"oracle\":\"all-pairs unsigned128, exact lexicographic indices\"}\n";
    return 0;
  }
  if (argc == 3 && std::string(argv[1]) == "invalid-singleton") { (void)full(argv[2], {{std::numeric_limits<Coord>::max(), 0}}); return 0; }
  if (argc == 3 && std::string(argv[1]) == "io") { io(argv[2]); return 0; }
  if (argc == 4 && std::string(argv[1]) == "generate") {
    const std::string kind = argv[2]; const int n = std::stoi(argv[3]);
    blueberry::FastOutput out;
    if (kind == "tiny") {
      out.writeln(n);
      for (int i = 0; i < n; ++i) { out.writeln(4); out.writeln(i, 0); out.writeln(i, 1); out.writeln(i + 1, 0); out.writeln(i + 1, 1); }
    } else { const auto p = generate(kind, n); out.writeln(1); out.writeln(n); for (const auto& [x, y] : p) out.writeln(x, y); }
    return out.flush() ? 0 : 1;
  }
  if (argc == 5 && (std::string(argv[1]) == "sample" || std::string(argv[1]) == "diagnostic")) {
    sample(argv[2], argv[3], std::stoi(argv[4]), std::string(argv[1]) == "diagnostic"); return 0;
  }
  throw std::invalid_argument("check | sample/diagnostic variant workload n | io fast/iostream | generate workload n | invalid-singleton variant");
}

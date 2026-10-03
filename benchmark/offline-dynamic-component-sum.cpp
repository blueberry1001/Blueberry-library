// Controlled storage comparison. The alternate header is generated from our
// frozen production header by the runner; only time-tree bucket storage changes.
#include "blueberry/graph/offline-dynamic-component-sum.hpp"
#include "control.hpp"
#include "blueberry/utility/fast-io.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <new>
#include <random>
#include <stdexcept>
#include <string>
#include <sys/resource.h>
#include <utility>
#include <vector>

#ifdef ODC_ALLOCATIONS
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
using Clock = std::chrono::steady_clock;
using Public = blueberry::OfflineDynamicComponentSum<long long>;
using Buckets = blueberry::OfflineDynamicComponentSumBuckets<long long>;
struct Op { int type, u, v; long long delta; };
struct Program { std::vector<long long> initial; std::vector<Op> ops; int queries = 0; };
struct Phases { long long constructor_ns, registration_ns, solve_ns, full_ns; };
static long long elapsed(Clock::time_point a, Clock::time_point b) {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(b - a).count();
}
static std::uint64_t mix(std::uint64_t h, std::uint64_t x) { return (h ^ x) * 1099511628211ULL; }
static std::uint64_t input_hash(const Program& p) {
  std::uint64_t h = mix(1469598103934665603ULL, p.initial.size());
  for (auto a : p.initial) h = mix(h, std::uint64_t(a));
  for (const auto& x : p.ops) { h = mix(h, x.type); h = mix(h, x.u); h = mix(h, x.v); h = mix(h, std::uint64_t(x.delta)); }
  return h;
}
static std::uint64_t output_hash(const std::vector<long long>& a) {
  std::uint64_t h = mix(1469598103934665603ULL, a.size());
  for (auto x : a) h = mix(h, std::uint64_t(x));
  return h;
}
static Program generate(const std::string& kind, int n, int q) {
  assert(n >= 1 || q == 0);
  Program p;
  p.initial.reserve(n); p.ops.reserve(q);
  for (int i = 0; i < n; ++i) p.initial.push_back(i % 29 - 14);
  std::mt19937_64 rng(0x4f44435f42454e43ULL);
  const auto add = [&](int type, int u, int v = 0, long long delta = 0) {
    if (int(p.ops.size()) == q) return;
    p.ops.push_back({type, u, v, delta}); p.queries += type == 3;
  };
  const auto add_random_value = [&] {
    const int vertex = int(rng() % n);
    const long long delta = static_cast<long long>(rng() % 19) - 9;
    add(2, vertex, 0, delta);
  };
  if (kind == "long") {
    for (int i = 0; i < q / 3; ++i) add(0, n == 1 ? 0 : i % (n - 1), n == 1 ? 0 : i % (n - 1) + 1);
    while (int(p.ops.size()) < q) { add_random_value(); add(3, int(rng() % n)); }
  } else if (kind == "churn") {
    while (int(p.ops.size()) < q) {
      const int u = int(rng() % n), v = n == 1 ? u : (u + 1 + int(rng() % (n - 1))) % n;
      add(0, u, v); add(3, u); add(2, v, 0, static_cast<long long>(rng() % 19) - 9); add(1, v, u); add(3, u);
    }
  } else if (kind == "updates") {
    while (int(p.ops.size()) < q) {
      for (int i = 0; i < 9; ++i) add_random_value();
      add(3, int(rng() % n));
    }
  } else {
    std::vector<int> counts(4 * n);
    while (int(p.ops.size()) < q) {
      const int id = int(rng() % counts.size()), u = id / 4, v = (u + 1 + (id % 4) * 17) % n;
      const int type = int(rng() % 10);
      if (type < 4 || (type < 6 && counts[id] == 0)) { add(0, u, v); ++counts[id]; }
      else if (type < 6) { add(1, v, u); --counts[id]; }
      else if (type < 8) add_random_value();
      else add(3, int(rng() % n));
    }
  }
  return p;
}
template<class DS>
static bool append(DS& ds, const Op& x, int& queries) {
  if (x.type == 0) ds.add_edge(x.u, x.v);
  else if (x.type == 1) return ds.remove_edge(x.u, x.v);
  else if (x.type == 2) ds.add_value(x.u, x.delta);
  else if (ds.query(x.u) != queries++) throw std::runtime_error("query ID mismatch");
  return true;
}
template<class DS>
static std::vector<long long> run(const Program& p, Phases& phase) {
  std::vector<long long> answers;
  const auto start = Clock::now();
  {
    DS ds(p.initial);
    const auto constructed = Clock::now();
    int queries = 0;
    for (const auto& x : p.ops) if (!append(ds, x, queries)) throw std::runtime_error("unexpected absent removal");
    const auto registered = Clock::now();
    answers = ds.solve();
    const auto solved = Clock::now();
    phase.constructor_ns = elapsed(start, constructed);
    phase.registration_ns = elapsed(constructed, registered);
    phase.solve_ns = elapsed(registered, solved);
  }
  phase.full_ns = elapsed(start, Clock::now());
  if (answers.size() != std::size_t(p.queries)) throw std::runtime_error("answer count mismatch");
  return answers;
}
static std::vector<long long> brute(const Program& p) {
  const int n = int(p.initial.size());
  auto values = p.initial;
  std::vector<std::vector<int>> edges(n, std::vector<int>(n));
  std::vector<long long> out;
  for (const auto& x : p.ops) {
    if (x.type == 0) { ++edges[x.u][x.v]; if (x.u != x.v) ++edges[x.v][x.u]; }
    else if (x.type == 1) {
      if (edges[x.u][x.v]) { --edges[x.u][x.v]; if (x.u != x.v) --edges[x.v][x.u]; }
    } else if (x.type == 2) values[x.u] += x.delta;
    else {
      std::vector<int> queue{x.u}; std::vector<bool> seen(n); seen[x.u] = true;
      long long sum = 0;
      for (std::size_t i = 0; i < queue.size(); ++i) {
        const int v = queue[i]; sum += values[v];
        for (int u = 0; u < n; ++u) if (edges[v][u] && !seen[u]) { seen[u] = true; queue.push_back(u); }
      }
      out.push_back(sum);
    }
  }
  return out;
}
static void check_program(const Program& p) {
  Public a(p.initial); Buckets b(p.initial);
  int qa = 0, qb = 0;
  for (std::size_t i = 0; i < p.ops.size(); ++i) {
    if (append(a, p.ops[i], qa) != append(b, p.ops[i], qb)) throw std::runtime_error("remove result mismatch");
    if (i % 17 == 0) {
      const Program prefix{p.initial, std::vector<Op>(p.ops.begin(), p.ops.begin() + i + 1), 0};
      const auto expected = brute(prefix);
      if (a.solve() != expected || b.solve() != expected) throw std::runtime_error("prefix BFS mismatch");
    }
  }
  const auto want = brute(p);
  if (a.solve() != want || b.solve() != want || a.solve() != want || b.solve() != want)
    throw std::runtime_error("BFS/repeated solve mismatch");
}
static void sample(const std::string& variant, const std::string& kind, int n, int q) {
  const auto p = generate(kind, n, q);
  Phases phases{};
#ifdef ODC_ALLOCATIONS
  allocation_calls = allocation_bytes = 0; allocation_enabled = true;
#endif
  const auto answers = variant == "public" ? run<Public>(p, phases) : run<Buckets>(p, phases);
#ifdef ODC_ALLOCATIONS
  allocation_enabled = false;
#endif
  rusage usage{}; if (getrusage(RUSAGE_SELF, &usage)) throw std::runtime_error("rss");
  std::cout << "{\"input_hash\":" << input_hash(p) << ",\"output_hash\":" << output_hash(answers)
            << ",\"n\":" << n << ",\"operations\":" << p.ops.size() << ",\"queries\":" << p.queries
            << ",\"constructor_ns\":" << phases.constructor_ns << ",\"registration_ns\":" << phases.registration_ns
            << ",\"solve_ns\":" << phases.solve_ns << ",\"full_ns\":" << phases.full_ns << ",\"rss_kib\":" << usage.ru_maxrss;
#ifdef ODC_ALLOCATIONS
  std::cout << ",\"allocation_calls\":" << allocation_calls << ",\"allocation_bytes\":" << allocation_bytes;
#endif
  std::cout << "}\n";
}
static void io(const std::string& method) {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  blueberry::FastInput input; blueberry::FastOutput output;
  const auto start = Clock::now();
  int n, q;
  if (method == "fast") { if (!input.read(n, q)) throw std::runtime_error("header"); }
  else if (!(std::cin >> n >> q)) throw std::runtime_error("header");
  Program p; p.initial.resize(n); p.ops.reserve(q);
  for (auto& a : p.initial) {
    if (method == "fast") { if (!input.read(a)) throw std::runtime_error("value"); }
    else if (!(std::cin >> a)) throw std::runtime_error("value");
  }
  for (int i = 0; i < q; ++i) {
    Op x{};
    if (method == "fast") { if (!input.read(x.type, x.u)) throw std::runtime_error("op"); }
    else if (!(std::cin >> x.type >> x.u)) throw std::runtime_error("op");
    if (x.type < 2) {
      if (method == "fast") { if (!input.read(x.v)) throw std::runtime_error("edge"); }
      else if (!(std::cin >> x.v)) throw std::runtime_error("edge");
    } else if (x.type == 2) {
      if (method == "fast") { if (!input.read(x.delta)) throw std::runtime_error("delta"); }
      else if (!(std::cin >> x.delta)) throw std::runtime_error("delta");
    }
    p.ops.push_back(x); p.queries += x.type == 3;
  }
  const auto parsed = Clock::now();
  Phases phases{}; const auto answers = run<Public>(p, phases);
  const auto solved = Clock::now();
  for (auto value : answers) { if (method == "fast") output.writeln(value); else std::cout << value << '\n'; }
  if (method == "fast") { if (!output.flush()) throw std::runtime_error("flush"); }
  else { std::cout.flush(); if (!std::cout) throw std::runtime_error("flush"); }
  const auto formatted = Clock::now();
  std::cerr << "{\"parse_ns\":" << elapsed(start, parsed) << ",\"algorithm_ns\":" << elapsed(parsed, solved)
            << ",\"format_ns\":" << elapsed(solved, formatted) << ",\"end_to_end_ns\":" << elapsed(start, formatted)
            << ",\"input_hash\":" << input_hash(p) << ",\"output_hash\":" << output_hash(answers) << "}\n";
}
struct NoDefault {
  long long value;
  NoDefault() = delete;
  explicit NoDefault(long long x) : value(x) {}
  friend NoDefault operator+(const NoDefault& a, const NoDefault& b) { return NoDefault(a.value + b.value); }
};
template<class DS>
static void check_type() {
  DS ds(std::vector<NoDefault>{NoDefault(3), NoDefault(5)});
  ds.add_edge(0, 1); ds.query(0); ds.add_value(1, NoDefault(4)); ds.query(1);
  if (!ds.remove_edge(1, 0)) throw std::runtime_error("type remove");
  ds.query(0); ds.query(1);
  const auto answer = ds.solve();
  if (answer.size() != 4 || answer[0].value != 8 || answer[1].value != 12 || answer[2].value != 3 || answer[3].value != 9)
    throw std::runtime_error("nondefault/noninverse type");
}
int main(int argc, char** argv) {
  if (argc == 2 && std::string(argv[1]) == "check") {
    check_type<blueberry::OfflineDynamicComponentSum<NoDefault>>();
    check_type<blueberry::OfflineDynamicComponentSumBuckets<NoDefault>>();
    int cases = 0;
    for (const auto& kind : {"long", "churn", "updates", "cyclic"})
      for (int n : {1, 2, 7, 16}) { check_program(generate(kind, n, 93)); ++cases; }
    check_program({{}, {}, 0}); ++cases;
    std::mt19937_64 rng(0x4f44435f4f524143ULL);
    for (int rep = 0; rep < 200; ++rep) {
      Program p; const int n = 1 + int(rng() % 12); p.initial.resize(n);
      for (auto& a : p.initial) a = static_cast<long long>(rng() % 31) - 15;
      for (int i = 0; i < 100; ++i) { const int type = int(rng() % 4); p.ops.push_back({type, int(rng() % n), int(rng() % n), static_cast<long long>(rng() % 31) - 15}); p.queries += type == 3; }
      check_program(p); ++cases;
    }
    std::cout << "{\"BFS_cases\":" << cases << ",\"repeat_solve_and_append\":true}\n"; return 0;
  }
  if (argc == 5 && std::string(argv[1]) == "compare") {
    const auto p = generate(argv[2], std::stoi(argv[3]), std::stoi(argv[4]));
    Phases a{}, b{};
    const auto expected = run<Public>(p, a), actual = run<Buckets>(p, b);
    if (expected != actual) throw std::runtime_error("full output mismatch");
    std::cout << "{\"input_hash\":" << input_hash(p) << ",\"output_hash\":" << output_hash(expected)
              << ",\"queries\":" << p.queries << "}\n";
    return 0;
  }
  if (argc == 6 && std::string(argv[1]) == "sample") { sample(argv[2], argv[3], std::stoi(argv[4]), std::stoi(argv[5])); return 0; }
  if (argc == 3 && std::string(argv[1]) == "io") { io(argv[2]); return 0; }
  if (argc == 5 && std::string(argv[1]) == "generate") {
    const auto p = generate(argv[2], std::stoi(argv[3]), std::stoi(argv[4]));
    blueberry::FastOutput out; out.writeln(p.initial.size(), p.ops.size());
    for (auto a : p.initial) out.writeln(a);
    for (const auto& x : p.ops) { if (x.type < 2) out.writeln(x.type, x.u, x.v); else if (x.type == 2) out.writeln(x.type, x.u, x.delta); else out.writeln(x.type, x.u); }
    return out.flush() ? 0 : 1;
  }
  throw std::invalid_argument("check | sample public/buckets long/churn/updates/cyclic n q | io fast/iostream | generate workload n q");
}

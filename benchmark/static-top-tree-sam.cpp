// Equal-API before/after profiles. Allocation counters are a separate build.
#include "blueberry/graph/static-top-tree.hpp"
#include "blueberry/string/suffix-automaton.hpp"
#include <atcoder/modint>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <new>
#include <random>
#include <stdexcept>
#include <string>
#include <sys/resource.h>
#include <tuple>
#include <type_traits>
#include <vector>

using Clock = std::chrono::steady_clock;
using U64 = std::uint64_t;
static U64 input_hash = 1469598103934665603ULL, checksum = input_hash;
static void mix(U64& hash, U64 value) { hash = (hash ^ value) * 1099511628211ULL; }
static long long ns(Clock::time_point a, Clock::time_point b) {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(b - a).count();
}
struct Allocation { U64 calls = 0, bytes = 0; };
static Allocation allocation;
static std::array<U64, 5> callbacks{};
#ifdef PROFILE_ALLOCATIONS
static bool counting = false;
static void* allocate(std::size_t bytes) {
  if (counting) { ++allocation.calls; allocation.bytes += bytes; }
  if (void* pointer = std::malloc(bytes ? bytes : 1)) return pointer;
  throw std::bad_alloc();
}
void* operator new(std::size_t bytes) { return allocate(bytes); }
void* operator new[](std::size_t bytes) { return allocate(bytes); }
void operator delete(void* pointer) noexcept { std::free(pointer); }
void operator delete[](void* pointer) noexcept { std::free(pointer); }
void operator delete(void* pointer, std::size_t) noexcept { std::free(pointer); }
void operator delete[](void* pointer, std::size_t) noexcept { std::free(pointer); }
#endif
static void begin_counting() {
  allocation = {};
  callbacks.fill(0);
#ifdef PROFILE_ALLOCATIONS
  counting = true;
#endif
}
static Allocation end_counting() {
#ifdef PROFILE_ALLOCATIONS
  counting = false;
#endif
  return allocation;
}
static void count_callback(int index) {
#ifdef PROFILE_ALLOCATIONS
  ++callbacks[index];
#else
  (void)index;
#endif
}

static long rss_kib() {
  rusage usage{};
  if (getrusage(RUSAGE_SELF, &usage)) throw std::runtime_error("getrusage failed");
  return usage.ru_maxrss;
}
static void allocation_json(const char* name, Allocation data) {
  std::cout << ",\"" << name << "\":{\"calls\":" << data.calls << ",\"bytes\":" << data.bytes << '}';
}
static void callback_json(const char* name, const std::array<U64, 5>& data) {
  std::cout << ",\"" << name << "\":[";
  for (std::size_t i = 0; i < data.size(); ++i) std::cout << (i ? "," : "") << data[i];
  std::cout << ']';
}
static void common_json() {
  std::cout << ",\"input_hash\":" << input_hash << ",\"checksum\":" << checksum
            << ",\"rss_kib\":" << rss_kib();
#ifdef PROFILE_ALLOCATIONS
  std::cout << ",\"instrumented\":true";
#else
  std::cout << ",\"instrumented\":false";
#endif
}

template<class Scalar> struct Affine { Scalar a, b; };
template<class Scalar> static Affine<Scalar> vertex(Affine<Scalar> v, Scalar light) {
  count_callback(0); return {v.a, v.a * light + v.b};
}
template<class Scalar> static Scalar edge(Affine<Scalar> path) { count_callback(1); return path.b; }
template<class Scalar> static Affine<Scalar> compress(Affine<Scalar> a, Affine<Scalar> b) {
  count_callback(2); return {a.a * b.a, a.a * b.b + a.b};
}
template<class Scalar> static Scalar rake(Scalar a, Scalar b) { count_callback(3); return a + b; }
template<class Scalar> static Scalar identity() { count_callback(4); return 0; }
static U64 value(U64 x) { return x; }
static U64 value(atcoder::modint998244353 x) { return x.val(); }

template<class Scalar>
static void tree_profile(const std::string& shape, int n, int q) {
  std::mt19937_64 rng(0x544f5054524545ULL);
  std::vector<std::vector<int>> graph(n);
  for (int v = 1; v < n; ++v) {
    int parent;
    if (shape == "path") parent = v - 1;
    else if (shape == "star") parent = 0;
    else if (shape == "balanced") parent = (v - 1) / 2;
    else if (shape == "broom") parent = v < n / 2 ? v - 1 : std::max(0, n / 2 - 1);
    else if (shape == "random") parent = rng() % v;
    else throw std::invalid_argument("unknown tree shape");
    graph[v].push_back(parent); graph[parent].push_back(v);
    mix(input_hash, parent);
  }
  std::vector<Affine<Scalar>> coefficients(n);
  for (auto& coefficient : coefficients) {
    const U64 a = rng() % 11 + 1, b = rng() % 101;
    coefficient = {a, b}; mix(input_hash, a); mix(input_hash, b);
  }
  struct Update { int vertex; Affine<Scalar> coefficient; };
  std::vector<Update> updates(q);
  for (int i = 0; i < q; ++i) {
    const int v = i % 16 == 0 ? 0 : i % 16 == 1 ? n - 1 : rng() % n;
    const U64 a = rng() % 11 + 1, b = rng() % 101;
    updates[i] = {v, {a, b}}; mix(input_hash, v); mix(input_hash, a); mix(input_hash, b);
  }
  using Tree = blueberry::StaticTopTree<Affine<Scalar>, Affine<Scalar>, Scalar,
      vertex<Scalar>, edge<Scalar>, compress<Scalar>, rake<Scalar>, identity<Scalar>>;
  begin_counting();
  const auto start = Clock::now();
  Tree tree(graph, coefficients);
  const auto built = Clock::now();
  const auto build_allocations = end_counting();
  const auto build_callbacks = callbacks;
  mix(checksum, value(tree.all_prod().b));
  begin_counting();
  const auto operations_start = Clock::now();
  for (const auto& update : updates) {
    tree.set(update.vertex, update.coefficient);
    mix(checksum, value(tree.all_prod().b));
  }
  const auto done = Clock::now();
  const auto operation_allocations = end_counting();
  const auto operation_callbacks = callbacks;
  std::cout << "{\"family\":\"tree\",\"shape\":\"" << shape << "\",\"n\":" << n << ",\"q\":" << q
            << ",\"build_ns\":" << ns(start, built) << ",\"operations_ns\":" << ns(operations_start, done);
  common_json();
  allocation_json("build_allocations", build_allocations);
  allocation_json("operation_allocations", operation_allocations);
  callback_json("build_callbacks", build_callbacks);
  callback_json("operation_callbacks", operation_callbacks);
  std::cout << "}\n";
}

template<class Symbol>
static void sam_profile(const std::string& shape, int n, int q) {
  std::mt19937_64 rng(0x535546464958ULL);
  const auto symbol = [](int code) -> Symbol {
    if constexpr (std::is_same_v<Symbol, char>) return char('a' + code);
    else return (code - 16) * 100003;
  };
  std::vector<Symbol> text(n), other(n);
  int alphabet = shape == "random26" ? 26 : shape == "int" ? 32 : 4;
  for (int i = 0; i < n; ++i) {
    int code;
    if (shape == "equal") code = 0;
    else if (shape == "periodic") code = int(std::string("abacaba")[i % 7] - 'a');
    else code = rng() % alphabet;
    text[i] = symbol(code);
    other[i] = i % 4 ? text[i] : symbol(rng() % alphabet);
    mix(input_hash, U64(text[i])); mix(input_hash, U64(other[i]));
  }
  std::vector<std::vector<Symbol>> patterns(q);
  for (int i = 0; i < q; ++i) {
    const int length = std::min(16, n);
    const int begin = rng() % (n - length + 1);
    patterns[i].assign(text.begin() + begin, text.begin() + begin + length);
    if (i % 3 == 0) patterns[i][0] = symbol(100);
    for (auto x : patterns[i]) mix(input_hash, U64(x));
  }
  const std::vector<Symbol> present(text.begin(), text.begin() + std::min(8, n));
  begin_counting();
  const auto start = Clock::now();
  blueberry::SuffixAutomaton<Symbol> sam(text);
  const auto built = Clock::now();
  const auto build_allocations = end_counting();
  begin_counting();
  const auto counts_start = Clock::now();
  mix(checksum, sam.count(present));
  const auto counts_done = Clock::now();
  const auto count_allocations = end_counting();
  begin_counting();
  const auto queries_start = Clock::now();
  for (const auto& pattern : patterns) {
    mix(checksum, sam.count(pattern)); mix(checksum, sam.contains(pattern));
  }
  const auto queries_done = Clock::now();
  const auto query_allocations = end_counting();
  begin_counting();
  const auto lcs_start = Clock::now();
  const auto [a, b, length] = sam.longest_common_substring(other);
  const auto lcs_done = Clock::now();
  const auto lcs_allocations = end_counting();
  mix(checksum, a); mix(checksum, b); mix(checksum, length);
  mix(checksum, sam.states()); mix(checksum, sam.distinct_substrings());
  std::cout << "{\"family\":\"sam\",\"shape\":\"" << shape << "\",\"n\":" << n << ",\"q\":" << q
            << ",\"states\":" << sam.states() << ",\"clones\":" << sam.states() - n - 1
            << ",\"build_ns\":" << ns(start, built) << ",\"counts_ns\":" << ns(counts_start, counts_done)
            << ",\"queries_ns\":" << ns(queries_start, queries_done) << ",\"lcs_ns\":" << ns(lcs_start, lcs_done);
  common_json();
  allocation_json("build_allocations", build_allocations);
  allocation_json("count_allocations", count_allocations);
  allocation_json("query_allocations", query_allocations);
  allocation_json("lcs_allocations", lcs_allocations);
  std::cout << "}\n";
}

int main(int argc, char** argv) {
  if (argc != 6) throw std::invalid_argument("tree|sam shape scalar n q");
  const std::string family = argv[1], shape = argv[2], scalar = argv[3];
  const int n = std::stoi(argv[4]), q = std::stoi(argv[5]);
  if (n < 1 || q < 1) throw std::invalid_argument("n,q must be positive");
  if (family == "tree") {
    if (scalar == "u64") tree_profile<U64>(shape, n, q);
    else if (scalar == "mod998") tree_profile<atcoder::modint998244353>(shape, n, q);
    else throw std::invalid_argument("unknown scalar");
  } else if (family == "sam") {
    if (shape == "int") sam_profile<int>(shape, n, q);
    else sam_profile<char>(shape, n, q);
  } else throw std::invalid_argument("unknown family");
}

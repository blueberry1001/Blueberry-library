// Final production-header profiles, including DynamicFenwick mixed operations.
// Build with PROFILE_ALLOCATIONS only for separate diagnostics.
#include "blueberry/data-structure/dynamic-fenwick-tree.hpp"
#include "blueberry/data-structure/persistent-segment-tree.hpp"
#include <atcoder/modint>

#include <algorithm>
#include <bit>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <sys/resource.h>
#include <unordered_map>
#include <vector>

using U64 = std::uint64_t;
struct SurveyResult { std::map<std::string, U64> values; };
static U64 mix(U64 x) {
  x += 0x9e3779b97f4a7c15ULL;
  x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
  x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
  return x ^ (x >> 31);
}
struct RNG { U64 x; U64 next() { return x = mix(x); } };
static void ensure(bool yes, const char* message) {
  if (!yes) throw std::runtime_error(message);
}
template<class Fn> static U64 timed(Fn&& fn) {
  const auto begin = std::chrono::steady_clock::now();
  fn();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::steady_clock::now() - begin).count();
}
struct Counts { U64 calls = 0, bytes = 0, adds = 0, subtracts = 0, combines = 0; };
static Counts counts;
static bool counting = false;
#ifdef PROFILE_ALLOCATIONS
static void* allocate(std::size_t size) {
  if (counting) { ++counts.calls; counts.bytes += size; }
  if (void* p = std::malloc(size ? size : 1)) return p;
  throw std::bad_alloc();
}
void* operator new(std::size_t size) { return allocate(size); }
void* operator new[](std::size_t size) { return allocate(size); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }
#endif
static void count(U64 Counts::*field) {
#ifdef PROFILE_ALLOCATIONS
  if (counting) ++(counts.*field);
#else
  (void)field;
#endif
}
template<class Fn> static void stage(SurveyResult& result, const std::string& name, Fn&& fn) {
  counts = {}; counting = true;
  const U64 elapsed = timed(fn);
  counting = false;
  result.values[name + "_ns"] = elapsed;
#ifdef PROFILE_ALLOCATIONS
  result.values[name + "_allocations"] = counts.calls;
  result.values[name + "_allocated_bytes"] = counts.bytes;
  result.values[name + "_adds"] = counts.adds;
  result.values[name + "_subtracts"] = counts.subtracts;
  result.values[name + "_combines"] = counts.combines;
#endif
}
static U64 scalar_value(U64 x) { return x; }
static U64 scalar_value(atcoder::modint998244353 x) { return x.val(); }
template<class Scalar> struct Number {
  Scalar x{};
  Number() = default;
  explicit Number(U64 value) : x(value) {}
  Number& operator+=(const Number& b) { count(&Counts::adds); x += b.x; return *this; }
  friend Number operator-(const Number& a, const Number& b) {
    count(&Counts::subtracts); Number r; r.x = a.x - b.x; return r;
  }
};

#include "representative-dense.hpp"

template<class Scalar>
static SurveyResult dynamic_profile(const std::string& shape, int n, int q, bool check) {
  using Value = Number<Scalar>;
  const long long domain = check ? 1023 : (1LL << 40) - 1;
  RNG rng{0x44594e46454eULL};
  std::vector<long long> positions(n), queries(q), range_l(q / 8), range_r(q / 8);
  std::vector<U64> values(n), deltas(q / 4);
  std::vector<long long> update_p(q / 4);
  U64 input = 0, checksum = 0, old_lookups = 0, new_lookups = 0;
  const auto coordinate = [&]() -> long long {
    if (shape == "broad") return rng.next() % domain;
    if (shape == "hotspot") return rng.next() % std::min<long long>(domain, 4096);
    if (shape == "boundary") {
      int k = rng.next() % (check ? 9 : 39) + 1;
      return ((1LL << k) - 1 + static_cast<long long>(rng.next() % 3) - 1) % domain;
    }
    throw std::invalid_argument("dynamic shape");
  };
  for (int i = 0; i < n; ++i) {
    positions[i] = coordinate(); values[i] = rng.next() % 1000;
    input = mix(input ^ positions[i]); input = mix(input ^ values[i]);
  }
  for (int i = 0; i < q; ++i) {
    queries[i] = i % 4 ? positions[rng.next() % n] : coordinate();
    input = mix(input ^ queries[i]);
    const U64 p = queries[i];
    old_lookups += std::popcount(p) + std::popcount(p + 1);
    new_lookups += 1 + std::countr_zero(p + 1);
  }
  for (int i = 0; i < q / 4; ++i) {
    update_p[i] = coordinate(); deltas[i] = rng.next() % 1000;
    input = mix(input ^ update_p[i]); input = mix(input ^ deltas[i]);
  }
  for (int i = 0; i < q / 8; ++i) {
    range_l[i] = coordinate(); range_r[i] = coordinate();
    if (range_l[i] > range_r[i]) std::swap(range_l[i], range_r[i]);
    input = mix(input ^ range_l[i]); input = mix(input ^ range_r[i]);
  }
  struct Mixed { long long p; U64 delta; bool add; };
  std::vector<Mixed> mixed;
  std::vector<long long> known_positions = positions;
  U64 mixed_old_lookups = 0, mixed_new_lookups = 0;
  for (int i = 0; i < q; ++i) {
    Mixed operation{};
    operation.add = i % 5 == 0;
    if (operation.add) {
      operation.p = coordinate(); operation.delta = rng.next() % 1000;
      known_positions.push_back(operation.p);
    } else {
      // Observe every latest add once, then combine known and fresh coordinates.
      operation.p = i % 5 == 1 ? known_positions.back()
                    : i % 4 ? known_positions[rng.next() % known_positions.size()]
                            : coordinate();
      const U64 p = operation.p;
      mixed_old_lookups += std::popcount(p) + std::popcount(p + 1);
      mixed_new_lookups += 1 + std::countr_zero(p + 1);
    }
    input = mix(input ^ operation.p); input = mix(input ^ operation.delta);
    input = mix(input ^ operation.add);
    mixed.push_back(operation);
  }
  SurveyResult out;
  std::unique_ptr<blueberry::DynamicFenwickTree<Value>> tree;
  stage(out, "build", [&] { tree = std::make_unique<blueberry::DynamicFenwickTree<Value>>(domain); });
  stage(out, "initialization", [&] { for (int i = 0; i < n; ++i) tree->add(positions[i], Value(values[i])); });
  stage(out, "update", [&] { for (int i = 0; i < q / 4; ++i) tree->add(update_p[i], Value(deltas[i])); });
  stage(out, "get", [&] { for (long long p : queries) checksum = mix(checksum ^ scalar_value(tree->get(p).x)); });
  stage(out, "range", [&] { for (int i = 0; i < q / 8; ++i) checksum = mix(checksum ^ scalar_value(tree->sum(range_l[i], range_r[i]).x)); });
  if (check) {
    std::vector<Scalar> oracle(domain);
    for (int i = 0; i < n; ++i) oracle[positions[i]] += Scalar(values[i]);
    for (int i = 0; i < q / 4; ++i) oracle[update_p[i]] += Scalar(deltas[i]);
    for (long long p = 0; p < domain; ++p) ensure(tree->get(p).x == oracle[p], "dynamic point oracle");
    for (int i = 0; i < q / 8; ++i) {
      Scalar expected{};
      for (long long p = range_l[i]; p < range_r[i]; ++p) expected += oracle[p];
      ensure(tree->sum(range_l[i], range_r[i]).x == expected, "dynamic range oracle");
    }
  }
  U64 mixed_checksum = 0;
  stage(out, "mixed", [&] {
    for (const Mixed& operation : mixed) {
      if (operation.add) tree->add(operation.p, Value(operation.delta));
      else mixed_checksum = mix(mixed_checksum ^ scalar_value(tree->get(operation.p).x));
    }
  });
  if (check) {
    std::vector<Scalar> oracle(domain);
    for (int i = 0; i < n; ++i) oracle[positions[i]] += Scalar(values[i]);
    for (int i = 0; i < q / 4; ++i) oracle[update_p[i]] += Scalar(deltas[i]);
    U64 expected_checksum = 0;
    for (const Mixed& operation : mixed) {
      if (operation.add) oracle[operation.p] += Scalar(operation.delta);
      else expected_checksum = mix(expected_checksum ^ scalar_value(oracle[operation.p]));
    }
    ensure(mixed_checksum == expected_checksum, "dynamic mixed oracle");
    for (long long p = 0; p < domain; ++p)
      ensure(tree->get(p).x == oracle[p], "dynamic mixed final-state oracle");
  }
  checksum = mix(checksum ^ mixed_checksum);
  out.values["mixed_checksum"] = mixed_checksum;
  out.values["mixed_add_count"] = (q + 4) / 5;
  out.values["mixed_get_count"] = q - (q + 4) / 5;
  out.values["baseline_mixed_get_lookup_calls"] = mixed_old_lookups;
  out.values["candidate_mixed_get_lookup_calls"] = mixed_new_lookups;
  out.values["input_hash"] = input; out.values["checksum"] = checksum;
  out.values["baseline_get_lookup_calls"] = old_lookups;
  out.values["candidate_get_lookup_calls"] = new_lookups;
  out.values["domain"] = domain;
  return out;
}

struct Affine { U64 a, b; bool operator==(const Affine&) const = default; };
struct Sum {
  U64 operator()(U64 a, U64 b) const { count(&Counts::combines); return a + b; }
};
struct Compose {
  Affine operator()(Affine a, Affine b) const {
    count(&Counts::combines); return {b.a * a.a, b.a * a.b + b.b};
  }
};
static U64 value_hash(U64 x) { return x; }
static U64 value_hash(Affine x) { return mix(x.a) ^ x.b; }
template<class T> static T generated(U64 x);
template<> U64 generated<U64>(U64 x) { return x; }
template<> Affine generated<Affine>(U64 x) { return {x % 11 + 1, x}; }

template<class T, class Op>
static SurveyResult persistent_profile(const std::string& shape, int n, int q, T identity, Op op, bool check) {
  const int updates = q / 4, mixed_count = q / 4;
  RNG rng{0x50455253495354ULL};
  U64 input = 0, checksum = 0;
  std::vector<T> initial(n, identity);
  if (shape != "sparse-branch") for (T& v : initial) v = generated<T>(rng.next() % 1000);
  for (const T& v : initial) input = mix(input ^ value_hash(v));
  struct Update { int version, p; T value; bool apply; };
  std::vector<Update> edits, mixed;
  std::vector<std::pair<int, int>> gets(q);
  struct Range { int version, l, r; };
  std::vector<Range> ranges(q / 8);
  for (int i = 0; i < updates; ++i) {
    Update u{shape == "dense-sequential" ? i : static_cast<int>(rng.next() % (i + 1)),
             static_cast<int>(rng.next() % n), generated<T>(rng.next() % 1000), i % 2 != 0};
    edits.push_back(u); input = mix(input ^ u.version); input = mix(input ^ u.p); input = mix(input ^ value_hash(u.value));
  }
  for (auto& [v, p] : gets) {
    v = rng.next() % (updates + 1); p = rng.next() % n;
    input = mix(input ^ v); input = mix(input ^ p);
  }
  for (Range& r : ranges) {
    r = {static_cast<int>(rng.next() % (updates + 1)), static_cast<int>(rng.next() % (n + 1)), static_cast<int>(rng.next() % (n + 1))};
    if (r.l > r.r) std::swap(r.l, r.r);
    input = mix(input ^ r.version); input = mix(input ^ r.l); input = mix(input ^ r.r);
  }
  int version_count = updates + 1;
  for (int i = 0; i < mixed_count; ++i) {
    Update u{static_cast<int>(rng.next() % version_count), static_cast<int>(rng.next() % n), generated<T>(rng.next() % 1000), i % 2 != 0};
    if (i % 5 == 0) ++version_count;
    mixed.push_back(u); input = mix(input ^ u.version); input = mix(input ^ u.p); input = mix(input ^ value_hash(u.value));
  }
  using Tree = blueberry::PersistentSegmentTree<T, Op>;
  std::unique_ptr<Tree> tree;
  SurveyResult out;
  stage(out, "build", [&] {
    if (shape == "sparse-branch") tree = std::make_unique<Tree>(n, op, identity);
    else tree = std::make_unique<Tree>(initial, op, identity);
  });
  stage(out, "update", [&] {
    for (const Update& u : edits) {
      if (u.apply) tree->apply(u.version, u.p, u.value);
      else tree->set(u.version, u.p, u.value);
    }
  });
  stage(out, "get", [&] {
    for (const auto& [v, p] : gets) checksum = mix(checksum ^ value_hash(tree->get(v, p)));
  });
  stage(out, "range", [&] {
    for (const Range& r : ranges) checksum = mix(checksum ^ value_hash(tree->prod(r.version, r.l, r.r)));
  });
  stage(out, "mixed", [&] {
    for (int i = 0; i < mixed_count; ++i) {
      const Update& u = mixed[i];
      if (i % 5 == 0) tree->set(u.version, u.p, u.value);
      else checksum = mix(checksum ^ value_hash(tree->get(u.version, u.p)));
    }
  });
  if (check) {
    std::vector<std::vector<T>> oracle{initial};
    for (const Update& u : edits) {
      auto next = oracle[u.version];
      next[u.p] = u.apply ? op(next[u.p], u.value) : u.value;
      oracle.push_back(std::move(next));
    }
    for (int i = 0; i < mixed_count; ++i) if (i % 5 == 0) {
      const auto& u = mixed[i]; auto next = oracle[u.version]; next[u.p] = u.value;
      oracle.push_back(std::move(next));
    }
    for (int v = 0; v < tree->versions(); ++v) {
      T total = identity;
      for (int p = 0; p < n; ++p) {
        ensure(tree->get(v, p) == oracle[v][p], "persistent point oracle");
        total = op(total, oracle[v][p]);
      }
      ensure(tree->all_prod(v) == total, "persistent all_prod oracle");
    }
    for (const Range& r : ranges) {
      T expected = identity;
      for (int p = r.l; p < r.r; ++p) expected = op(expected, oracle[r.version][p]);
      ensure(tree->prod(r.version, r.l, r.r) == expected, "persistent range oracle");
    }
  }
  out.values["input_hash"] = input; out.values["checksum"] = checksum;
  out.values["versions"] = tree->versions();
  return out;
}
int main(int argc, char** argv) {
  if (argc != 8) throw std::invalid_argument("family shape scalar implementation n q check");
  const std::string family = argv[1], shape = argv[2], scalar = argv[3], implementation = argv[4];
  const int n = std::stoi(argv[5]), q = std::stoi(argv[6]);
  const bool check = std::stoi(argv[7]);
  ensure(n > 0 && q >= 8, "profile sizes");
  SurveyResult out;
  if (family == "dynamic-fenwick") {
    if (scalar == "u64") out = dynamic_profile<U64>(shape, n, q, check);
    else out = dynamic_profile<atcoder::modint998244353>(shape, n, q, check);
  } else if (family == "persistent-segment") {
    if (scalar == "u64") out = persistent_profile(shape, n, q, U64{0}, Sum{}, check);
    else out = persistent_profile(shape, n, q, Affine{1, 0}, Compose{}, check);
  } else out = run_dense(family, shape, implementation, n, q, check);
  rusage usage{}; ensure(getrusage(RUSAGE_SELF, &usage) == 0, "getrusage");
  out.values["rss_kib"] = usage.ru_maxrss;
  std::cout << '{';
  bool first = true;
  for (const auto& [key, value] : out.values) {
    std::cout << (first ? "" : ",") << '"' << key << "\":" << value; first = false;
  }
  std::cout << "}\n";
}

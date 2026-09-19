// Reproducible microbenchmark: see docs/development/range-product-selection-comparison.md.
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <atcoder/segtree>
#include "blueberry/data-structure/disjoint-sparse-table.hpp"
#include "blueberry/data-structure/sqrt-tree.hpp"
#include "blueberry/data-structure/dynamic-sqrt-tree.hpp"

long long sum_op(long long a, long long b) { return a + b; }
long long sum_identity() { return 0; }
struct Sum { long long operator()(long long a, long long b) const { return a + b; } };
struct DST : blueberry::DisjointSparseTable<long long, Sum> {
  explicit DST(const std::vector<long long>& a) : DisjointSparseTable(a, Sum{}) {}
};
struct Query { int l, r; long long value; bool update; };

// Defined unsigned arithmetic; this checks that each process received equal inputs.
void mix(std::uint64_t& hash, std::uint64_t value) {
  for (int i = 0; i < 8; ++i) {
    hash ^= value & 255;
    hash *= 1099511628211ULL;
    value >>= 8;
  }
}

template <class Tree>
void measure(const std::string& name, const std::vector<long long>& values,
             const std::vector<Query>& queries, int update_percent,
             std::uint64_t seed, std::uint64_t input_hash) {
  using Clock = std::chrono::steady_clock;
  const auto start = Clock::now();
  Tree tree(values);
  const auto built = Clock::now();
  std::uint64_t checksum = 14695981039346656037ULL;
  for (const auto& query : queries) {
    if (query.update) {
      if constexpr (requires(Tree& t) { t.set(0, 0LL); }) tree.set(query.l, query.value);
      else std::abort();
    } else mix(checksum, static_cast<std::uint64_t>(tree.prod(query.l, query.r)));
  }
  // Include final state to catch mismatches even after the last query.
  mix(checksum, static_cast<std::uint64_t>(tree.prod(0, static_cast<int>(values.size()))));
  const auto finished = Clock::now();
  long peak_rss_kib = -1;
#ifdef __linux__
  // ru_maxrss may inherit the launcher memory high-water mark across exec.
  // VmHWM belongs to this executable's current address space.
  std::ifstream status("/proc/self/status");
  std::string label, rest;
  while (status >> label) {
    if (label == "VmHWM:") { status >> peak_rss_kib; break; }
    std::getline(status, rest);
  }
#endif
  std::cout << std::setprecision(10)
            << "{\"implementation\":\"" << name << "\",\"n\":" << values.size()
            << ",\"queries\":" << queries.size() << ",\"update_percent\":" << update_percent
            << ",\"seed\":" << seed << ",\"input_hash\":" << input_hash
            << ",\"checksum\":" << checksum
            << ",\"build_ms\":" << std::chrono::duration<double, std::milli>(built - start).count()
            << ",\"operations_ms\":" << std::chrono::duration<double, std::milli>(finished - built).count()
            << ",\"peak_process_rss_kib\":" << peak_rss_kib << "}\n";
}

int main(int argc, char** argv) {
  if (argc != 6) {
    std::cerr << "usage: benchmark <dst|static|dynamic|acl> N Q update_percent seed\n";
    return 2;
  }
  const std::string implementation = argv[1];
  const int n = std::stoi(argv[2]), q = std::stoi(argv[3]), rate = std::stoi(argv[4]);
  const auto seed = std::strtoull(argv[5], nullptr, 10);
  if (n <= 0 || q < 0 || rate < 0 || rate > 100 || ((implementation == "static" || implementation == "dst") && rate != 0)) return 2;
  std::mt19937_64 rng(seed);
  std::uint64_t input_hash = 14695981039346656037ULL;
  std::vector<long long> values(n);
  for (auto& value : values) { value = rng() % 1001; mix(input_hash, value); }
  std::vector<Query> queries;
  queries.reserve(q);
  for (int i = 0; i < q; ++i) {
    const bool update = i % 100 < rate;
    int l = rng() % n, r = rng() % n;
    const long long value = rng() % 1001;
    if (!update) { if (l > r) std::swap(l, r); ++r; }
    queries.push_back({l, r, value, update});
    mix(input_hash, l); mix(input_hash, r); mix(input_hash, value); mix(input_hash, update);
  }
  if (implementation == "dst") measure<DST>(implementation, values, queries, rate, seed, input_hash);
  else if (implementation == "static") measure<blueberry::SqrtTree<long long, sum_op, sum_identity>>(implementation, values, queries, rate, seed, input_hash);
  else if (implementation == "dynamic") measure<blueberry::DynamicSqrtTree<long long, sum_op, sum_identity>>(implementation, values, queries, rate, seed, input_hash);
  else if (implementation == "acl") measure<atcoder::segtree<long long, sum_op, sum_identity>>(implementation, values, queries, rate, seed, input_hash);
  else return 2;
}

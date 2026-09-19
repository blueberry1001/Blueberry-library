#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "blueberry/data-structure/fast-set.hpp"
#include "blueberry/data-structure/wavelet-matrix.hpp"
#include "blueberry/data-structure/static-range-lis.hpp"

using Clock = std::chrono::steady_clock;
std::uint64_t input_hash = 0, checksum = 0;
void mix(std::uint64_t& hash, std::uint64_t x) { hash = (hash ^ x) * 0x9e3779b185ebca87ULL; }
double ms(Clock::time_point a, Clock::time_point b) {
  return std::chrono::duration<double, std::milli>(b - a).count();
}
void report(Clock::time_point start, Clock::time_point built, Clock::time_point end) {
  std::ifstream status("/proc/self/status");
  std::string word; long rss = -1;
  while (status >> word) if (word == "VmHWM:") { status >> rss; break; }
  std::cout << "{\"build_ms\":" << ms(start, built) << ",\"operations_ms\":" << ms(built, end)
            << ",\"input_hash\":\"" << input_hash << "\",\"checksum\":\"" << checksum
            << "\",\"peak_process_rss_kib\":" << rss << "}\n";
}
struct Query { int l, r, k; };
int main(int argc, char** argv) {
  if (argc != 5) return 2;
  const std::string kind = argv[1], distribution = argv[2];
  const int n = std::atoi(argv[3]), q = std::atoi(argv[4]);
  std::mt19937_64 rng(20260920);
  if (kind == "set") {
    std::string initial(n, '0');
    for (int i = 0; i < n; ++i) {
      initial[i] = distribution == "dense" || (distribution == "mixed" && rng() % 2) ? '1' : '0';
      mix(input_hash, initial[i]);
    }
    std::vector<Query> operations(q);
    for (int i = 0; i < q; ++i) {
      // Mixed updates exercise both empty/nonempty transitions as density evolves.
      int x = static_cast<int>(rng() % n);
      int op = distribution == "duplicate" ? 0 : static_cast<int>(rng() % 5);
      if (distribution == "fill") { x = i % n; op = 0; }
      operations[i] = {x, 0, op}; mix(input_hash, x); mix(input_hash, op);
    }
    const auto start = Clock::now(); blueberry::FastSet set(initial); const auto built = Clock::now();
    for (const auto& op : operations) {
      if (op.k == 0) mix(checksum, set.insert(op.l));
      if (op.k == 1) mix(checksum, set.erase(op.l));
      if (op.k == 2) mix(checksum, set.next(op.l));
      if (op.k == 3) mix(checksum, set.prev(op.l));
      if (op.k == 4) mix(checksum, set.contains(op.l));
    }
    mix(checksum, set.size()); const auto end = Clock::now(); report(start, built, end); return 0;
  }
  std::vector<long long> values(n);
  for (int i = 0; i < n; ++i) {
    auto x = static_cast<long long>(rng() & 0x7fffffffffffffffULL);
    if (i % 2) x = -x;
    if (distribution == "duplicates") x = static_cast<int>(rng() % 17) - 8;
    if (distribution == "equal") x = -1;
    if (distribution == "sorted") x = i - n / 2;
    if (distribution == "reverse") x = n / 2 - i;
    values[i] = x; mix(input_hash, static_cast<std::uint64_t>(x));
  }
  std::vector<Query> queries(q);
  for (auto& query : queries) {
    query.l = static_cast<int>(rng() % n); query.r = static_cast<int>(rng() % n);
    if (query.l > query.r) std::swap(query.l, query.r);
    ++query.r; query.k = static_cast<int>(rng() % (query.r - query.l));
    mix(input_hash, query.l); mix(input_hash, query.r); mix(input_hash, query.k);
  }
  if (kind == "wavelet") {
    const auto start = Clock::now(); blueberry::WaveletMatrix<long long> matrix(values);
    const auto built = Clock::now();
    for (int i = 0; i < q; ++i) {
      const auto [l, r, k] = queries[i];
      // Include rank, exact count and both order statistics, not only kth.
      if (i % 4 == 0) mix(checksum, static_cast<std::uint64_t>(matrix.kth_smallest(l, r, k)));
      if (i % 4 == 1) mix(checksum, matrix.range_freq(l, r, values[k]));
      if (i % 4 == 2) mix(checksum, matrix.count(l, r, values[k]));
      if (i % 4 == 3) mix(checksum, static_cast<std::uint64_t>(matrix.kth_largest(l, r, k)));
    }
    const auto end = Clock::now(); report(start, built, end);
  } else if (kind == "lis") {
    const auto start = Clock::now(); blueberry::StaticRangeLIS<long long> lis(values);
    const auto built = Clock::now();
    for (const auto& query : queries) mix(checksum, lis.lis(query.l, query.r));
    const auto end = Clock::now(); report(start, built, end);
  } else return 2;
}

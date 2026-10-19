// Official verify flow with wall-clock phases; JSON goes only to stderr.
#include <chrono>
#include <cstdio>
#include <utility>
#include <vector>

#include <atcoder/modint>
#include "blueberry/math/sparse-exp.hpp"
#include "blueberry/utility/fast-io.hpp"

int main() {
  using Clock = std::chrono::steady_clock;
  const auto start = Clock::now();
  blueberry::FastInput in;
  blueberry::FastOutput out;
  int n, k;
  if (!in.read(n, k)) return 0;
  using Mint = atcoder::modint998244353;
  std::vector<std::pair<int, Mint>> terms;
  terms.reserve(k);
  for (int i = 0; i < k; ++i) {
    int degree, coefficient;
    in.read(degree, coefficient);
    terms.emplace_back(degree, coefficient);
  }
  const auto parsed = Clock::now();
  auto result = blueberry::sparse_exp(terms, n);
  const auto computed = Clock::now();
  for (int i = 0; i < n; ++i) {
    if (i) out.write(' ');
    out.write(result[i].val());
  }
  out.write('\n');
  const bool flushed = out.flush();
  const auto formatted = Clock::now();
  auto elapsed = [](Clock::time_point first, Clock::time_point last) {
    return std::chrono::duration<double, std::milli>(last - first).count();
  };
  std::fprintf(stderr,
               "{\"n\":%d,\"k\":%d,\"parse_ms\":%.9f,\"algorithm_ms\":%.9f,"
               "\"format_ms\":%.9f,\"total_ms\":%.9f,\"flush_ok\":%s}\n",
               n, k, elapsed(start, parsed), elapsed(parsed, computed),
               elapsed(computed, formatted), elapsed(start, formatted),
               flushed ? "true" : "false");
  return flushed ? 0 : 1;
}

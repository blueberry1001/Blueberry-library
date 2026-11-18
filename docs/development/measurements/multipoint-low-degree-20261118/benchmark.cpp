#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>
#if defined(__unix__) || defined(__APPLE__)
#include <sys/resource.h>
#endif
#include <atcoder/modint>
#include "blueberry/math/multipoint-evaluation.hpp"

using mint = atcoder::modint998244353;
int main(int argc, char** argv) {
  if (argc != 5) return 2;
  const int n = std::atoi(argv[1]), m = std::atoi(argv[2]), mode = std::atoi(argv[4]);
  const unsigned seed = std::strtoul(argv[3], nullptr, 10);
  if (n < 0 || m < 0 || mode < 0 || mode > 2) return 2;
  std::mt19937 rng(seed);
  std::vector<mint> f(n), x(m);
  for (auto& v : f) v = rng();
  for (auto& v : x) v = mode == 1 ? rng() % 5 : rng();
  if (mode == 2) for (int i = n / 2; i < n; ++i) f[i] = 0;
  const auto start = std::chrono::steady_clock::now();
  auto result = blueberry::multipoint_evaluation(f, x);
  const auto end = std::chrono::steady_clock::now();
  if (result.size() != x.size()) return 3;
  unsigned long long checksum = 1469598103934665603ULL;
  for (int i = 0; i < m; ++i) {
    mint expected = 0;
    for (auto it = f.rbegin(); it != f.rend(); ++it) expected = expected * x[i] + *it;
    if (result[i] != expected) { std::cerr << "mismatch at " << i << '\n'; return 4; }
    checksum = (checksum ^ result[i].val()) * 1099511628211ULL;
  }
  long long peak_rss = -1;
#if defined(__unix__) || defined(__APPLE__)
  struct rusage usage;
  if (!getrusage(RUSAGE_SELF, &usage)) peak_rss = usage.ru_maxrss;
#endif
  std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << ' ' << checksum << ' ' << peak_rss << '\n';
}

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <set>
#include <vector>
#include "blueberry/math/discrete-log.hpp"
#include "blueberry/math/divisor-convolution.hpp"
#include "blueberry/math/enumerate-quotients.hpp"
#include "blueberry/math/kth-root-integer.hpp"

int main(int argc, char** argv) {
  const unsigned seed = argc > 1 ? static_cast<unsigned>(std::strtoul(argv[1], nullptr, 10)) : 20260918;
  std::cerr << "math-batch-two seed=" << seed << '\n';
  std::mt19937_64 rng(seed);
  for (int m = 1; m <= 100; ++m) for (int a = 0; a < m; ++a) {
    std::vector<int> expected(m, -1);
    int x = 1 % m;
    for (int e = 0; expected[x] == -1; ++e, x = x * a % m) expected[x] = e;
    for (int b = 0; b < m; ++b) {
      auto actual = blueberry::discrete_log(a, b, m).value_or(-1);
      if (actual != expected[b]) {
        std::cerr << "a=" << a << " b=" << b << " m=" << m
                  << " expected=" << expected[b] << " actual=" << actual << '\n';
        return 1;
      }
      assert(blueberry::discrete_log(a - m, b - m, m).value_or(-1) == expected[b]);
    }
  }
  for (int n = 0; n <= 1000; ++n) {
    std::set<std::int64_t> expected;
    for (int i = 1; i <= n; ++i) expected.insert(n / i);
    auto actual = blueberry::enumerate_quotients(n);
    assert(actual == std::vector<std::int64_t>(expected.begin(), expected.end()));
  }
  auto fits = [](std::uint64_t x, int k, std::uint64_t n) {
    __uint128_t p = 1;
    for (int i = 0; i < k; ++i) { p *= x; if (p > n) return false; }
    return true;
  };
  for (int t = 0; t < 10000; ++t) {
    std::uint64_t n = t < 100 ? static_cast<std::uint64_t>(t) : rng();
    if (t == 100) n = std::numeric_limits<std::uint64_t>::max();
    for (int k = 1; k <= 65; ++k) {
      auto r = blueberry::kth_root_integer(n, k);
      assert(fits(r, k, n));
      assert(r == std::numeric_limits<std::uint64_t>::max() || !fits(r + 1, k, n));
    }
  }
  for (int n = 0; n <= 80; ++n) for (int t = 0; t < 20; ++t) {
    std::vector<long long> a(n), b(n), g(n), l(n), d(n), m(n);
    for (int i = 1; i < n; ++i) { a[i] = static_cast<int>(rng() % 11) - 5; b[i] = rng() % 7; }
    for (int i = 1; i < n; ++i) for (int j = 1; j < n; ++j) {
      g[std::gcd(i, j)] += a[i] * b[j];
      if (std::lcm(i, j) < n) l[std::lcm(i, j)] += a[i] * b[j];
      if (j % i == 0) { d[j] += a[i]; m[i] += a[j]; }
    }
    assert(blueberry::gcd_convolution(a, b) == g);
    assert(blueberry::lcm_convolution(a, b) == l);
    auto x = a;
    blueberry::divisor_zeta(x); assert(x == d);
    blueberry::divisor_mobius(x); assert(x == a);
    blueberry::multiple_zeta(x); assert(x == m);
    blueberry::multiple_mobius(x); assert(x == a);
  }
}

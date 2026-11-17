#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <vector>
#include "blueberry/math/discrete-log.hpp"

int main() {
  if (blueberry::discrete_log(5, 1073741823, 1073741824)) return 7;
  if (blueberry::discrete_log(-1, 1073741823, 1073741824).value_or(-1) != 1) return 8;
  for (int m = 1; m <= 150; ++m) for (int a = 0; a < m; ++a) {
    std::vector<int> expected(m, -1);
    int p = 1 % m;
    for (int x = 0; expected[p] < 0; ++x, p = int(std::int64_t(p) * a % m)) expected[p] = x;
    for (int b = 0; b < m; ++b) for (int neg = 0; neg < 2; ++neg) {
      auto actual = blueberry::discrete_log(a - neg * m, b - neg * m, m).value_or(-1);
      if (actual != expected[b]) {
        std::cerr << "a=" << a << " b=" << b << " m=" << m << " expected=" << expected[b] << " actual=" << actual << '\n';
        return 1;
      }
    }
  }
  std::mt19937_64 rng(891724);
  for (int k = 10; k <= 30; ++k) for (int i = 0; i < 30; ++i) {
    const std::int64_t m = std::int64_t(1) << k, exponent = rng() % (m / 4);
    std::int64_t p = 5, b = 1, e = exponent;
    for (; e; e >>= 1, p = p * p % m) if (e & 1) b = b * p % m;
    if (blueberry::discrete_log(5, b, m).value_or(-1) != exponent) return 9;
    if (blueberry::discrete_log(5, m - b, m)) return 10;
  }
  for (int i = 0; i < 3000; ++i) {
    const std::int64_t m = 2 + rng() % 2147483646, a = rng() % m;
    const int exponent = rng() % 100;
    std::int64_t b = 1 % m;
    for (int j = 0; j < exponent; ++j) b = b * a % m;
    auto actual = blueberry::discrete_log(a, b, m);
    if (!actual || *actual > exponent) return 2;
    std::int64_t p = 1 % m;
    for (int j = 0; j < *actual; ++j) { if (p == b) return 3; p = p * a % m; }
    if (p != b) return 4;
  }
  const auto low = std::numeric_limits<std::int64_t>::min();
  const auto high = std::numeric_limits<std::int64_t>::max();
  for (auto m : {1, 2, 3, 97, 1000000007, 2147483647}) {
    for (auto a : {low, high, std::int64_t(-1), std::int64_t(0), std::int64_t(1)}) {
      if (blueberry::discrete_log(a, 1, m).value_or(-1) != 0) return 5;
      if (m > 1 && a % m == 0 && blueberry::discrete_log(a, 0, m).value_or(-1) != 1) return 6;
    }
  }
}

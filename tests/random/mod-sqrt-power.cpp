#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <vector>
#include "blueberry/math/mod-sqrt.hpp"
using i64 = std::int64_t;
bool prime(int p) {
  for (int d = 2; d * d <= p; ++d) if (p % d == 0) return false;
  return p >= 2;
}
i64 power(i64 a, i64 e, i64 p) {
  i64 r = 1;
  for (; e; e >>= 1, a = a * a % p) if (e & 1) r = r * a % p;
  return r;
}
int main() {
  for (int p = 2; p <= 500; ++p) if (prime(p)) {
    std::vector<int> roots(p, -1);
    for (int x = 0; x < p; ++x) if (roots[x * x % p] < 0) roots[x * x % p] = x;
    for (i64 a = -2 * p; a <= 2 * p; ++a) {
      i64 expected = roots[(a % p + p) % p];
      i64 actual = blueberry::mod_sqrt(a, p).value_or(-1);
      if (actual != expected) {
        std::cerr << "p=" << p << " a=" << a << " expected=" << expected << " actual=" << actual << '\n';
        return 1;
      }
    }
  }
  std::mt19937_64 rng(20261117);
  for (i64 p : {65537LL, 998244353LL, 1000000007LL, 1000000009LL, 2013265921LL, 2147483647LL}) {
    for (int i = 0; i < 10000; ++i) {
      i64 a = i == 0 ? std::numeric_limits<i64>::min() : i == 1 ? std::numeric_limits<i64>::max() : static_cast<i64>(rng());
      i64 n = (a % p + p) % p;
      auto r = blueberry::mod_sqrt(a, p);
      bool exists = n == 0 || power(n, (p - 1) / 2, p) == 1;
      if (r.has_value() != exists || (r && (*r * *r % p != n || *r > p - *r))) {
        std::cerr << "seed=20261117 p=" << p << " a=" << a << " actual=" << r.value_or(-1) << '\n';
        return 1;
      }
    }
  }
}

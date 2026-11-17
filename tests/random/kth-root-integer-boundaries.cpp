#include <bit>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <limits>
#include <random>
#include "blueberry/math/kth-root-integer.hpp"
using U = std::uint64_t;

U oracle(U n, int k) {
  if (k == 1 || n <= 1) return n;
  if (k >= 64) return 1;
  U lo = 1, hi = U{1} << ((std::bit_width(n) + k - 1) / k);
  while (hi - lo > 1) {
    const U mid = lo + (hi - lo) / 2;
    U value = 1;
    bool fits = true;
    for (int i = 0; i < k; ++i) {
      if (value > n / mid) { fits = false; break; }
      value *= mid;
    }
    if (fits) lo = mid; else hi = mid;
  }
  return lo;
}

int main(int argc, char** argv) {
  const U seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 20261117;
  std::mt19937_64 rng(seed);
  auto check = [&](U n, int k) {
    const U expected = oracle(n, k), actual = blueberry::kth_root_integer(n, k);
    if (actual != expected) {
      std::fprintf(stderr, "seed=%llu n=%llu k=%d expected=%llu actual=%llu\n",
                   static_cast<unsigned long long>(seed), static_cast<unsigned long long>(n), k,
                   static_cast<unsigned long long>(expected), static_cast<unsigned long long>(actual));
      std::exit(1);
    }
  };
  for (U n = 0; n < 8192; ++n) for (int k = 1; k <= 70; ++k) check(n, k);
  for (U base = 2; base <= 4096; ++base) {
    U value = 1;
    for (int k = 1; k <= 63; ++k) {
      if (value > std::numeric_limits<U>::max() / base) break;
      value *= base;
      if (k >= 2) {
        check(value - 1, k); check(value, k);
        if (value != std::numeric_limits<U>::max()) check(value + 1, k);
      }
    }
  }
  for (U base : {U{65535}, U{65536}, U{1048575}, U{1048576}, U{2097151}, U{2097152}, U{2147483647}, U{2147483648}, U{4294967294}, U{4294967295}}) {
    U value = 1;
    for (int k = 1; k <= 63; ++k) {
      if (value > std::numeric_limits<U>::max() / base) break;
      value *= base;
      if (k >= 2) {
        check(value - 1, k); check(value, k);
        if (value != std::numeric_limits<U>::max()) check(value + 1, k);
      }
    }
  }
  for (int i = 0; i < 20000; ++i) check(rng(), 1 + static_cast<int>(rng() % 70));
  for (int k = 1; k <= 100; ++k) {
    check(std::numeric_limits<U>::max(), k);
    check(std::numeric_limits<U>::max() - 1, k);
  }
  check(std::numeric_limits<U>::max(), std::numeric_limits<int>::max());
}

#include "blueberry/math/primitive-root.hpp"
#include "blueberry/math/primitive-root.hpp"
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

using u64 = std::uint64_t;
u64 seed;

void check(bool ok, u64 p, const char* expected, u64 actual) {
  if (!ok) {
    std::cerr << "seed=" << seed << " input=" << p << " expected=" << expected
              << " actual=" << actual << '\n';
    std::exit(1);
  }
}

// Independent left-to-right modular exponentiation.
u64 power(u64 a, u64 b, u64 p) {
  u64 result = 1;
  for (int bit = 63; bit >= 0; --bit) {
    result = static_cast<unsigned __int128>(result) * result % p;
    if ((b >> bit) & 1) result = static_cast<unsigned __int128>(result) * a % p;
  }
  return result;
}

void verify(u64 p, bool brute) {
  const u64 g = blueberry::primitive_root(p);
  check(1 <= g && g < p, p, "1 <= root < p", g);
  check(g == blueberry::primitive_root(p), p, "deterministic result", g);
  check(power(g, p - 1, p) == 1, p, "root^(p-1) == 1", g);
  if (brute) {
    std::vector<bool> seen(p);
    u64 current = 1;
    for (u64 i = 0; i < p - 1; ++i) {
      check(!seen[current], p, "all nonzero residues exactly once", g);
      seen[current] = true;
      current = static_cast<unsigned __int128>(current) * g % p;
    }
    check(current == 1, p, "order p-1", g);
  } else {
    for (u64 q : blueberry::factorize(p - 1))
      check(power(g, (p - 1) / q, p) != 1, p, "root^((p-1)/q) != 1", g);
  }
}

int main(int argc, char** argv) {
  seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  std::mt19937_64 rng(seed);
  // Exhaustive primality and group enumeration do not depend on factorize().
  for (u64 p = 2; p <= 2000; ++p) {
    bool prime = true;
    for (u64 d = 2; d * d <= p; ++d) if (p % d == 0) prime = false;
    if (prime) verify(p, true);
  }
  for (u64 p : {998244353ULL, 1000000007ULL, 1000000009ULL,
                2305843009213693951ULL, 18446744069414584321ULL,
                18446744073709551557ULL}) verify(p, false);
  for (int test = 0; test < 50; ++test) {
    u64 p = rng() % 1000000000000ULL + 2;
    while (!blueberry::is_prime(p)) ++p;
    verify(p, false);
  }
}

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>

#include <boost/multiprecision/cpp_int.hpp>
#include "blueberry/math/tetration-mod.hpp"

using u64 = std::uint64_t;
using u32 = std::uint32_t;
using boost::multiprecision::cpp_int;

// Independent reference: full Euler chain with an exact capped tower test.
u64 bounded(u64 a, u64 h, u64 cap) {
  if (!cap) return 0;
  if (!h) return 1;
  if (!a) return (h % 2 == 0);
  if (a == 1 || cap == 1) return 1;
  u64 limit = 0, value = 1;
  while (value < cap) {
    ++limit;
    if (value > (cap - 1) / a) { value = cap; break; }
    value *= a;
  }
  const auto exponent = bounded(a, h - 1, limit);
  if (exponent == limit) return cap;
  value = 1;
  for (u64 i = 0; i < exponent; ++i) value *= a;
  return value;
}

u32 reference(u64 a, u64 h, u32 m) {
  if (m == 1) return 0;
  if (!h || a == 1) return 1;
  if (!a) return static_cast<u32>(h % 2 == 0);
  if (h == 1) return static_cast<u32>(a % m);
  u32 phi = m, n = m;
  for (u64 p = 2; p * p <= n; ++p) {
    if (n % p) continue;
    phi -= phi / static_cast<u32>(p);
    while (n % p == 0) n /= static_cast<u32>(p);
  }
  if (n > 1) phi -= phi / n;
  u64 e = reference(a, h - 1, phi);
  if (bounded(a, h - 1, phi) >= phi) e += phi;
  // cpp_int modular power independently checks uint32 maximum products.
  return boost::multiprecision::powm(cpp_int(a), cpp_int(e), cpp_int(m)).convert_to<u32>();
}

int main(int argc, char** argv) {
  const u64 seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 0x74e7a710ULL;
  std::mt19937_64 random(seed);
  auto check = [&](u64 a, u64 h, u32 m, u32 expected) {
    const auto actual = blueberry::tetration_mod(a, h, m);
    if (actual != expected) {
      std::cerr << "seed=" << seed << " input=" << a << ' ' << h << ' ' << m
                << " expected=" << expected << " actual=" << actual << '\n';
      std::abort();
    }
  };
  // Brute-force exact towers, including 0^0, and every small modulus.
  for (u64 a = 0; a <= 6; ++a) {
    cpp_int value = 1;
    for (u64 h = 0; h <= 4; ++h) {
      if (h && value > 1000) break;
      if (h) value = boost::multiprecision::pow(cpp_int(a), value.convert_to<unsigned>());
      for (u32 m = 1; m <= 150; ++m) check(a, h, m, (value % m).convert_to<u32>());
    }
  }
  for (u64 a : {0ULL, 1ULL, 2ULL, 3ULL, 6ULL, 32ULL, 65536ULL,
                4294967295ULL, 18446744073709551615ULL})
    for (u64 h : {0ULL, 1ULL, 2ULL, 3ULL, 4ULL, 5ULL, 32ULL,
                  18446744073709551615ULL})
      for (u32 m : {1U, 2U, 3U, 4U, 6U, 8U, 12U, 72U, 65536U,
                    2147483648U, 3221225472U, 4294967291U, 4294967295U})
        check(a, h, m, reference(a, h, m));
  for (int i = 0; i < 5000; ++i) {
    const u64 a = i % 2 ? random() : random() % 100;
    const u64 h = i % 3 ? random() : random() % 8;
    const u32 m = i % 4 ? static_cast<u32>(random() % 100000 + 1)
                       : static_cast<u32>(random() % 4294967295ULL + 1);
    check(a, h, m, reference(a, h, m));
  }
  std::cout << "tetration_mod seed=" << seed << " passed\n";
}

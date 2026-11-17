#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>

namespace blueberry {

// Tonelli--Shanks. prime_mod must be prime, 2 <= prime_mod <= INT32_MAX.
inline std::optional<std::int64_t> mod_sqrt(std::int64_t a, std::int64_t prime_mod) {
  assert(2 <= prime_mod && prime_mod <= std::numeric_limits<std::int32_t>::max());
  const std::int64_t p = prime_mod;
  a %= p;
  if (a < 0) a += p;
  if (a == 0 || p == 2) return a;
  auto power = [p](std::int64_t x, std::int64_t exponent) {
    std::int64_t result = 1;
    for (; exponent; exponent >>= 1, x = x * x % p)
      if (exponent & 1) result = result * x % p;
    return result;
  };
  if (power(a, (p - 1) / 2) != 1) return std::nullopt;
  if (p % 4 == 3) {
    const auto root = power(a, (p + 1) / 4);
    return std::min(root, p - root);
  }
  std::int64_t odd = p - 1;
  int twos = 0;
  while (!(odd & 1)) odd >>= 1, ++twos;
  std::int64_t nonresidue = 2;
  while (power(nonresidue, (p - 1) / 2) == 1) ++nonresidue;
  std::int64_t c = power(nonresidue, odd);
  std::int64_t root = power(a, (odd + 1) / 2);
  std::int64_t residue = power(a, odd);
  while (residue != 1) {
    int exponent = 0;
    for (std::int64_t x = residue; x != 1; x = x * x % p) ++exponent;
    std::int64_t correction = c;
    for (int i = 0; i < twos - exponent - 1; ++i) correction = correction * correction % p;
    root = root * correction % p;
    c = correction * correction % p;
    residue = residue * c % p;
    twos = exponent;
  }
  return std::min(root, p - root);
}

}  // namespace blueberry

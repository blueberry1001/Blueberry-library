#pragma once

#include <cassert>
#include <cstdint>
#include <numeric>

#include "blueberry/math/factorize.hpp"

namespace blueberry {
namespace tetration_mod_detail {

// The exponent needed to kill every non-coprime prime power is at most 31.
inline std::uint32_t capped_tower(std::uint64_t base, std::uint64_t height) {
  if (height == 0) return 1;
  if (base == 0) return static_cast<std::uint32_t>((height & 1) == 0);
  if (base == 1) return 1;
  if (base >= 32 || height >= 4) return 32;
  if (height == 1) return static_cast<std::uint32_t>(base);
  if (height == 3) return base == 2 ? 16 : 32;
  std::uint32_t value = 1;
  for (std::uint64_t i = 0; i < base; ++i) {
    value *= static_cast<std::uint32_t>(base);
    if (value >= 32) return 32;
  }
  return value;
}

inline std::uint32_t pow_mod(std::uint64_t base, std::uint64_t exponent,
                             std::uint32_t mod) {
  base %= mod;
  std::uint64_t result = 1;
  while (exponent) {
    if (exponent & 1) result = result * base % mod;
    exponent >>= 1;
    if (exponent) base = base * base % mod;
  }
  return static_cast<std::uint32_t>(result);
}

inline std::uint32_t totient(std::uint32_t n) {
  std::uint32_t result = n;
  std::uint64_t previous = 0;
  for (auto p : factorize(n)) {
    if (p != previous) result -= result / static_cast<std::uint32_t>(p);
    previous = p;
  }
  return result;
}

}  // namespace tetration_mod_detail

// Returns base^^height modulo mod, with base^^0 = 1 and 0^0 = 1.
inline std::uint32_t tetration_mod(std::uint64_t base, std::uint64_t height,
                                  std::uint32_t mod) {
  assert(mod >= 1);
  if (mod == 1) return 0;
  if (height == 0 || base == 1) return 1;
  if (base == 0) return static_cast<std::uint32_t>((height & 1) == 0);
  if (height == 1) return static_cast<std::uint32_t>(base % mod);
  if (height == 2) return tetration_mod_detail::pow_mod(base, base, mod);
  const auto small = tetration_mod_detail::capped_tower(base, height - 1);
  if (small < 32) return tetration_mod_detail::pow_mod(base, small, mod);

  // After `threshold` powers, the factors shared by base and mod vanish.
  // The remaining modulus is coprime to base, so only its totient is needed.
  std::uint32_t coprime_mod = mod, threshold = 0;
  for (auto g = std::gcd(coprime_mod, static_cast<std::uint32_t>(base % mod));
       g != 1; g = std::gcd(coprime_mod, g)) {
    coprime_mod /= g;
    ++threshold;
  }
  if (coprime_mod == 1) return 0;
  const auto phi = tetration_mod_detail::totient(coprime_mod);
  std::uint64_t exponent = tetration_mod(base, height - 1, phi);
  if (exponent < threshold)
    exponent += ((threshold - exponent + phi - 1) / phi) * phi;
  return tetration_mod_detail::pow_mod(base, exponent, mod);
}

}  // namespace blueberry

#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <numeric>
#include <vector>

namespace blueberry {
namespace factorize_detail {

using u64 = std::uint64_t;

inline u64 mul_mod(u64 a, u64 b, u64 mod) {
  return static_cast<u64>(static_cast<unsigned __int128>(a) * b % mod);
}

inline u64 pow_mod(u64 a, u64 exponent, u64 mod) {
  u64 result = 1;
  for (; exponent; exponent >>= 1, a = mul_mod(a, a, mod))
    if (exponent & 1) result = mul_mod(result, a, mod);
  return result;
}

// Local SplitMix64 state: deterministic restarts without changing global RNG state.
inline u64 next_random(u64& state) {
  u64 z = (state += 0x9e3779b97f4a7c15ULL);
  z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
  z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
  return z ^ (z >> 31);
}

inline u64 find_factor(u64 n) {
  u64 state = n;
  constexpr int budget = 1 << 20;
  constexpr u64 batch = 128;
  for (;;) {
    const u64 c = next_random(state) % (n - 1) + 1;
    u64 y = next_random(state) % (n - 1) + 1;
    auto advance = [&](u64 x) {
      // Reduce the product first: x*x+c can overflow even unsigned __int128.
      return static_cast<u64>((static_cast<unsigned __int128>(mul_mod(x, x, n)) + c) % n);
    };
    auto difference = [](u64 x, u64 z) { return x > z ? x - z : z - x; };
    u64 g = 1, x = 0, saved = 0;
    int used = 0;
    for (u64 length = 1; g == 1 && used < budget; length <<= 1) {
      x = y;
      for (u64 i = 0; i < length && used < budget; ++i, ++used) y = advance(y);
      for (u64 offset = 0; offset < length && g == 1 && used < budget; offset += batch) {
        saved = y;
        u64 product = 1;
        const u64 count = std::min(batch, length - offset);
        for (u64 i = 0; i < count && used < budget; ++i, ++used) {
          y = advance(y);
          product = mul_mod(product, difference(x, y), n);
        }
        g = std::gcd(product, n);
      }
    }
    if (g == n) {
      // Recover the first nontrivial gcd hidden by a zero product modulo n.
      do {
        saved = advance(saved);
        g = std::gcd(difference(x, saved), n);
        ++used;
      } while (g == 1 && used < budget);
    }
    if (g > 1 && g < n) return g;
    // A bounded unsuccessful walk starts again with fresh c and y.
  }
}

}  // namespace factorize_detail

// Deterministic Miller--Rabin for the complete uint64_t range.
inline bool is_prime(std::uint64_t n) {
  if (n < 2) return false;
  for (std::uint64_t p : {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37})
    if (n % p == 0) return n == p;
  std::uint64_t d = n - 1;
  int twos = 0;
  while (!(d & 1)) d >>= 1, ++twos;
  constexpr std::array<std::uint64_t, 7> bases{
      2, 325, 9375, 28178, 450775, 9780504, 1795265022};
  for (std::uint64_t base : bases) {
    if (base % n == 0) continue;
    std::uint64_t x = factorize_detail::pow_mod(base % n, d, n);
    if (x == 1 || x == n - 1) continue;
    bool passes = false;
    for (int r = 1; r < twos; ++r) {
      x = factorize_detail::mul_mod(x, x, n);
      if (x == n - 1) { passes = true; break; }
    }
    if (!passes) return false;
  }
  return true;
}

// n >= 1. Returns all prime factors, with multiplicity, in ascending order.
inline std::vector<std::uint64_t> factorize(std::uint64_t n) {
  assert(n >= 1);
  std::vector<std::uint64_t> result;
  for (std::uint64_t p : {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37})
    while (n % p == 0) result.push_back(p), n /= p;
  std::vector<std::uint64_t> pending;
  if (n > 1) pending.push_back(n);
  while (!pending.empty()) {
    n = pending.back();
    pending.pop_back();
    if (is_prime(n)) {
      result.push_back(n);
    } else {
      const auto divisor = factorize_detail::find_factor(n);
      pending.push_back(divisor);
      pending.push_back(n / divisor);
    }
  }
  std::sort(result.begin(), result.end());
  return result;
}

}  // namespace blueberry

#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>

#include "blueberry/math/factorize.hpp"

namespace blueberry {

// p must be prime. Returns a primitive root in [1, p), not necessarily the least.
inline std::uint64_t primitive_root(std::uint64_t p) {
  assert(p >= 2);
  if (p == 2) return 1;
  auto exponents = factorize(p - 1);
  exponents.erase(std::unique(exponents.begin(), exponents.end()), exponents.end());
  for (auto& q : exponents) q = (p - 1) / q;
  std::uint64_t state = p, candidate = 2;
  for (;;) {
    // Reuse candidate^(2^i) instead of repeating squarings for every exponent.
    std::array<std::uint64_t, 64> squares;
    squares[0] = candidate;
    for (int bit = 1; bit < 64 && (exponents[0] >> bit); ++bit)
      squares[bit] = factorize_detail::mul_mod(squares[bit - 1], squares[bit - 1], p);
    bool ok = true;
    for (auto exponent : exponents) {
      std::uint64_t value = 1;
      for (int bit = 0; exponent; exponent >>= 1, ++bit)
        if (exponent & 1) value = factorize_detail::mul_mod(value, squares[bit], p);
      if (value == 1) {
        ok = false;
        break;
      }
    }
    if (ok) return candidate;
    candidate = factorize_detail::next_random(state) % (p - 2) + 2;
  }
}

}  // namespace blueberry

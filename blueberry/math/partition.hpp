#pragma once

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>
#include "blueberry/math/formal-power-series.hpp"

namespace blueberry {

// p(0)..p(n), by inverting Euler's pentagonal-number product.
template <class Mint>
std::vector<Mint> partition(int n) {
  static_assert(atcoder::internal::is_static_modint<Mint>::value);
  static_assert(atcoder::internal::is_prime_constexpr(Mint::mod()));
  assert(0 <= n && n < Mint::mod() - 1);
  [[maybe_unused]] const auto length = std::bit_ceil(static_cast<std::size_t>(n) + 1);
  assert((Mint::mod() - 1) % length == 0);
  FormalPowerSeries<Mint> denominator(static_cast<std::size_t>(n) + 1);
  denominator[0] = 1;
  for (std::int64_t k = 1;; ++k) {
    const std::int64_t first = k * (3 * k - 1) / 2;
    if (first > n) break;
    const Mint sign = (k & 1) ? Mint(-1) : Mint(1);
    denominator[static_cast<std::size_t>(first)] = sign;
    const std::int64_t second = first + k;
    if (second <= n) denominator[static_cast<std::size_t>(second)] = sign;
  }
  return denominator.inv(n + 1);
}

}  // namespace blueberry

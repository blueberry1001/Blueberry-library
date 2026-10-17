#pragma once

#include <bit>
#include <cassert>
#include <cstddef>
#include <vector>
#include "blueberry/math/formal-power-series.hpp"

namespace blueberry {

// Bell(0)..Bell(n), from exp(exp(x)-1); the inner exp is inverse factorials.
template <class Mint>
std::vector<Mint> bell(int n) {
  static_assert(atcoder::internal::is_static_modint<Mint>::value);
  static_assert(atcoder::internal::is_prime_constexpr(Mint::mod()));
  assert(0 <= n && n < Mint::mod() - 1);
  [[maybe_unused]] const auto length = std::bit_ceil(static_cast<std::size_t>(n) + 1);
  assert((Mint::mod() - 1) % length == 0);
  FormalPowerSeries<Mint> exponent(static_cast<std::size_t>(n) + 1);
  Mint factorial = 1;
  for (int i = 1; i <= n; ++i) factorial *= Mint(i);
  exponent[n] = factorial.inv();
  for (int i = n; i > 1; --i) exponent[i - 1] = exponent[i] * Mint(i);
  exponent[0] = 0;
  auto result = exponent.exp(n + 1);
  factorial = 1;
  for (int i = 1; i <= n; ++i) result[i] *= (factorial *= Mint(i));
  return result;
}

}  // namespace blueberry

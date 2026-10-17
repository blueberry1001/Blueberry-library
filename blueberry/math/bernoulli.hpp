#pragma once

#include <bit>
#include <cassert>
#include <cstddef>
#include <vector>
#include "blueberry/math/formal-power-series.hpp"

namespace blueberry {

// B(0)..B(n), with B(1)=-1/2, from x/(exp(x)-1).
template <class Mint>
std::vector<Mint> bernoulli(int n) {
  static_assert(atcoder::internal::is_static_modint<Mint>::value);
  static_assert(atcoder::internal::is_prime_constexpr(Mint::mod()));
  assert(0 <= n && n < Mint::mod() - 1);
  [[maybe_unused]] const auto length = std::bit_ceil(static_cast<std::size_t>(n) + 1);
  assert((Mint::mod() - 1) % length == 0);
  FormalPowerSeries<Mint> denominator(static_cast<std::size_t>(n) + 1);
  Mint factorial = 1;
  for (int i = 1; i <= n + 1; ++i) factorial *= Mint(i);
  denominator[n] = factorial.inv();
  for (int i = n; i > 0; --i) denominator[i - 1] = denominator[i] * Mint(i + 1);
  auto result = denominator.inv(n + 1);
  factorial = 1;
  for (int i = 1; i <= n; ++i) result[i] *= (factorial *= Mint(i));
  return result;
}

}  // namespace blueberry

#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <vector>
#include <atcoder/convolution>

namespace blueberry {

// Coefficients of f(x + c), preserving the input length.
template <class Mint>
std::vector<Mint> taylor_shift(const std::vector<Mint>& f, Mint c) {
  static_assert(atcoder::internal::is_static_modint<Mint>::value);
  const std::size_t n = f.size();
  if (!n) return {};
  assert(n <= static_cast<std::size_t>(Mint::mod()));
  [[maybe_unused]] std::size_t length = 1;
  while (length < 2 * n - 1) length *= 2;
  assert((Mint::mod() - 1) % length == 0);
  std::vector<Mint> fact(n, 1), invfact(n, 1), a(n), b(n);
  for (std::size_t i = 1; i < n; ++i) fact[i] = fact[i - 1] * Mint(i);
  invfact[n - 1] = fact[n - 1].inv();
  for (std::size_t i = n - 1; i; --i) invfact[i - 1] = invfact[i] * Mint(i);
  Mint power = 1;
  for (std::size_t i = 0; i < n; ++i) {
    a[n - 1 - i] = f[i] * fact[i];
    b[i] = power * invfact[i];
    power *= c;
  }
  auto result = atcoder::convolution(a, b);
  result.resize(n);
  std::reverse(result.begin(), result.end());
  for (std::size_t i = 0; i < n; ++i) result[i] *= invfact[i];
  return result;
}

}  // namespace blueberry

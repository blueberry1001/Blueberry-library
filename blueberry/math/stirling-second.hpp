#pragma once

#include <cassert>
#include <cstddef>
#include <vector>
#include <atcoder/convolution>

namespace blueberry {

// S(n, k), k = 0..n, via the inclusion-exclusion convolution.
template <class Mint>
std::vector<Mint> stirling_second(int n) {
  static_assert(atcoder::internal::is_static_modint<Mint>::value);
  assert(0 <= n && n < Mint::mod());
  [[maybe_unused]] std::size_t length = 1;
  while (length < 2 * static_cast<std::size_t>(n) + 1) length *= 2;
  assert((Mint::mod() - 1) % length == 0);
  std::vector<Mint> inverse_factorial(static_cast<std::size_t>(n) + 1, 1);
  Mint factorial = 1;
  for (int i = 1; i <= n; ++i) factorial *= Mint(i);
  inverse_factorial[n] = factorial.inv();
  for (int i = n; i; --i) inverse_factorial[i - 1] = inverse_factorial[i] * Mint(i);
  std::vector<Mint> a(n + 1), b(n + 1);
  for (int i = 0; i <= n; ++i) {
    a[i] = Mint(i).pow(n) * inverse_factorial[i];
    b[i] = (i & 1) ? -inverse_factorial[i] : inverse_factorial[i];
  }
  auto result = atcoder::convolution(a, b);
  result.resize(static_cast<std::size_t>(n) + 1);
  return result;
}

}  // namespace blueberry

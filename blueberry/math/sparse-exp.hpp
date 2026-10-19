#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

#include "blueberry/math/formal-power-series.hpp"

namespace blueberry {

// exp(f) modulo x^n, from g' = f'g. Terms may be unordered or repeated.
// The sum of constant terms must be zero; n must be smaller than the prime.
template <class Mint>
FPS<Mint> sparse_exp(const std::vector<std::pair<int, Mint>>& terms, int n) {
  static_assert(atcoder::internal::is_static_modint<Mint>::value);
  static_assert(atcoder::internal::is_prime_constexpr(Mint::mod()));
  assert(0 <= n && n < Mint::mod());
  std::vector<std::pair<int, Mint>> sparse;
  sparse.reserve(terms.size());
  Mint constant = 0;
  for (const auto& [degree, coefficient] : terms) {
    assert(degree >= 0);
    if (degree == 0) constant += coefficient;
    else if (degree < n && coefficient != Mint(0)) sparse.emplace_back(degree, coefficient);
  }
  assert(constant == Mint(0));
  if (n == 0) return {};
  std::sort(sparse.begin(), sparse.end(), [](const auto& a, const auto& b) {
    return a.first < b.first;
  });
  std::size_t count = 0;
  for (const auto& term : sparse) {
    if (count && sparse[count - 1].first == term.first) sparse[count - 1].second += term.second;
    else sparse[count++] = term;
  }
  sparse.resize(count);
  count = 0;
  for (auto term : sparse) {
    if (term.second == Mint(0)) continue;
    sparse[count++] = term;
  }
  sparse.resize(count);

  FPS<Mint> result(n);
  result[0] = 1;
  if (sparse.empty()) return result;
  // If f^2 vanishes modulo x^n, exp(f) is just 1 + f.
  if (sparse.front().first >= n - sparse.front().first) {
    for (const auto& [degree, coefficient] : sparse) result[degree] = coefficient;
    return result;
  }
  for (auto& [degree, coefficient] : sparse) coefficient *= Mint(degree);
  std::vector<Mint> inverse(n);
  if (n > 1) inverse[1] = 1;
  for (int i = 2; i < n; ++i) inverse[i] = -Mint(Mint::mod() / i) * inverse[Mint::mod() % i];
  for (int i = 1; i < n; ++i) {
    Mint value = 0;
    if (sparse.size() < 4) {
      for (const auto& [degree, coefficient] : sparse) {
        if (degree > i) break;
        value += coefficient * result[i - degree];
      }
    } else {
      // At most block products fit in uint64_t, for every supported prime.
      constexpr std::uint64_t largest = std::uint64_t(Mint::mod() - 1) * (Mint::mod() - 1);
      constexpr auto block = std::min<std::uint64_t>(16, std::numeric_limits<std::uint64_t>::max() / largest);
      std::uint64_t sum = 0, used = 0;
      for (const auto& [degree, coefficient] : sparse) {
        if (degree > i) break;
        sum += std::uint64_t(coefficient.val()) * result[i - degree].val();
        if (++used == block) {
          value += Mint::raw(static_cast<int>(sum % Mint::mod()));
          sum = used = 0;
        }
      }
      value += Mint::raw(static_cast<int>(sum % Mint::mod()));
    }
    result[i] = value * inverse[i];
  }
  return result;
}

}  // namespace blueberry

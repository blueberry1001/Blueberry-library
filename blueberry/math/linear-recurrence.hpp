#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

#include <atcoder/convolution>

namespace blueberry {

// Shortest recurrence fitting the supplied prefix over a field.
// a[i] = sum(coeff[j] * a[i-j-1]); an empty/all-zero prefix returns {}.
template <class Mint>
std::vector<Mint> berlekamp_massey(const std::vector<Mint>& sequence) {
  assert(sequence.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
  std::vector<Mint> current{Mint(1)}, previous{Mint(1)};
  int degree = 0, shift = 1;
  Mint last_discrepancy = 1;
  for (int i = 0; i < static_cast<int>(sequence.size()); ++i) {
    Mint discrepancy = sequence[i];
    for (int j = 1; j <= degree; ++j) discrepancy += current[j] * sequence[i - j];
    if (discrepancy == Mint(0)) { ++shift; continue; }
    const Mint scale = discrepancy / last_discrepancy;
    auto saved = current;
    if (current.size() < previous.size() + shift) current.resize(previous.size() + shift);
    for (std::size_t j = 0; j < previous.size(); ++j) current[j + shift] -= scale * previous[j];
    if (degree <= i / 2) {
      degree = i + 1 - degree;
      previous = std::move(saved);
      last_discrepancy = discrepancy;
      shift = 1;
    } else {
      ++shift;
    }
  }
  std::vector<Mint> coefficients(degree);
  for (int j = 0; j < degree; ++j) coefficients[j] = -current[j + 1];
  return coefficients;
}

// Bostan--Mori using ACL convolution. Mint is an ACL static prime modint;
// the power of two covering 2*d+1 must divide Mint::mod()-1.
template <class Mint>
Mint linear_recurrence_kth(const std::vector<Mint>& initial,
                           const std::vector<Mint>& coefficients, std::uint64_t k) {
  static_assert(atcoder::internal::is_static_modint<Mint>::value);
  assert(initial.size() == coefficients.size());
  const std::size_t d = coefficients.size();
  if (d == 0) return Mint(0);
  if (k < d) return initial[static_cast<std::size_t>(k)];
  assert(d < static_cast<std::size_t>(std::numeric_limits<int>::max()) / 2);
  [[maybe_unused]] std::size_t length = 1;
  while (length < 2 * d + 1) length *= 2;
  assert((Mint::mod() - 1) % length == 0);
  std::vector<Mint> q(d + 1);
  q[0] = 1;
  for (std::size_t j = 0; j < d; ++j) q[j + 1] = -coefficients[j];
  auto p = atcoder::convolution(initial, q);
  p.resize(d);
  while (k) {
    auto negative = q;
    for (std::size_t j = 1; j <= d; j += 2) negative[j] = -negative[j];
    auto numerator = atcoder::convolution(p, negative);
    auto denominator = atcoder::convolution(q, negative);
    for (std::size_t j = 0; j < d; ++j) p[j] = numerator[2 * j + (k & 1)];
    for (std::size_t j = 0; j <= d; ++j) q[j] = denominator[2 * j];
    k >>= 1;
  }
  return p[0] / q[0];
}

}  // namespace blueberry

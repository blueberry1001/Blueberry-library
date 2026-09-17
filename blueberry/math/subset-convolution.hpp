#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

namespace blueberry {

// c[S] = sum_{A subset S} a[A] b[S\A], by ranked zeta/Mobius transforms.
template <class T>
std::vector<T> subset_convolution(const std::vector<T>& a, const std::vector<T>& b) {
  const std::size_t n = a.size();
  assert(n == b.size() && (n == 0 || std::has_single_bit(n)));
  if (n == 0) return {};
  const int bits = std::countr_zero(n), ranks = bits + 1;
  assert(n <= std::numeric_limits<std::size_t>::max() / static_cast<std::size_t>(ranks));
  std::vector<T> f(n * ranks), g(n * ranks);
  for (std::size_t mask = 0; mask < n; ++mask) {
    const int rank = std::popcount(mask);
    f[mask * ranks + rank] = a[mask]; g[mask * ranks + rank] = b[mask];
  }
  for (std::size_t half = 1; half < n; half <<= 1)
    for (std::size_t start = 0; start < n; start += half * 2)
      for (std::size_t mask = start; mask < start + half; ++mask) {
        const int rank = std::popcount(mask);
        const std::size_t from = mask * ranks, to = (mask + half) * ranks;
        for (int k = 0; k <= rank; ++k) {
          f[to + k] += f[from + k]; g[to + k] += g[from + k];
        }
      }
  for (std::size_t mask = 0; mask < n; ++mask) {
    const int rank = std::popcount(mask);
    const std::size_t base = mask * ranks;
    // Descending degree preserves the original lower-degree coefficients of f.
    for (int k = std::min(bits, 2 * rank); k >= 0; --k) {
      T value{};
      for (int j = std::max(0, k - rank); j <= std::min(k, rank); ++j)
        value += f[base + j] * g[base + k - j];
      f[base + k] = value;
    }
  }
  for (std::size_t half = 1; half < n; half <<= 1)
    for (std::size_t start = 0; start < n; start += half * 2)
      for (std::size_t mask = start; mask < start + half; ++mask) {
        const int rank = std::popcount(mask);
        const std::size_t from = mask * ranks, to = (mask + half) * ranks;
        // Only ranks >= |target| can contribute to a final diagonal entry.
        for (int k = rank + 1; k <= std::min(bits, 2 * rank); ++k)
          f[to + k] -= f[from + k];
      }
  std::vector<T> result(n);
  for (std::size_t mask = 0; mask < n; ++mask) result[mask] = f[mask * ranks + std::popcount(mask)];
  return result;
}
}  // namespace blueberry

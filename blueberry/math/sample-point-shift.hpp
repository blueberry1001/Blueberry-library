#pragma once

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstddef>
#include <vector>
#include <atcoder/convolution>

namespace blueberry {
template <class Mint>
std::vector<Mint> sample_point_shift(const std::vector<Mint>& y, Mint start, int count) {
  static_assert(atcoder::internal::is_static_modint<Mint>::value);
  assert(count >= 0);
  assert(y.size() < static_cast<std::size_t>(Mint::mod()));
  assert(count < Mint::mod());
  assert(2 * y.size() + static_cast<std::size_t>(count) <= static_cast<std::size_t>(INT_MAX));
  const int n = static_cast<int>(y.size());
  std::vector<Mint> result(count);
  if (n == 0 || count == 0) return result;
  [[maybe_unused]] std::size_t transform = 1;
  while (transform < 2 * y.size() + static_cast<std::size_t>(count)) transform *= 2;
  assert((Mint::mod() - 1) % transform == 0);
  std::vector<Mint> invfact(n, Mint(1)), weight(n);
  for (int i = 1; i < n; ++i) invfact[i] = invfact[i - 1] * Mint(i);
  invfact[n - 1] = invfact[n - 1].inv();
  for (int i = n - 1; i; --i) invfact[i - 1] = invfact[i] * Mint(i);
  for (int i = 0; i < n; ++i) {
    weight[i] = y[i] * invfact[i] * invfact[n - 1 - i];
    if ((n - 1 - i) & 1) weight[i] = -weight[i];
  }
  int done = 0, s = start.val();
  while (done < count) {
    if (s < n) {
      result[done++] = y[s++];
    } else {
      const int length = std::min(count - done, Mint::mod() - s);
      const int k = n + length - 1;
      std::vector<Mint> inverse(k), prefix(k + 1, Mint(1));
      for (int i = 0; i < k; ++i) prefix[i + 1] = prefix[i] * Mint(s - n + 1 + i);
      Mint suffix = prefix[k].inv();
      for (int i = k - 1; i >= 0; --i) {
        inverse[i] = prefix[i] * suffix;
        suffix *= Mint(s - n + 1 + i);
      }
      const auto convolution = atcoder::convolution(weight, inverse);
      Mint product = prefix[n];
      for (int i = 0; i < length; ++i) {
        result[done + i] = convolution[n - 1 + i] * product;
        product *= Mint(s + i + 1) * inverse[i];
      }
      done += length;
      s += length;
    }
    if (s == Mint::mod()) s = 0;
  }
  return result;
}
}  // namespace blueberry

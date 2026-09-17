#pragma once
#include <cassert>
#include <cstddef>
#include <vector>

namespace blueberry {
namespace detail {
template <bool Divisor, bool Inverse, class T>
void divisor_transform(std::vector<T>& a) {
  const std::size_t n = a.size();
  std::vector<bool> composite(n);
  for (std::size_t p = 2; p < n; ++p) {
    if (composite[p]) continue;
    const auto count = (n - 1) / p;
    for (std::size_t j = 1; j <= count; ++j) {
      composite[j * p] = true;
      const auto i = (Divisor != Inverse) ? j : count + 1 - j;
      if constexpr (Divisor) {
        if constexpr (Inverse) a[i * p] -= a[i];
        else a[i * p] += a[i];
      } else {
        if constexpr (Inverse) a[i] -= a[i * p];
        else a[i] += a[i * p];
      }
    }
  }
}
}  // namespace detail
template <class T> void divisor_zeta(std::vector<T>& a) {
  detail::divisor_transform<true, false>(a);
}
template <class T> void divisor_mobius(std::vector<T>& a) {
  detail::divisor_transform<true, true>(a);
}
template <class T> void multiple_zeta(std::vector<T>& a) {
  detail::divisor_transform<false, false>(a);
}
template <class T> void multiple_mobius(std::vector<T>& a) {
  detail::divisor_transform<false, true>(a);
}
template <class T>
std::vector<T> gcd_convolution(std::vector<T> a, std::vector<T> b) {
  assert(a.size() == b.size());
  multiple_zeta(a);
  multiple_zeta(b);
  for (std::size_t i = 1; i < a.size(); ++i) a[i] *= b[i];
  multiple_mobius(a);
  if (!a.empty()) a[0] = T{};
  return a;
}
template <class T>
std::vector<T> lcm_convolution(std::vector<T> a, std::vector<T> b) {
  assert(a.size() == b.size());
  divisor_zeta(a);
  divisor_zeta(b);
  for (std::size_t i = 1; i < a.size(); ++i) a[i] *= b[i];
  divisor_mobius(a);
  if (!a.empty()) a[0] = T{};
  return a;
}
}  // namespace blueberry

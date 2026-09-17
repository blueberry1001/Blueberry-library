#pragma once

#include <bit>
#include <cassert>
#include <cstddef>
#include <vector>

namespace blueberry {
namespace bitwise_convolution_detail {
template <class T>
void transform(std::vector<T>& a, int kind, bool inverse) {
  const std::size_t n = a.size();
  for (std::size_t half = 1; half < n; half <<= 1)
    for (std::size_t start = 0; start < n; start += half * 2)
      for (std::size_t j = start; j < start + half; ++j) {
        if (kind == 0) {
          if (inverse) a[j] -= a[j + half]; else a[j] += a[j + half];
        } else if (kind == 1) {
          if (inverse) a[j + half] -= a[j]; else a[j + half] += a[j];
        } else {
          T x = a[j], y = a[j + half];
          a[j] = x + y; a[j + half] = x - y;
        }
      }
}
template <class T>
void check(const std::vector<T>& a, const std::vector<T>& b) {
  assert(a.size() == b.size() && (a.empty() || std::has_single_bit(a.size())));
  (void)a; (void)b;
}
}  // namespace bitwise_convolution_detail

template <class T>
std::vector<T> bitwise_convolution_and(std::vector<T> a, std::vector<T> b) {
  bitwise_convolution_detail::check(a, b);
  bitwise_convolution_detail::transform(a, 0, false);
  bitwise_convolution_detail::transform(b, 0, false);
  for (std::size_t i = 0; i < a.size(); ++i) a[i] *= b[i];
  bitwise_convolution_detail::transform(a, 0, true);
  return a;
}
template <class T>
std::vector<T> bitwise_convolution_or(std::vector<T> a, std::vector<T> b) {
  bitwise_convolution_detail::check(a, b);
  bitwise_convolution_detail::transform(a, 1, false);
  bitwise_convolution_detail::transform(b, 1, false);
  for (std::size_t i = 0; i < a.size(); ++i) a[i] *= b[i];
  bitwise_convolution_detail::transform(a, 1, true);
  return a;
}
template <class T>
std::vector<T> bitwise_convolution_xor(std::vector<T> a, std::vector<T> b) {
  bitwise_convolution_detail::check(a, b);
  if (a.empty()) return {};
  bitwise_convolution_detail::transform(a, 2, false);
  bitwise_convolution_detail::transform(b, 2, false);
  for (std::size_t i = 0; i < a.size(); ++i) a[i] *= b[i];
  bitwise_convolution_detail::transform(a, 2, false);
  const T inverse_size = T(1) / T(a.size());
  for (T& value : a) value *= inverse_size;
  return a;
}
}  // namespace blueberry

#pragma once
#include <cstddef>
#include <type_traits>
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
template <int Kind, bool Inverse, class T>
void transform4(std::vector<T>& a) {
  const std::size_t n = a.size();
#if defined(_MSC_VER) && !defined(__clang__)
  if constexpr (Kind != 2 && (std::is_integral_v<T> || sizeof(T) > 4)) {
#else
  if constexpr (Kind != 2) {
#endif
    for (std::size_t half = 1; half < n; half <<= 1)
      for (std::size_t start = 0; start < n; start += half * 2)
        for (std::size_t j = start; j < start + half; ++j) {
          if constexpr (Kind == 0) { if constexpr (Inverse) a[j] -= a[j + half]; else a[j] += a[j + half]; }
          else { if constexpr (Inverse) a[j + half] -= a[j]; else a[j + half] += a[j]; }
        }
    return;
  }
  std::size_t half = 1;
  for (; half <= n / 4; half *= 4) {
    for (std::size_t start = 0; start < n; start += half * 4) {
      for (std::size_t j = start; j < start + half; ++j) {
        auto &x0 = a[j], &x1 = a[j + half], &x2 = a[j + half * 2], &x3 = a[j + half * 3];
        if constexpr (Kind == 0) {
          if constexpr (Inverse) { x0 -= x1; x2 -= x3; x0 -= x2; x1 -= x3; }
          else { x0 += x1; x2 += x3; x0 += x2; x1 += x3; }
        } else if constexpr (Kind == 1) {
          if constexpr (Inverse) { x1 -= x0; x3 -= x2; x2 -= x0; x3 -= x1; }
          else { x1 += x0; x3 += x2; x2 += x0; x3 += x1; }
        } else {
          T a0 = x0, a1 = x1, a2 = x2, a3 = x3;
          x0 = a0 + a1; x1 = a0 - a1; x2 = a2 + a3; x3 = a2 - a3;
          T b0 = x0, b1 = x1, b2 = x2, b3 = x3;
          x0 = b0 + b2; x2 = b0 - b2; x1 = b1 + b3; x3 = b1 - b3;
        }
      }
    }
  }
  if (half < n) for (std::size_t start = 0; start < n; start += half * 2) {
    for (std::size_t j = start; j < start + half; ++j) {
      if constexpr (Kind == 0) { if constexpr (Inverse) a[j] -= a[j + half]; else a[j] += a[j + half]; }
      else if constexpr (Kind == 1) { if constexpr (Inverse) a[j + half] -= a[j]; else a[j + half] += a[j]; }
      else { T x = a[j], y = a[j + half]; a[j] = x + y; a[j + half] = x - y; }
    }
  }
}
}
template <class T> std::vector<T> bitwise_convolution_and(std::vector<T> a, std::vector<T> b) {
#if defined(_MSC_VER) && !defined(__clang__)
  if constexpr (!std::is_integral_v<T> && sizeof(T) <= 4) {
    bitwise_convolution_detail::transform4<0, false>(a);
    bitwise_convolution_detail::transform4<0, false>(b);
    for (std::size_t i = 0; i < a.size(); ++i) a[i] *= b[i];
    bitwise_convolution_detail::transform4<0, true>(a);
    return a;
  }
#endif
  bitwise_convolution_detail::transform(a, 0, false);
  bitwise_convolution_detail::transform(b, 0, false);
  for (std::size_t i = 0; i < a.size(); ++i) a[i] *= b[i];
  bitwise_convolution_detail::transform(a, 0, true);
  return a;
}
template <class T> std::vector<T> bitwise_convolution_or(std::vector<T> a, std::vector<T> b) {
#if defined(_MSC_VER) && !defined(__clang__)
  if constexpr (!std::is_integral_v<T> && sizeof(T) <= 4) {
    bitwise_convolution_detail::transform4<1, false>(a);
    bitwise_convolution_detail::transform4<1, false>(b);
    for (std::size_t i = 0; i < a.size(); ++i) a[i] *= b[i];
    bitwise_convolution_detail::transform4<1, true>(a);
    return a;
  }
#endif
  bitwise_convolution_detail::transform(a, 1, false);
  bitwise_convolution_detail::transform(b, 1, false);
  for (std::size_t i = 0; i < a.size(); ++i) a[i] *= b[i];
  bitwise_convolution_detail::transform(a, 1, true);
  return a;
}
template <class T> std::vector<T> bitwise_convolution_xor(std::vector<T> a, std::vector<T> b) {
  if (a.empty()) return {};
  bitwise_convolution_detail::transform4<2, false>(a);
  bitwise_convolution_detail::transform4<2, false>(b);
  for (std::size_t i = 0; i < a.size(); ++i) a[i] *= b[i];
  bitwise_convolution_detail::transform4<2, false>(a);
  const T inverse_size = T(1) / T(a.size());
  for (T& value : a) value *= inverse_size;
  return a;
}
}

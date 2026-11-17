#pragma once
#include <bit>
#include <cstdint>

namespace blueberry {
namespace detail {
template <int K>
inline std::uint64_t kth_root_newton(std::uint64_t n, int width) {
  std::uint64_t x = std::uint64_t{1} << ((width + K - 1) / K);
  while (true) {
    std::uint64_t power = 1;
    for (int i = 1; i < K; ++i) power *= x;
    const std::uint64_t next = ((K - 1) * x + n / power) / K;
    if (next >= x) return x;
    x = next;
  }
}
}
inline std::uint64_t kth_root_integer(std::uint64_t n, int k) {
  if (k == 1 || n <= 1) return n;
  const int width = static_cast<int>(std::bit_width(n));
  if (k >= width) return 1;
  switch (k) {
    case 2: return detail::kth_root_newton<2>(n, width);
    case 3: return detail::kth_root_newton<3>(n, width);
    case 4: return detail::kth_root_newton<4>(n, width);
    case 5: return detail::kth_root_newton<5>(n, width);
    case 6: return detail::kth_root_newton<6>(n, width);
    case 7: return detail::kth_root_newton<7>(n, width);
    case 8: return detail::kth_root_newton<8>(n, width);
  }
#if defined(__GNUC__) || defined(__clang__)
  std::uint64_t root = 0;
  for (int bit = (width - 1) / k; bit >= 0; --bit) {
    std::uint64_t x = root | (std::uint64_t{1} << bit), value = 1;
    int exponent = k;
    bool fits = true;
    while (exponent) {
      if (exponent & 1)
        if (__builtin_mul_overflow(value, x, &value) || value > n) { fits = false; break; }
      exponent >>= 1;
      if (!exponent) break;
      if (__builtin_mul_overflow(x, x, &x) || x > n) { fits = false; break; }
    }
    if (fits) root |= std::uint64_t{1} << bit;
  }
  return root;
#else
  std::uint64_t lo = 1, hi = std::uint64_t{1} << ((width + k - 1) / k);
  while (hi - lo > 1) {
    const std::uint64_t x = lo + (hi - lo) / 2, bound = n / x;
    std::uint64_t value = 1;
    bool fits = true;
    for (int i = 0; i < k; ++i) {
      if (value > bound) { fits = false; break; }
      value *= x;
    }
    if (fits) lo = x; else hi = x;
  }
  return lo;
#endif
}
}

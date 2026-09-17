#pragma once
#include <atcoder/math>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <unordered_map>

namespace blueberry {
inline std::optional<std::int64_t> discrete_log(std::int64_t a, std::int64_t b,
                                               std::int64_t m) {
  assert(1 <= m && m <= std::numeric_limits<std::int32_t>::max());
  a %= m;
  b %= m;
  if (a < 0) a += m;
  if (b < 0) b += m;
  std::int64_t factor = 1 % m, offset = 0;
  while (true) {
    if (b == factor) return offset;
    auto g = std::gcd(a, m);
    if (g == 1) break;
    if (b % g != 0) return std::nullopt;
    b /= g;
    m /= g;
    factor = factor * (a / g) % m;
    ++offset;
  }
  b = b * atcoder::inv_mod(factor, m) % m;
  const std::int64_t width = static_cast<std::int64_t>(std::sqrt(m)) + 1;
  std::unordered_map<std::int64_t, std::int64_t> baby;
  baby.reserve(static_cast<std::size_t>(width));
  std::int64_t power = 1;
  for (std::int64_t j = 0; j < width; ++j) {
    baby.emplace(power, j);
    power = power * a % m;
  }
  const auto step = atcoder::inv_mod(power, m);
  for (std::int64_t i = 0; i <= width; ++i) {
    auto it = baby.find(b);
    if (it != baby.end()) return offset + i * width + it->second;
    b = b * step % m;
  }
  return std::nullopt;
}
}  // namespace blueberry

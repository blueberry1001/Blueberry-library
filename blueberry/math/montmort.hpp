#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace blueberry {

// D(0)..D(n), with D(0)=1, under any positive 32-bit modulus.
inline std::vector<std::uint32_t> montmort(int n, std::uint32_t mod) {
  assert(n >= 0 && mod > 0);
  std::vector<std::uint32_t> result(static_cast<std::size_t>(n) + 1);
  result[0] = 1 % mod;
  for (std::size_t i = 1; i < result.size(); ++i) {
    const std::uint64_t product = i * std::uint64_t(result[i - 1]);
    result[i] = static_cast<std::uint32_t>((product + ((i & 1) ? mod - 1 : 1)) % mod);
  }
  return result;
}

}  // namespace blueberry

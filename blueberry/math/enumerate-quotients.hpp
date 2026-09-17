#pragma once
#include <cassert>
#include <cstdint>
#include <vector>

namespace blueberry {
inline std::vector<std::int64_t> enumerate_quotients(std::int64_t n) {
  assert(n >= 0);
  std::vector<std::int64_t> result;
  std::int64_t i = 1;
  for (; i <= n / i; ++i) result.push_back(i);
  for (--i; i >= 1; --i) {
    auto q = n / i;
    if (q != result.back()) result.push_back(q);
  }
  return result;
}
}  // namespace blueberry

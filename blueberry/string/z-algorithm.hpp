#pragma once

#include <algorithm>
#include <vector>

namespace blueberry {

/**
 * @brief Z Algorithm
 */
template <class Sequence>
std::vector<int> z_algorithm(const Sequence& sequence) {
  const int n = static_cast<int>(sequence.size());
  if (n == 0) return {};
  std::vector<int> z(n);
  z[0] = n;
  for (int i = 1, matched = 0; i < n;) {
    while (i + matched < n && sequence[matched] == sequence[i + matched]) ++matched;
    z[i] = matched;
    if (matched == 0) {
      ++i;
      continue;
    }
    int offset = 1;
    while (offset < matched && offset + z[offset] < matched) {
      z[i + offset] = z[offset];
      ++offset;
    }
    i += offset;
    matched -= offset;
  }
  return z;
}

}  // namespace blueberry

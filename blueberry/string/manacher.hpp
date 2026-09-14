#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

namespace blueberry {

/**
 * @brief Longest palindrome lengths at all character and gap centers, in O(N).
 */
template <class Sequence>
std::vector<int> manacher(const Sequence& sequence) {
  assert(sequence.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max() / 2));
  const int n = static_cast<int>(sequence.size());
  if (n == 0) return {};
  std::vector<int> length(2 * n - 1);
  // parity 0: center at i; parity 1: center between i and i + 1.
  // Keep the center whose palindrome has the furthest exclusive right end.
  for (int parity = 0; parity < 2; ++parity) {
    int center = 0, right = 0;
    for (int i = 0; i < n - parity; ++i) {
      int radius = 0;
      if (i + 1 < right) {
        const int mirror = 2 * center - i;
        radius = std::min(length[2 * mirror + parity] / 2, right - i - 1);
      }
      while (radius < i + parity && i + radius + 1 < n &&
             sequence[i - radius - 1 + parity] == sequence[i + radius + 1]) {
        ++radius;
      }
      length[2 * i + parity] = 2 * radius + 1 - parity;
      if (i + radius + 1 > right) {
        center = i;
        right = i + radius + 1;
      }
    }
  }
  return length;
}

}  // namespace blueberry

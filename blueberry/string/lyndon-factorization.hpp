#pragma once

#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

namespace blueberry {

// Duval: factor boundaries, including 0 and n; the empty sequence returns {0}.
template <class Sequence>
std::vector<int> lyndon_factorization(const Sequence& sequence) {
  assert(sequence.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
  const int n = static_cast<int>(sequence.size());
  std::vector<int> boundaries{0};
  int begin = 0;
  while (begin < n) {
    int scan = begin + 1, matched = begin;
    while (scan < n && !(sequence[scan] < sequence[matched])) {
      matched = sequence[matched] < sequence[scan] ? begin : matched + 1;
      ++scan;
    }
    const int period = scan - matched;
    while (begin <= matched) {
      begin += period;
      boundaries.push_back(begin);
    }
  }
  return boundaries;
}

}  // namespace blueberry

#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

namespace blueberry {

// Return indices of one longest strictly increasing subsequence.
template <class T>
std::vector<int> longest_increasing_subsequence(const std::vector<T>& values) {
  assert(values.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
  const int n = static_cast<int>(values.size());
  std::vector<int> tails, previous(values.size(), -1);
  tails.reserve(values.size());
  for (int i = 0; i < n; ++i) {
    auto it = std::lower_bound(tails.begin(), tails.end(), values[i],
                              [&](int j, const T& value) { return values[j] < value; });
    if (it != tails.begin()) previous[i] = *(it - 1);
    if (it == tails.end()) tails.push_back(i);
    else *it = i;
  }
  std::vector<int> result;
  if (!tails.empty()) {
    for (int i = tails.back(); i != -1; i = previous[i]) result.push_back(i);
    std::reverse(result.begin(), result.end());
  }
  return result;
}

}  // namespace blueberry

#pragma once

#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

namespace blueberry {

template <class Sequence>
std::vector<int> prefix_function(const Sequence& sequence) {
  assert(sequence.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
  const int n = static_cast<int>(sequence.size());
  std::vector<int> prefix(n);
  for (int i = 1; i < n; ++i) {
    int j = prefix[i - 1];
    while (j > 0 && !(sequence[i] == sequence[j])) j = prefix[j - 1];
    if (sequence[i] == sequence[j]) ++j;
    prefix[i] = j;
  }
  return prefix;
}

template <class Text, class Pattern>
std::vector<int> kmp_search(const Text& text, const Pattern& pattern) {
  assert(text.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
  assert(pattern.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
  const int n = static_cast<int>(text.size()), m = static_cast<int>(pattern.size());
  std::vector<int> result;
  if (m == 0) {
    result.resize(static_cast<std::size_t>(n) + 1);
    // Avoid incrementing an int beyond INT_MAX for the last boundary.
    for (std::size_t i = 0; i < result.size(); ++i) result[i] = static_cast<int>(i);
    return result;
  }
  const auto prefix = prefix_function(pattern);
  for (int i = 0, j = 0; i < n; ++i) {
    while (j > 0 && !(text[i] == pattern[j])) j = prefix[j - 1];
    if (text[i] == pattern[j]) ++j;
    if (j == m) {
      result.push_back(i - m + 1);
      j = prefix[j - 1];
    }
  }
  return result;
}

}  // namespace blueberry

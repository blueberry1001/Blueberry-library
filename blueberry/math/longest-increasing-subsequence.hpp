#pragma once
#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <vector>

namespace blueberry {
template <class T>
std::vector<int> longest_increasing_subsequence(const std::vector<T>& values) {
  const int n = static_cast<int>(values.size());
  if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool> && sizeof(T) <= 8) {
    std::vector<int> rank(n);
    std::vector<T> tails; tails.reserve(values.size());
    for (int i = 0; i < n; ++i) {
      const T v = values[i];
      if (tails.empty() || tails.back() < v) { tails.push_back(v); rank[i] = static_cast<int>(tails.size()); continue; }
      if (tails.size() == 1) { tails[0] = v; rank[i] = 1; continue; }
      std::size_t pos = 0;
#if defined(_MSC_VER) && !defined(__clang__)
      std::size_t width = tails.size() + 1;
      while (width > 1) { auto half = width / 2; width -= half; pos += std::size_t(tails[pos + half - 1] < v) * half; }
#else
      pos = std::lower_bound(tails.begin(), tails.end(), v) - tails.begin();
#endif
      tails[pos] = v; rank[i] = static_cast<int>(pos) + 1;
    }
    int need = static_cast<int>(tails.size()); std::vector<int> result(need);
    for (int i = n; i-- && need;) if (rank[i] == need) result[--need] = i;
    return result;
  } else {
    std::vector<int> tails, previous(values.size(), -1); tails.reserve(values.size());
    for (int i = 0; i < n; ++i) {
      auto it = std::lower_bound(tails.begin(), tails.end(), values[i],
                                [&](int j, const T& value) { return values[j] < value; });
      if (it != tails.begin()) previous[i] = *(it - 1);
      if (it == tails.end()) tails.push_back(i); else *it = i;
    }
    std::vector<int> result;
    if (!tails.empty()) {
      for (int i = tails.back(); i != -1; i = previous[i]) result.push_back(i);
      std::reverse(result.begin(), result.end());
    }
    return result;
  }
}
}

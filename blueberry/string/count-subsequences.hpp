#pragma once

#include <algorithm>
#include <cstddef>
#include <numeric>
#include <vector>

namespace blueberry {

// Number of distinct nonempty subsequences. Symbols need a strict weak order;
// Count needs exact + and - (for example an ACL modint).
template <class Count, class Sequence>
Count count_subsequences(const Sequence& sequence) {
  const std::size_t n = sequence.size();
  std::vector<std::size_t> order(n), rank(n);
  std::iota(order.begin(), order.end(), std::size_t{0});
  std::sort(order.begin(), order.end(), [&](auto i, auto j) {
    return sequence[i] < sequence[j];
  });
  std::size_t kinds = 0;
  for (std::size_t i = 0; i < n; ++i) {
    if (i == 0 || sequence[order[i - 1]] < sequence[order[i]]) ++kinds;
    rank[order[i]] = kinds - 1;
  }
  std::vector<Count> previous(kinds, Count(0));
  Count total(1);
  for (auto symbol : rank) {
    const Count next = total + total - previous[symbol];
    previous[symbol] = total;
    total = next;
  }
  return total - Count(1);
}

}  // namespace blueberry

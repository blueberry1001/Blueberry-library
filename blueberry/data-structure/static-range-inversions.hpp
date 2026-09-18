#pragma once
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>
#include <limits>
#include <utility>
#include <vector>
#include <atcoder/fenwicktree>

namespace blueberry {
template <class T>
std::vector<long long> static_range_inversions(
    const std::vector<T>& a, const std::vector<std::pair<int, int>>& queries) {
  assert(a.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
  assert(queries.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
  const int n = static_cast<int>(a.size()), q = static_cast<int>(queries.size());
  for ([[maybe_unused]] auto [l, r] : queries) assert(0 <= l && l <= r && r <= n);
  std::vector<T> values = a;
  std::sort(values.begin(), values.end());
  values.erase(std::unique(values.begin(), values.end()), values.end());
  std::vector<int> rank(n), order(q);
  for (int i = 0; i < n; ++i)
    rank[i] = static_cast<int>(std::lower_bound(values.begin(), values.end(), a[i]) - values.begin());
  std::iota(order.begin(), order.end(), 0);
  const int block = std::max(1, static_cast<int>(std::sqrt(std::max(1, n))));
  std::sort(order.begin(), order.end(), [&](int x, int y) {
    int bx = queries[x].first / block, by = queries[y].first / block;
    if (bx != by) return bx < by;
    return (bx & 1) ? queries[x].second > queries[y].second : queries[x].second < queries[y].second;
  });
  atcoder::fenwick_tree<int> bit(static_cast<int>(values.size()));
  std::vector<long long> answer(q);
  int l = 0, r = 0;
  long long inversions = 0;
  for (int id : order) {
    auto [ql, qr] = queries[id];
    while (l > ql) { int x = rank[--l]; inversions += bit.sum(0, x); bit.add(x, 1); }
    while (r < qr) { int x = rank[r]; inversions += r - l - bit.sum(0, x + 1); bit.add(x, 1); ++r; }
    while (l < ql) { int x = rank[l++]; inversions -= bit.sum(0, x); bit.add(x, -1); }
    while (r > qr) { int x = rank[--r]; bit.add(x, -1); inversions -= r - l - bit.sum(0, x + 1); }
    answer[id] = inversions;
  }
  return answer;
}
}  // namespace blueberry

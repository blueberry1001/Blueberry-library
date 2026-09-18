#pragma once
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>
#include <limits>
#include <utility>
#include <vector>

namespace blueberry {
// Each result is {a representative input index of a mode value, its frequency}.
// The representative may be outside the queried interval; empty returns {-1, 0}.
template <class T>
std::vector<std::pair<int, int>> static_range_mode(
    const std::vector<T>& a, const std::vector<std::pair<int, int>>& queries) {
  assert(a.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
  assert(queries.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
  const int n = static_cast<int>(a.size()), q = static_cast<int>(queries.size());
  for ([[maybe_unused]] auto [l, r] : queries) assert(0 <= l && l <= r && r <= n);
  std::vector<T> values = a;
  std::sort(values.begin(), values.end());
  values.erase(std::unique(values.begin(), values.end()), values.end());
  const int m = static_cast<int>(values.size());
  std::vector<int> rank(n), representative(m), order(q);
  for (int i = 0; i < n; ++i) {
    rank[i] = static_cast<int>(std::lower_bound(values.begin(), values.end(), a[i]) - values.begin());
    representative[rank[i]] = i;
  }
  std::iota(order.begin(), order.end(), 0);
  const int block = std::max(1, static_cast<int>(std::sqrt(std::max(1, n))));
  std::sort(order.begin(), order.end(), [&](int x, int y) {
    int bx = queries[x].first / block, by = queries[y].first / block;
    if (bx != by) return bx < by;
    return (bx & 1) ? queries[x].second > queries[y].second : queries[x].second < queries[y].second;
  });
  std::vector<int> count(m), head(n + 1, -1), prev(m, -1), next(m, -1);
  int maximum = 0;
  auto change = [&](int x, int delta) {
    int f = count[x];
    if (f) {
      if (prev[x] == -1) head[f] = next[x]; else next[prev[x]] = next[x];
      if (next[x] != -1) prev[next[x]] = prev[x];
    }
    f = (count[x] += delta);
    if (f) {
      prev[x] = -1; next[x] = head[f];
      if (head[f] != -1) prev[head[f]] = x;
      head[f] = x;
    }
    maximum = std::max(maximum, f);
    if (maximum && head[maximum] == -1) --maximum;
  };
  std::vector<std::pair<int, int>> answer(q);
  int l = 0, r = 0;
  for (int id : order) {
    auto [ql, qr] = queries[id];
    while (l > ql) change(rank[--l], 1);
    while (r < qr) change(rank[r++], 1);
    while (l < ql) change(rank[l++], -1);
    while (r > qr) change(rank[--r], -1);
    answer[id] = {maximum ? representative[head[maximum]] : -1, maximum};
  }
  return answer;
}
}  // namespace blueberry

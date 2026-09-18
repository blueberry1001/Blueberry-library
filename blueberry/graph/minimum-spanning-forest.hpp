#pragma once
#include <algorithm>
#include <cassert>
#include <numeric>
#include <tuple>
#include <utility>
#include <vector>
#include <atcoder/dsu>

namespace blueberry {
template <class T>
std::pair<T, std::vector<int>> minimum_spanning_forest(int n, const std::vector<std::tuple<int, int, T>>& edges) {
  assert(n >= 0);
  std::vector<int> order(edges.size());
  std::iota(order.begin(), order.end(), 0);
  for (const auto& [u, v, w] : edges) { assert(0 <= u && u < n && 0 <= v && v < n); (void)u; (void)v; (void)w; }
  std::sort(order.begin(), order.end(), [&](int a, int b) {
    return std::get<2>(edges[a]) < std::get<2>(edges[b]);
  });
  atcoder::dsu dsu(n);
  T total{};
  std::vector<int> selected;
  if (n > 0) selected.reserve(n - 1);
  for (int id : order) {
    const auto& [u, v, weight] = edges[id];
    if (dsu.same(u, v)) continue;
    dsu.merge(u, v); total += weight; selected.push_back(id);
  }
  return {total, std::move(selected)};
}
}  // namespace blueberry

#pragma once
#include <cassert>
#include <utility>
#include <vector>
#include "blueberry/math/matrix.hpp"

namespace blueberry {
template <class T>
T count_spanning_trees(int n, const std::vector<std::pair<int, int>>& edges) {
  assert(n >= 0);
  Matrix<T> lap(n == 0 ? 0 : n - 1, n == 0 ? 0 : n - 1);
  for (auto [u, v] : edges) {
    assert(0 <= u && u < n && 0 <= v && v < n);
    if (u == v) continue;
    if (u < n - 1) lap(u, u) += T(1);
    if (v < n - 1) lap(v, v) += T(1);
    if (u < n - 1 && v < n - 1) { lap(u, v) -= T(1); lap(v, u) -= T(1); }
  }
  return lap.det();
}

// Out-arborescences: every vertex is reachable from root.
template <class T>
T count_directed_spanning_trees(int n, const std::vector<std::pair<int, int>>& edges, int root) {
  assert(n > 0 && 0 <= root && root < n);
  Matrix<T> lap(n - 1, n - 1);
  for (auto [u, v] : edges) {
    assert(0 <= u && u < n && 0 <= v && v < n);
    if (u == v || v == root) continue;
    const int a = u - (u > root), b = v - (v > root);
    lap(b, b) += T(1);
    if (u != root) lap(b, a) -= T(1);
  }
  return lap.det();
}
}  // namespace blueberry

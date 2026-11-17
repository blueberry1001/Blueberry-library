#include <algorithm>
#include <cassert>
#include <random>
#include <utility>
#include <vector>
#include "blueberry/graph/hopcroft-karp.hpp"
using Edges = std::vector<std::pair<int, int>>;

int brute(int n, int m, const Edges& edges) {
  std::vector<int> masks(n);
  for (auto [u, v] : edges) masks[u] |= 1 << v;
  std::vector<int> dp(1 << m, -1); dp[0] = 0;
  for (int u = 0; u < n; ++u) {
    auto next = dp;
    for (int mask = 0; mask < (1 << m); ++mask) if (dp[mask] >= 0)
      for (int v = 0; v < m; ++v) if ((masks[u] & (1 << v)) && !(mask & (1 << v)))
        next[mask | (1 << v)] = std::max(next[mask | (1 << v)], dp[mask] + 1);
    dp.swap(next);
  }
  return *std::max_element(dp.begin(), dp.end());
}

void check(int n, int m, const Edges& edges, int expected) {
  blueberry::HopcroftKarp h(n, m, edges);
  assert(h.size() == expected);
  auto pairs = h.pairs(); assert(int(pairs.size()) == expected);
  std::vector<bool> usedl(n), usedr(m), present(n), coverl(n), coverr(m);
  for (auto [u, v] : pairs) {
    assert(0 <= u && u < n && 0 <= v && v < m && !usedl[u] && !usedr[v]);
    assert(h.left_match(u) == v && h.right_match(v) == u);
    usedl[u] = usedr[v] = true;
  }
  for (auto [u, v] : edges) if (h.left_match(u) == v) present[u] = true;
  for (auto [u, v] : pairs) { (void)v; assert(present[u]); }
  auto [left, right] = h.min_vertex_cover(); assert(int(left.size() + right.size()) == expected);
  for (int u : left) coverl[u] = true;
  for (int v : right) coverr[v] = true;
  for (auto [u, v] : edges) assert(coverl[u] || coverr[v]);
}

int main() {
  std::mt19937 rng(891247);
  for (int rep = 0; rep < 5000; ++rep) {
    int n = rng() % 9, m = rng() % 9; Edges edges;
    if (n && m) for (int j = rng() % 80; j > 0; --j) edges.emplace_back(rng() % n, rng() % m);
    check(n, m, edges, brute(n, m, edges));
  }
  const int n = 50000; Edges deep;
  for (int i = 0; i + 1 < n; ++i) { deep.emplace_back(i, i); deep.emplace_back(i, i + 1); }
  deep.emplace_back(n - 1, 0);
  check(n, n, deep, n);
  check(n, n, {}, 0);
}

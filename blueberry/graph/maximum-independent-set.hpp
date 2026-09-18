#pragma once
#include <bit>
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

namespace blueberry {
// Exact meet-in-the-middle algorithm for a simple undirected graph, n <= 40.
inline std::vector<int> maximum_independent_set(int n, const std::vector<std::pair<int, int>>& edges) {
  assert(0 <= n && n <= 40);
  std::vector<std::uint64_t> adj(n);
  for (auto [u, v] : edges) {
    assert(0 <= u && u < n && 0 <= v && v < n && u != v);
    adj[u] |= std::uint64_t{1} << v;
    adj[v] |= std::uint64_t{1} << u;
  }
  const int a = n / 2, b = n - a;
  const unsigned left_size = 1U << a, right_size = 1U << b;
  std::vector<unsigned> best(right_size);
  for (unsigned s = 1; s < right_size; ++s) {
    int v = std::countr_zero(s);
    unsigned rest = s & (s - 1);
    unsigned take = best[rest & ~static_cast<unsigned>(adj[a + v] >> a)] | (1U << v);
    best[s] = std::popcount(take) > std::popcount(best[rest]) ? take : best[rest];
  }
  std::vector<unsigned> allowed(left_size, right_size - 1);
  std::vector<bool> independent(left_size, true);
  std::uint64_t answer = std::uint64_t{best.back()} << a;
  for (unsigned s = 1; s < left_size; ++s) {
    int v = std::countr_zero(s);
    unsigned rest = s & (s - 1);
    independent[s] = independent[rest] && !(adj[v] & rest);
    allowed[s] = allowed[rest] & ~static_cast<unsigned>(adj[v] >> a);
    if (!independent[s]) continue;
    std::uint64_t candidate = s | (std::uint64_t{best[allowed[s]]} << a);
    if (std::popcount(candidate) > std::popcount(answer)) answer = candidate;
  }
  std::vector<int> result;
  for (int v = 0; v < n; ++v) if ((answer >> v) & 1) result.push_back(v);
  return result;
}
}  // namespace blueberry

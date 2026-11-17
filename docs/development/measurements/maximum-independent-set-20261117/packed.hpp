#pragma once
#include <bit>
#include <cstdint>
#include <utility>
#include <vector>
namespace blueberry {
inline std::vector<int> maximum_independent_set(int n, const std::vector<std::pair<int, int>>& edges) {
  std::vector<std::uint64_t> adj(n);
  for (auto [u, v] : edges) { adj[u] |= std::uint64_t{1} << v; adj[v] |= std::uint64_t{1} << u; }
  const int a = n / 2, b = n - a;
  const unsigned left_size = 1U << a, right_size = 1U << b, mask = right_size - 1, unit = 1U << b;
  std::vector<unsigned> best(right_size);
  for (unsigned s = 1; s < right_size; ++s) {
    int v = std::countr_zero(s);
    unsigned rest = s & (s - 1);
    unsigned take = best[rest & ~static_cast<unsigned>(adj[a + v] >> a)] + unit + (1U << v);
    best[s] = (take >> b) > (best[rest] >> b) ? take : best[rest];
  }
  std::vector<unsigned> allowed(left_size, mask);
  std::uint64_t answer = std::uint64_t{best.back() & mask} << a;
  unsigned answer_size = best.back() >> b;
  for (unsigned s = 1; s < left_size; ++s) {
    int v = std::countr_zero(s);
    unsigned rest = s & (s - 1), state = allowed[rest];
    if ((state & (1U << 31)) || (adj[v] & rest)) { allowed[s] = 1U << 31; continue; }
    state = ((state & mask) & ~static_cast<unsigned>(adj[v] >> a)) | ((state & ~mask) + unit);
    allowed[s] = state;
    unsigned right = best[state & mask];
    unsigned size = (state >> b) + (right >> b);
    if (size > answer_size) { answer_size = size; answer = s | (std::uint64_t{right & mask} << a); }
  }
  std::vector<int> result;
  for (int v = 0; v < n; ++v) if ((answer >> v) & 1) result.push_back(v);
  return result;
}
}

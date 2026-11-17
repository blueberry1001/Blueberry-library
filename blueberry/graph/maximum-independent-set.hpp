#pragma once
#include <bit>
#include <cstdint>
#include <utility>
#include <vector>
namespace blueberry {
inline std::vector<int> maximum_independent_set(int n, const std::vector<std::pair<int, int>>& edges) {
  std::vector<std::uint64_t> adj(n);
  for (auto [u, v] : edges) { adj[u] |= std::uint64_t{1} << v; adj[v] |= std::uint64_t{1} << u; }
  std::uint64_t branch_answer = 0;
  int branch_size = 0;
  unsigned budget = 1U << ((n + 1) / 2);
  if (budget > (1U << 18)) budget = 1U << 18;
  auto search = [&](auto&& self, std::uint64_t remaining, std::uint64_t chosen, int count) -> bool {
    if (budget == 0) return false;
    --budget;
    if (count + std::popcount(remaining) <= branch_size) return true;
    int vertex = -1, degree = -1;
    for (std::uint64_t bits = remaining; bits; bits &= bits - 1) {
      if (budget == 0) return false;
      --budget;
      int v = std::countr_zero(bits), d = std::popcount(adj[v] & remaining);
      if (d > degree) { vertex = v; degree = d; }
    }
    if (degree <= 2) {
      while (remaining) {
        int v = -1;
        for (std::uint64_t bits = remaining; bits; bits &= bits - 1) {
          if (budget == 0) return false;
          --budget;
          int candidate = std::countr_zero(bits);
          if (std::popcount(adj[candidate] & remaining) <= 1) { v = candidate; break; }
        }
        if (v == -1) v = std::countr_zero(remaining);
        chosen |= std::uint64_t{1} << v;
        remaining &= ~((std::uint64_t{1} << v) | adj[v]);
        ++count;
      }
      if (count > branch_size) { branch_size = count; branch_answer = chosen; }
      return true;
    }
    const auto bit = std::uint64_t{1} << vertex;
    return self(self, remaining & ~(bit | adj[vertex]), chosen | bit, count + 1) &&
           self(self, remaining & ~bit, chosen, count);
  };
  std::uint64_t remaining = (std::uint64_t{1} << n) - 1, branch_result = 0;
  bool complete = true;
  while (remaining) {
    std::uint64_t component = 0, frontier = std::uint64_t{1} << std::countr_zero(remaining);
    while (frontier) {
      const int v = std::countr_zero(frontier);
      const auto bit = std::uint64_t{1} << v;
      frontier &= ~bit;
      remaining &= ~bit;
      component |= bit;
      frontier |= adj[v] & remaining;
    }
    branch_answer = 0;
    branch_size = 0;
    if (!search(search, component, 0, 0)) { complete = false; break; }
    branch_result |= branch_answer;
  }
  if (complete) {
    std::vector<int> result;
    for (int v = 0; v < n; ++v) if ((branch_result >> v) & 1) result.push_back(v);
    return result;
  }
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

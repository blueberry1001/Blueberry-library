#pragma once
#include <algorithm>
#include <cassert>
#include <limits>
#include <utility>
#include <vector>

namespace blueberry {
// Emit every nonempty clique once; the callback's vector is ephemeral.
template<class Callback>
void enumerate_cliques(int n, const std::vector<std::pair<int, int>>& edges, Callback&& emit) {
  assert(n >= 0 && edges.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
  std::vector<std::vector<int>> adj(n), out(n);
  for (auto [u, v] : edges) {
    assert(0 <= u && u < n && 0 <= v && v < n && u != v);
    adj[u].push_back(v); adj[v].push_back(u);
  }
  for (auto& list : adj) std::sort(list.begin(), list.end());
  for (auto [u, v] : edges) {
    if (std::pair{adj[u].size(), u} > std::pair{adj[v].size(), v}) std::swap(u, v);
    out[u].push_back(v);
  }
  std::vector<int> clique;
  auto visit = [&](auto&& self, const std::vector<int>& candidates) -> void {
    emit(static_cast<const std::vector<int>&>(clique));
    for (std::size_t i = 0; i < candidates.size(); ++i) {
      int v = candidates[i];
      std::vector<int> next;
      for (std::size_t j = i + 1; j < candidates.size(); ++j)
        if (std::binary_search(adj[v].begin(), adj[v].end(), candidates[j])) next.push_back(candidates[j]);
      clique.push_back(v);
      self(self, next);
      clique.pop_back();
    }
  };
  for (int v = 0; v < n; ++v) {
    clique.push_back(v);
    visit(visit, out[v]);
    clique.pop_back();
  }
}
}  // namespace blueberry

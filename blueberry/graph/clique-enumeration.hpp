#pragma once
#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

namespace blueberry {
template<class Callback>
void enumerate_cliques(int n, const std::vector<std::pair<int, int>>& edges, Callback&& emit) {
  std::vector<std::vector<int>> adj(n), out(n);
  for (auto [u, v] : edges) {
    adj[u].push_back(v); adj[v].push_back(u);
  }
  for (auto& list : adj) std::sort(list.begin(), list.end());
  for (auto [u, v] : edges) {
    if (std::pair{adj[u].size(), u} > std::pair{adj[v].size(), v}) std::swap(u, v);
    out[u].push_back(v);
  }
  std::size_t depth = 0;
  for (const auto& list : out) depth = std::max(depth, list.size());
  std::vector<int> clique;
  if (depth <= 1) {
    for (int v = 0; v < n; ++v) {
      clique.push_back(v); emit(static_cast<const std::vector<int>&>(clique));
      if (!out[v].empty()) { clique.push_back(out[v][0]); emit(static_cast<const std::vector<int>&>(clique)); clique.pop_back(); }
      clique.pop_back();
    }
    return;
  }
  std::vector<std::vector<int>> scratch(depth);
  auto visit = [&](auto&& self, const std::vector<int>& candidates) -> void {
    emit(static_cast<const std::vector<int>&>(clique));
    for (std::size_t i = 0; i < candidates.size(); ++i) {
      int v = candidates[i];
      auto& next = scratch[clique.size() - 1];
      next.clear();
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
}

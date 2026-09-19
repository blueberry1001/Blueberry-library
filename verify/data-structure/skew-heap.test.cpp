#define PROBLEM "https://judge.yosupo.jp/problem/shortest_path"
#include <algorithm>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>
#include "blueberry/data-structure/skew-heap.hpp"
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n, m, s, t; std::cin >> n >> m >> s >> t;
  std::vector<std::vector<std::pair<int, int>>> graph(n);
  while (m--) { int u, v, w; std::cin >> u >> v >> w; graph[u].emplace_back(v, w); }
  constexpr long long inf = std::numeric_limits<long long>::max() / 4;
  std::vector<long long> distance(n, inf); std::vector<int> previous(n, -1);
  blueberry::SkewHeap<std::pair<long long, int>> queue;
  distance[s] = 0; queue.push({0, s});
  while (!queue.empty()) {
    auto [d, u] = queue.top(); queue.pop();
    if (d != distance[u]) continue;
    // Batch relaxations exercise meld between independently owned nonempty heaps.
    blueberry::SkewHeap<std::pair<long long, int>> batch;
    for (auto [v, w] : graph[u]) if (d + w < distance[v]) {
      distance[v] = d + w; previous[v] = u; batch.push({distance[v], v});
    }
    queue.meld(batch);
  }
  if (distance[t] == inf) { std::cout << -1 << '\n'; return 0; }
  std::vector<std::pair<int, int>> path;
  for (int v = t; v != s; v = previous[v]) path.emplace_back(previous[v], v);
  std::reverse(path.begin(), path.end());
  std::cout << distance[t] << ' ' << path.size() << '\n';
  for (auto [u, v] : path) std::cout << u << ' ' << v << '\n';
}

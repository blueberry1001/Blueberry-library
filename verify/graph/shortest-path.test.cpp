#define PROBLEM "https://judge.yosupo.jp/problem/shortest_path"

#include <iostream>

#include "blueberry/graph/dijkstra.hpp"

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  int n, m, source, target;
  std::cin >> n >> m >> source >> target;
  blueberry::WeightedGraph<long long> graph(n);
  for (int i = 0; i < m; ++i) {
    int from, to;
    long long cost;
    std::cin >> from >> to >> cost;
    graph[from].push_back({to, cost});
  }

  constexpr long long kInfinity = 1LL << 62;
  const auto result = blueberry::dijkstra(graph, source, kInfinity);
  const auto path = result.path_to(target);
  if (path.empty()) {
    std::cout << -1 << '\n';
    return 0;
  }

  std::cout << result.distance[target] << ' ' << path.size() - 1 << '\n';
  for (int i = 1; i < static_cast<int>(path.size()); ++i) {
    std::cout << path[i - 1] << ' ' << path[i] << '\n';
  }
}

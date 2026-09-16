#include "blueberry/graph/dijkstra.hpp"

#include <cstdlib>
#include <iostream>
#include <random>

// Catches overflowed relaxations, incorrect clipping and invalid restored paths.
int main(int argc, char** argv) {
  const auto seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  std::mt19937_64 rng(seed);
  const auto high = std::numeric_limits<long long>::max();
  blueberry::WeightedGraph<long long> boundary(3);
  boundary[0] = {{1, high - 2}, {2, 8}};
  boundary[1] = {{2, 5}};
  assert(blueberry::dijkstra(boundary, 0).distance[2] == 8);
  blueberry::WeightedGraph<unsigned long long> unsigned_graph(3);
  const auto umax = std::numeric_limits<unsigned long long>::max();
  unsigned_graph[0] = {{1, umax - 2}, {2, 8}};
  unsigned_graph[1] = {{2, 5}};
  assert(blueberry::dijkstra(unsigned_graph, 0).distance[2] == 8);
  assert(blueberry::dijkstra(boundary, 0, 8LL).path_to(2).empty());
  assert(blueberry::dijkstra(boundary, 0, 9LL).distance[2] == 8);
  blueberry::WeightedGraph<double> real(2);
  real[0] = {{1, 1.5}};
  assert(blueberry::dijkstra(real, 0).distance[1] == 1.5);
  for (int trial = 0; trial < 150; ++trial) {
    const int n = 1 + static_cast<int>(rng() % 18);
    blueberry::WeightedGraph<long long> g(n);
    std::vector<std::vector<long long>> dist(n, std::vector<long long>(n, 1000000));
    for (int i = 0; i < n; ++i) dist[i][i] = 0;
    for (int e = 0; e < 4 * n; ++e) {
      int u = rng() % n, v = rng() % n;
      long long w = rng() % 25;
      g[u].push_back({v, w});
      dist[u][v] = std::min(dist[u][v], w);
    }
    for (int k = 0; k < n; ++k)
      for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
          dist[i][j] = std::min(dist[i][j], dist[i][k] + dist[k][j]);
    for (int source = 0; source < n; ++source) {
      const long long inf = trial % 2 ? 30 : 1000000;
      auto result = blueberry::dijkstra(g, source, inf);
      for (int v = 0; v < n; ++v) {
        const auto expected = std::min(dist[source][v], inf);
        if (result.distance[v] != expected) {
          std::cerr << "seed=" << seed << " trial=" << trial << " source=" << source
                    << " target=" << v << " expected=" << expected
                    << " actual=" << result.distance[v] << '\n';
          for (int u = 0; u < n; ++u) for (auto e : g[u])
            std::cerr << u << ' ' << e.to << ' ' << e.cost << '\n';
          return 1;
        }
        auto path = result.path_to(v);
        if (expected == inf) { assert(path.empty()); continue; }
        assert(path.front() == source && path.back() == v);
        assert(path.size() <= static_cast<std::size_t>(n));
        long long length = 0;
        for (std::size_t i = 1; i < path.size(); ++i) {
          long long best = 1000000;
          for (auto e : g[path[i - 1]]) if (e.to == path[i]) best = std::min(best, e.cost);
          length += best;
        }
        assert(length == expected);
      }
    }
  }
}

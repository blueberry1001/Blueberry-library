#include "blueberry/graph/dijkstra.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <limits>
#include <queue>
#include <random>
#include <utility>
#include <vector>

struct CustomCost {
  long long value = 0;
  friend bool operator<(CustomCost a, CustomCost b) { return a.value < b.value; }
  friend bool operator>(CustomCost a, CustomCost b) { return a.value > b.value; }
  friend bool operator==(CustomCost a, CustomCost b) { return a.value == b.value; }
  friend CustomCost operator+(CustomCost a, CustomCost b) { return {a.value + b.value}; }
};

static auto reference_dijkstra(const blueberry::WeightedGraph<long long>& g, int source, long long inf) {
  std::vector<long long> dist(g.size(), inf);
  std::vector<int> parent(g.size(), -1);
  using Item = std::pair<long long, int>;
  std::priority_queue<Item, std::vector<Item>, std::greater<Item>> pending;
  dist[source] = 0; pending.emplace(0, source);
  while (!pending.empty()) {
    auto [d, u] = pending.top(); pending.pop();
    if (d != dist[u]) continue;
    for (auto e : g[u]) if (d + e.cost < dist[e.to]) {
      dist[e.to] = d + e.cost;
      parent[e.to] = u;
      pending.emplace(dist[e.to], e.to);
    }
  }
  return std::pair{dist, parent};
}

template<class Cost>
static void test_radix_boundaries() {
  const Cost inf = std::numeric_limits<Cost>::max();
  blueberry::WeightedGraph<Cost> g(257);
  g[0] = {{1, Cost(inf - 4)}, {2, Cost(inf - 1)}, {5, Cost(0)}, {7, Cost(inf - 2)}};
  g[1] = {{2, Cost(3)}, {3, Cost(3)}, {4, Cost(4)}};
  g[5] = {{6, Cost(0)}, {7, Cost(1)}};
  auto result = blueberry::dijkstra(g, 0);
  assert(result.distance[1] == Cost(inf - 4));
  assert(result.distance[2] == Cost(inf - 1) && result.parent[2] == 0);
  assert(result.distance[3] == Cost(inf - 1) && result.parent[3] == 1);
  assert(result.distance[4] == inf && result.path_to(4).empty());
  assert(result.distance[6] == 0 && result.distance[7] == 1);
  for (int v : {0, 2, 3, 4, 6, 7}) {
    auto single = blueberry::dijkstra_path(g, 0, v);
    assert(single.first == result.distance[v] && single.second == result.path_to(v));
  }
}

int main(int argc, char** argv) {
  const auto seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  std::mt19937_64 rng(seed);
  test_radix_boundaries<std::uint8_t>();
  test_radix_boundaries<std::uint64_t>();
  test_radix_boundaries<std::int64_t>();
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
  assert((blueberry::dijkstra_path(boundary, 0, 0) ==
          std::pair<long long, std::vector<int>>{0, {0}}));
  assert((blueberry::dijkstra_path(boundary, 0, 2, 8LL) ==
          std::pair<long long, std::vector<int>>{8, {}}));
  assert((blueberry::dijkstra_path(boundary, 0, 2, 9LL) ==
          std::pair<long long, std::vector<int>>{8, {0, 2}}));
  assert(blueberry::dijkstra_path(unsigned_graph, 0, 2).first == 8);
  blueberry::WeightedGraph<double> real(2);
  real[0] = {{1, 1.5}};
  assert(blueberry::dijkstra(real, 0).distance[1] == 1.5);
  blueberry::WeightedGraph<CustomCost> custom(3);
  custom[0] = {{1, {7}}, {2, {15}}};
  custom[1] = {{2, {3}}};
  assert(blueberry::dijkstra(custom, 0, CustomCost{100}).distance[2].value == 10);
  assert(blueberry::dijkstra_path(custom, 0, 2, CustomCost{100}).first.value == 10);
  blueberry::WeightedGraph<long long> tied(8);
  tied[0] = {{6, 0}, {2, 0}, {1, 0}};
  tied[1] = {{4, 0}, {3, 0}};
  tied[2] = {{3, 0}, {7, 1}};
  tied[3] = {{6, 0}, {7, 1}};
  tied[4] = {{7, 1}};
  auto tied_result = blueberry::dijkstra(tied, 0);
  assert(tied_result.parent[3] == 1 && tied_result.parent[7] == 2);
  blueberry::WeightedGraph<bool> bit_graph(3);
  bit_graph[0].push_back({1, false});
  bit_graph[0].push_back({2, true});
  assert(blueberry::dijkstra_path(bit_graph, 0, 1, true).first == false);
  assert(blueberry::dijkstra_path(bit_graph, 0, 2, true).second.empty());
#if defined(__SIZEOF_INT128__)
  using Wide = __int128_t;
  const Wide magnitude = Wide{1} << 80;
  blueberry::WeightedGraph<Wide> wide(3);
  wide[0] = {{1, magnitude}, {2, magnitude + 7}};
  wide[1] = {{2, 3}};
  assert(blueberry::dijkstra_path(wide, 0, 2, magnitude + 100).first == magnitude + 3);
#endif
  for (int n : {128, 129, 256, 257}) {
    blueberry::WeightedGraph<long long> large(n);
    for (int i = 3; i < n; ++i) large[0].push_back({i, 100 + i});
    large[0].push_back({5, 0}); large[0].push_back({2, 0});
    large[2].push_back({100, 0}); large[5].push_back({100, 0});
    auto result = blueberry::dijkstra(large, 0);
    assert(result.parent[100] == 2);
    auto single = blueberry::dijkstra_path(large, 0, 100);
    assert(single.first == 0 && single.second == result.path_to(100));
  }
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
      auto reference = reference_dijkstra(g, source, inf);
      assert(result.distance == reference.first && result.parent == reference.second);
      for (int target : {source, static_cast<int>(rng() % n), static_cast<int>(rng() % n)}) {
        auto single = blueberry::dijkstra_path(g, source, target, inf);
        assert(single.first == result.distance[target] && single.second == result.path_to(target));
      }
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
  for (int n : {129, 256, 257}) for (int trial = 0; trial < 20; ++trial) {
    blueberry::WeightedGraph<long long> graph(n);
    for (int e = 0; e < n * 6; ++e)
      graph[rng() % n].push_back({static_cast<int>(rng() % n), static_cast<long long>(rng() % 32)});
    auto expected = reference_dijkstra(graph, 0, 1000000);
    auto actual = blueberry::dijkstra(graph, 0, 1000000LL);
    assert(actual.distance == expected.first && actual.parent == expected.second);
    for (int i = 0; i < 4; ++i) {
      int target = rng() % n;
      auto single = blueberry::dijkstra_path(graph, 0, target, 1000000LL);
      assert(single.first == actual.distance[target] && single.second == actual.path_to(target));
    }
  }
}

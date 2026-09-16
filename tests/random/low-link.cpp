#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

#include "blueberry/graph/low-link.hpp"

using Edges = std::vector<std::pair<int, int>>;

// Independent oracle: flood the graph after deleting one vertex or edge.
std::vector<int> components(int n, const Edges& edges, int removed_vertex,
                            int removed_edge, const std::vector<bool>& bridges = {}) {
  std::vector<std::vector<int>> graph(n);
  for (int i = 0; i < static_cast<int>(edges.size()); ++i) {
    const auto [u, v] = edges[i];
    if (u == removed_vertex || v == removed_vertex || i == removed_edge ||
        (!bridges.empty() && bridges[i])) continue;
    graph[u].push_back(v);
    graph[v].push_back(u);
  }
  std::vector<int> label(n, -1);
  int next = 0;
  for (int root = 0; root < n; ++root) {
    if (root == removed_vertex || label[root] != -1) continue;
    std::vector<int> queue{root};
    label[root] = next++;
    for (int k = 0; k < static_cast<int>(queue.size()); ++k) {
      for (int to : graph[queue[k]]) {
        if (label[to] != -1) continue;
        label[to] = label[root];
        queue.push_back(to);
      }
    }
  }
  return label;
}

int count(const std::vector<int>& label) {
  return label.empty() ? 0 : *std::max_element(label.begin(), label.end()) + 1;
}

void check(int n, const Edges& edges, unsigned long long seed) {
  const blueberry::LowLink graph(n, edges);
  auto require = [&](bool condition, const char* operation, int id, int expected, int actual) {
    if (condition) return;
    std::cerr << "seed=" << seed << " n=" << n << " edges=";
    for (const auto& [u, v] : edges) std::cerr << '(' << u << ',' << v << ')';
    std::cerr << " operation=" << operation << " id=" << id
              << " expected=" << expected << " actual=" << actual << '\n';
    std::exit(1);
  };
  require(graph.size() == n, "size", 0, n, graph.size());
  const int baseline = count(components(n, edges, -1, -1));
  std::vector<bool> bridges(edges.size());
  std::vector<int> expected_bridges, expected_articulation;
  for (int i = 0; i < static_cast<int>(edges.size()); ++i) {
    bridges[i] = count(components(n, edges, -1, i)) > baseline;
    require(graph.is_bridge(i) == bridges[i], "bridge", i, bridges[i], graph.is_bridge(i));
    if (bridges[i]) expected_bridges.push_back(i);
  }
  for (int v = 0; v < n; ++v) {
    const bool expected = count(components(n, edges, v, -1)) > baseline;
    require(graph.is_articulation(v) == expected, "articulation", v, expected,
            graph.is_articulation(v));
    if (expected) expected_articulation.push_back(v);
  }
  require(graph.bridges() == expected_bridges, "bridge-list", 0, 1, 0);
  require(graph.articulation_points() == expected_articulation, "articulation-list", 0, 1, 0);
  const auto expected = components(n, edges, -1, -1, bridges);
  const auto groups = graph.groups();
  std::vector<int> seen(n);
  for (int i = 0; i < static_cast<int>(groups.size()); ++i) {
    for (int v : groups[i]) {
      require(v >= 0 && v < n, "group vertex", i, 1, 0);
      ++seen[v];
      require(graph.component(v) == i, "component", v, i, graph.component(v));
    }
  }
  for (int u = 0; u < n; ++u) {
    require(seen[u] == 1, "partition", u, 1, seen[u]);
    for (int v = 0; v < n; ++v) {
      const bool same = expected[u] == expected[v];
      require((graph.component(u) == graph.component(v)) == same, "same component", u,
              same, graph.component(u) == graph.component(v));
    }
  }
}

int main(int argc, char** argv) {
  const auto seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  std::mt19937_64 random(seed);
  check(0, {}, seed);
  check(1, {{0, 0}}, seed);
  check(4, {{0, 1}, {0, 1}, {1, 2}, {2, 2}}, seed);
  check(6, {{0, 1}, {1, 2}, {2, 0}, {3, 4}}, seed);
  for (int trial = 0; trial < 350; ++trial) {
    const int n = 1 + random() % 9;
    const int m = random() % 23;
    Edges edges;
    for (int i = 0; i < m; ++i) edges.emplace_back(random() % n, random() % n);
    check(n, edges, seed);
  }
  const int n = 200000;
  Edges chain;
  for (int v = 1; v < n; ++v) chain.emplace_back(v - 1, v);
  const blueberry::LowLink graph(n, chain);
  assert(static_cast<int>(graph.bridges().size()) == n - 1);
  assert(static_cast<int>(graph.articulation_points().size()) == n - 2);
  assert(static_cast<int>(graph.groups().size()) == n);
}

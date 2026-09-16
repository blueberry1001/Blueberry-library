#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "blueberry/graph/rerooting.hpp"

using Edges = std::vector<std::pair<int, int>>;

template <class T, class Merge, class Vertex, class Transfer>
void check(int n, const Edges& edges, T identity, Merge merge, Vertex vertex,
           Transfer transfer, unsigned long long seed) {
  const auto actual = blueberry::rerooting(n, edges, identity, merge, vertex, transfer);
  assert(static_cast<int>(actual.size()) == n);
  std::vector<std::vector<std::pair<int, int>>> graph(n);
  for (int i = 0; i < static_cast<int>(edges.size()); ++i) {
    const auto [u, v] = edges[i];
    graph[u].emplace_back(v, i);
    graph[v].emplace_back(u, i);
  }
  // Small recursive oracle separately roots the whole tree at every vertex.
  auto brute = [&](auto&& self, int v, int parent) -> T {
    T value = identity;
    for (const auto& [to, id] : graph[v]) {
      if (to == parent) continue;
      value = merge(value, transfer(self(self, to, v), id, to, v));
    }
    return vertex(value, v);
  };
  for (int root = 0; root < n; ++root) {
    const auto expected = brute(brute, root, -1);
    if (actual[root] == expected) continue;
    std::cerr << "seed=" << seed << " n=" << n << " root=" << root << " edges=";
    for (const auto& [u, v] : edges) std::cerr << '(' << u << ',' << v << ')';
    std::cerr << " expected=" << expected << " actual=" << actual[root] << '\n';
    std::exit(1);
  }
}

int main(int argc, char** argv) {
  const auto seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  std::mt19937_64 random(seed);
  for (int trial = 0; trial < 250; ++trial) {
    const int n = trial < 2 ? trial : 2 + random() % 22;
    Edges edges;
    for (int v = 1; v < n; ++v) edges.emplace_back(random() % v, v);
    std::shuffle(edges.begin(), edges.end(), random);
    for (auto& [u, v] : edges) if (random() & 1) std::swap(u, v);
    // Noncommutative concatenation exposes reordered parent/child contributions.
    check(n, edges, std::string{},
          [](const std::string& a, const std::string& b) { return a + b; },
          [](const std::string& value, int v) { return "(" + std::to_string(v) + value + ")"; },
          [](const std::string& value, int id, int from, int to) {
            return "[" + std::to_string(id) + ":" + std::to_string(from) + ">" +
                   std::to_string(to) + value + "]";
          }, seed);
    // Longest weighted distance: asymmetric transfer exercises both directions.
    check(n, edges, 0LL,
          [](long long a, long long b) { return std::max(a, b); },
          [](long long value, int) { return value; },
          [](long long value, int id, int from, int to) {
            return value + 1 + id + (from < to ? 5 : 0);
          }, seed);
  }
  for (bool star : {false, true}) {
    const int n = 200000;
    Edges edges;
    for (int v = 1; v < n; ++v) edges.emplace_back(star ? 0 : v - 1, v);
    auto result = blueberry::rerooting(n, edges, 0,
        [](int a, int b) { return std::max(a, b); },
        [](int value, int) { return value; },
        [](int value, int, int, int) { return value + 1; });
    for (int v = 0; v < n; ++v) {
      assert(result[v] == (star ? (v == 0 ? 1 : 2) : std::max(v, n - 1 - v)));
    }
  }
}

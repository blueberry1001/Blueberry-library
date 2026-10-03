#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>
#include "blueberry/graph/dominator-tree.hpp"

using Graph = std::vector<std::vector<int>>;

std::vector<bool> reachable(const Graph& graph, int root, int removed) {
  std::vector<bool> seen(graph.size());
  if (root == removed) return seen;
  std::vector<int> queue{root};
  seen[root] = true;
  for (std::size_t i = 0; i < queue.size(); ++i)
    for (int to : graph[queue[i]]) if (to != removed && !seen[to]) {
      seen[to] = true;
      queue.push_back(to);
    }
  return seen;
}

// Removing d disconnects v exactly when d dominates reachable v.
std::vector<int> brute(const Graph& graph, int root) {
  const int n = static_cast<int>(graph.size());
  const auto seen = reachable(graph, root, -1);
  std::vector<std::vector<bool>> dominates(n, std::vector<bool>(n));
  for (int d = 0; d < n; ++d) {
    const auto without = reachable(graph, root, d);
    for (int v = 0; v < n; ++v) dominates[d][v] = seen[v] && !without[v];
  }
  std::vector<int> result(n, -1);
  result[root] = root;
  for (int v = 0; v < n; ++v) if (v != root && seen[v]) {
    for (int d = 0; d < n; ++d) if (d != v && dominates[d][v]) {
      bool immediate = true;
      for (int other = 0; other < n; ++other)
        if (other != v && dominates[other][v] && !dominates[other][d]) immediate = false;
      if (immediate) { assert(result[v] == -1); result[v] = d; }
    }
    assert(result[v] != -1);
  }
  return result;
}

void check(const Graph& graph, int root, unsigned seed) {
  const auto expected = brute(graph, root);
  const auto before = graph;
  const auto actual = blueberry::dominator_tree(graph, root);
  if (expected != actual || graph != before) {
    std::cerr << "dominator-tree seed=" << seed << " root=" << root << '\n';
    std::cerr << "n=" << graph.size() << " edges:\n";
    for (std::size_t u = 0; u < graph.size(); ++u)
      for (int v : graph[u]) std::cerr << u << ' ' << v << '\n';
    std::cerr << "expected:";
    for (int v : expected) std::cerr << ' ' << v;
    std::cerr << "\nactual:";
    for (int v : actual) std::cerr << ' ' << v;
    std::cerr << '\n';
    std::abort();
  }
}

int main(int argc, char** argv) {
  const unsigned seed = argc > 1 ? static_cast<unsigned>(std::strtoul(argv[1], nullptr, 10)) : 20261003;
  std::mt19937 rng(seed);
  assert(blueberry::dominator_tree({}, -1).empty());
  check({{}}, 0, seed);
  check({{0, 0, 0}}, 0, seed);
  check({{1, 2}, {3}, {3}, {4}, {}, {3, 4}}, 0, seed);
  check({{2}, {0}, {3}, {2}, {1, 3}}, 1, seed);
  check(Graph(8), 5, seed);

  // Exhaustive loop-free directed graphs with up to four vertices, every root.
  for (int n = 1; n <= 4; ++n) {
    const unsigned count = 1U << (n * (n - 1));
    for (unsigned mask = 0; mask < count; ++mask) {
      Graph graph(n);
      int bit = 0;
      for (int u = 0; u < n; ++u) for (int v = 0; v < n; ++v) if (u != v) {
        if ((mask >> bit) & 1U) graph[u].push_back(v);
        ++bit;
      }
      for (int root = 0; root < n; ++root) check(graph, root, seed);
    }
  }

  for (int trial = 0; trial < 500; ++trial) {
    const int n = 1 + static_cast<int>(rng() % 12);
    Graph graph(n);
    const int m = static_cast<int>(rng() % (3 * n * n + 1));
    for (int i = 0; i < m; ++i) graph[rng() % n].push_back(static_cast<int>(rng() % n));
    for (int root = 0; root < n; ++root) check(graph, root, seed);
    for (auto& row : graph) std::shuffle(row.begin(), row.end(), rng);
    check(graph, static_cast<int>(rng() % n), seed);
  }

  // Exercise both deep DFS and a long union-find compression path.
  const int n = 200000;
  Graph chain(n);
  for (int v = 1; v < n; ++v) chain[v - 1].push_back(v);
  auto idom = blueberry::dominator_tree(chain, 0);
  assert(idom[0] == 0);
  for (int v = 1; v < n; ++v) assert(idom[v] == v - 1);
  chain.back().push_back(1);
  idom = blueberry::dominator_tree(chain, 0);
  for (int v = 1; v < n; ++v) assert(idom[v] == v - 1);
  chain[0].push_back(n - 1);
  idom = blueberry::dominator_tree(chain, 0);
  assert(idom[0] == 0 && idom[n - 1] == 0);
  for (int v = 1; v + 1 < n; ++v) assert(idom[v] == v - 1);
  idom = blueberry::dominator_tree(chain, n / 2);
  for (int v = 0; v < n; ++v) {
    const int expected = v == 0 ? -1 : v == n / 2 ? v : v == 1 ? n - 1 : v - 1;
    assert(idom[v] == expected);
  }
  std::cerr << "dominator-tree seed=" << seed << " passed\n";
}

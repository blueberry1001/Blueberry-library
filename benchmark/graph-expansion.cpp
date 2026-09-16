// Compare our CSR layout with our own generated nested-adjacency variant,
// and dedicated bipartite matching with ACL's generic residual flow graph.
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <vector>
#include <atcoder/maxflow>
#include "blueberry/graph/biconnected-components.hpp"
#include "blueberry/graph/eulerian-trail.hpp"
#include "blueberry/graph/hopcroft-karp.hpp"
#include "nested-biconnected-components.hpp"
#include "nested-eulerian-trail.hpp"

using Edges = std::vector<std::pair<int, int>>;
std::uint64_t fingerprint(const std::vector<std::vector<int>>& groups) {
  std::uint64_t result = 0;
  for (const auto& group : groups) {
    std::uint64_t hash = 1;
    for (int v : group) hash = hash * 1000003 + v;
    result += hash;
  }
  return result;
}
template<class F>
std::pair<double, std::uint64_t> measure(F run) {
  const auto start = std::chrono::steady_clock::now();
  auto checksum = run();
  const auto end = std::chrono::steady_clock::now();
  return {std::chrono::duration<double>(end - start).count(), checksum};
}
template<class F, class G>
void compare(const std::string& shape, int n, const Edges& edges, const char* name_a, F a, const char* name_b, G b) {
  for (int run = 0; run < 6; ++run) {
    std::pair<double, std::uint64_t> x, y;
    if (run % 2) { x = measure(a); y = measure(b); }
    else { y = measure(b); x = measure(a); }
    if (x.second != y.second) { std::cerr << "checksum mismatch: " << shape << '\n'; std::exit(1); }
    if (!run) continue; // One untimed/report-excluded warmup per candidate.
    for (auto [name, value] : {std::pair{name_a, x}, std::pair{name_b, y}})
      std::cout << shape << ',' << n << ',' << edges.size() << ',' << name << ',' << run << ',' << value.first << ',' << value.second << '\n';
  }
}
void matching(const std::string& shape, int n, const Edges& edges) {
  compare(shape, n, edges, "hopcroft_karp", [&] { return blueberry::HopcroftKarp(n, n, edges).size(); },
    "acl_maxflow", [&] {
      atcoder::mf_graph<int> graph(2 * n + 2);
      for (int v = 0; v < n; ++v) { graph.add_edge(2 * n, v, 1); graph.add_edge(v + n, 2 * n + 1, 1); }
      for (auto [u, v] : edges) graph.add_edge(u, v + n, 1);
      return graph.flow(2 * n, 2 * n + 1);
    });
}
void blocks(const std::string& shape, int n, const Edges& edges) {
  compare(shape, n, edges, "csr_blocks", [&] { return fingerprint(blueberry::BiconnectedComponents(n, edges).groups()); },
    "nested_blocks", [&] { return fingerprint(blueberry::NestedBiconnectedComponents(n, edges).groups()); });
}
void euler(const std::string& shape, int n, const Edges& edges) {
  auto checksum = [](const auto& result) {
    std::uint64_t sum = result.exists();
    for (int v : result.vertices()) sum = sum * 1000003 + v;
    for (int e : result.edges()) sum = sum * 1000003 + e;
    return sum;
  };
  compare(shape, n, edges, "csr_euler", [&] { return checksum(blueberry::EulerianTrail<true>(n, edges)); },
    "nested_euler", [&] { return checksum(blueberry::NestedEulerianTrail<true>(n, edges)); });
}
int main() {
  std::mt19937 random(20260917);
  std::cout << "shape,n,m,candidate,run,seconds,checksum\n";
  Edges edges;
  for (int i = 0; i < 100000; ++i) edges.emplace_back(random() % 30000, random() % 30000);
  matching("matching_sparse", 30000, edges);
  edges.clear();
  for (int i = 0; i < 9999; ++i) { edges.emplace_back(i, i + 1); edges.emplace_back(i, i); }
  edges.emplace_back(9999, 9999);
  matching("matching_ladder", 10000, edges);
  edges.clear();
  for (int i = 1; i < 100000; ++i) edges.emplace_back(i - 1, i);
  blocks("blocks_path", 100000, edges);
  euler("euler_path", 100000, edges);
  for (int i = 0; i < 100000; ++i) {
    const int u = random() % 100000, v = random() % 100000;
    if (u != v) edges.emplace_back(u, v);
  }
  blocks("blocks_chords", 100000, edges);
  edges.clear();
  int vertex = 0;
  for (int i = 0; i < 200000; ++i) {
    int next = random() % 100000;
    edges.emplace_back(vertex, next); vertex = next;
  }
  edges.emplace_back(vertex, 0);
  euler("euler_walk", 100000, edges);
}

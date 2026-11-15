#include "blueberry/graph/dijkstra.hpp"
#include <chrono>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <utility>
#include <vector>

using Clock = std::chrono::steady_clock;
using Arc = blueberry::WeightedEdge<long long>;
struct Edge { int from, to; long long cost; };
struct Nested {
  blueberry::WeightedGraph<long long> edges;
  Nested(int n, const std::vector<Edge>& input) : edges(n) {
    for (auto e : input) edges[e.from].push_back({e.to, e.cost});
  }
  template<class F> void visit(int v, F f) const { for (auto e : edges[v]) f(e); }
};
struct CSR {
  std::vector<int> offsets;
  std::vector<Arc> edges;
  CSR(int n, const std::vector<Edge>& input) : offsets(n + 1), edges(input.size()) {
    for (auto e : input) ++offsets[e.from + 1];
    for (int i = 0; i < n; ++i) offsets[i + 1] += offsets[i];
    auto cursor = offsets;
    for (auto e : input) edges[cursor[e.from]++] = {e.to, e.cost};
  }
  template<class F> void visit(int v, F f) const { for (int i = offsets[v]; i < offsets[v + 1]; ++i) f(edges[i]); }
};
struct Result { std::vector<long long> dist; std::vector<int> parent; double build, search; };
template<class Graph> Result run(int n, const std::vector<Edge>& input) {
  auto start = Clock::now();
  Graph graph(n, input);
  auto built = Clock::now();
  std::vector<long long> dist(n, std::numeric_limits<long long>::max());
  std::vector<int> parent(n, -1);
  blueberry::detail::DijkstraQueue<long long, true> q;
  dist[0] = 0; q.emplace(0, 0);
  while (!q.empty()) {
    auto [d, v] = q.pop();
    if (d != dist[v]) continue;
    graph.visit(v, [&](auto e) {
      auto next = d + e.cost;
      if (next < dist[e.to]) { dist[e.to] = next; parent[e.to] = v; q.emplace(next, e.to); }
    });
  }
  auto end = Clock::now();
  return {std::move(dist), std::move(parent), std::chrono::duration<double, std::milli>(built - start).count(),
          std::chrono::duration<double, std::milli>(end - built).count()};
}
int main(int argc, char** argv) {
  std::string shape = argc > 1 ? argv[1] : "random";
  int n = 100000, m = 400000;
  std::mt19937_64 rng(20261115);
  std::vector<Edge> input;
  for (int i = 1; i < n; ++i) input.push_back({i - 1, i, shape == "zero" ? 0 : 1});
  if (shape != "line") for (int i = 0; i < m; ++i) {
    int from = int(rng() % n), to = int(rng() % n);
    if (shape == "dag" && from >= to) std::swap(from, to);
    if (shape == "dag" && from == to) continue;
    input.push_back({from, to, shape == "zero" ? 0 : static_cast<long long>(rng() % 1000000000)});
  }
  auto baseline = run<Nested>(n, input);
  auto candidate = run<CSR>(n, input);
  if (baseline.dist != candidate.dist || baseline.parent != candidate.parent) { std::cerr << "result mismatch\n"; return 1; }
  for (int round = 0; round < 8; ++round) for (int j = 0; j < 2; ++j) {
    bool csr = ((round + j) & 1) != 0;
    auto result = csr ? run<CSR>(n, input) : run<Nested>(n, input);
    if (baseline.dist != result.dist || baseline.parent != result.parent) { std::cerr << "result mismatch\n"; return 1; }
    std::cout << shape << ' ' << round << ' ' << csr << ' ' << input.size() << ' ' << result.build << ' ' << result.search << '\n';
  }
}

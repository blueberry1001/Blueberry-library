#include "blueberry/graph/dijkstra.hpp"
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>

using Clock = std::chrono::steady_clock;
using Graph = blueberry::WeightedGraph<long long>;
struct Edge { int from, to; long long cost; };
struct Result { std::vector<long long> distance; double setup, search; std::size_t pops, pushes, ready; };

Result run(const Graph& graph, bool fifo) {
  auto start = Clock::now();
  const int n = static_cast<int>(graph.size());
  std::vector<long long> dist(n, std::numeric_limits<long long>::max());
  std::vector<int> incoming(n), queue;
  std::vector<bool> done(n);
  if (fifo) {
    for (const auto& edges : graph) for (const auto& edge : edges) ++incoming[edge.to];
    queue.reserve(n);
  }
  blueberry::detail::DijkstraQueue<long long, true> heap;
  dist[0] = 0;
  if (fifo) queue.push_back(0); else heap.emplace(0, 0);
  std::size_t head = 0, pops = 0, pushes = fifo ? 0 : 1, ready = fifo ? 1 : 0;
  auto setup_done = Clock::now();
  while (head < queue.size() || !heap.empty()) {
    long long d;
    int v;
    if (head < queue.size()) { v = queue[head++]; d = dist[v]; }
    else { auto item = heap.pop(); d = item.first; v = item.second; ++pops; }
    if (done[v] || dist[v] != d) continue;
    done[v] = true;
    for (const auto& edge : graph[v]) {
      long long next = d + edge.cost;
      bool improved = next < dist[edge.to];
      if (improved) dist[edge.to] = next;
      if (fifo && --incoming[edge.to] == 0 && !done[edge.to]) { queue.push_back(edge.to); ++ready; }
      else if (improved && !done[edge.to]) { heap.emplace(next, edge.to); ++pushes; }
    }
  }
  auto end = Clock::now();
  return {std::move(dist), std::chrono::duration<double, std::milli>(setup_done - start).count(),
          std::chrono::duration<double, std::milli>(end - setup_done).count(), pops, pushes, ready};
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
  Graph graph(n);
  for (auto edge : input) graph[edge.from].push_back({edge.to, edge.cost});
  auto baseline = run(graph, false);
  auto candidate = run(graph, true);
  if (baseline.distance != candidate.distance) { std::cerr << "distance mismatch\n"; return 1; }
  for (int round = 0; round < 8; ++round) for (int j = 0; j < 2; ++j) {
    bool fifo = ((round + j) & 1) != 0;
    auto result = run(graph, fifo);
    if (baseline.distance != result.distance) { std::cerr << "distance mismatch\n"; return 1; }
    std::cout << shape << ' ' << round << ' ' << fifo << ' ' << result.setup << ' ' << result.search << ' '
              << result.pops << ' ' << result.pushes << ' ' << result.ready << '\n';
  }
}

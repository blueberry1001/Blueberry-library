// Run through dominator-tree-benchmark.py, which generates the comparison header.
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "blueberry/graph/dominator-tree.hpp"
#include "dominator-tree-vector.hpp"

using Graph = std::vector<std::vector<int>>;
using Clock = std::chrono::steady_clock;

Graph make_graph(const std::string& name) {
  std::mt19937 rng(20261003);
  int n = name == "dense" ? 20000 : name == "random" ? 100000 : 200000;
  Graph graph(n);
  if (name == "chain" || name == "backedge") {
    for (int v = 1; v < n; ++v) graph[v - 1].push_back(v);
    if (name == "backedge") graph.back().push_back(1);
  } else if (name == "unreachable") {
    for (int v = 1; v < 2000; ++v) graph[v - 1].push_back(v);
    for (int i = 1999; i < 200000; ++i)
      graph[2000 + rng() % (n - 2000)].push_back(static_cast<int>(rng() % n));
  } else {
    for (int v = 1; v < n; ++v) graph[rng() % v].push_back(v);
    for (int i = n - 1; i < 200000; ++i) graph[rng() % n].push_back(static_cast<int>(rng() % n));
  }
  return graph;
}

int main(int argc, char** argv) {
  const std::string mode = argc > 1 ? argv[1] : "compare";
  const int repeats = argc > 2 ? std::atoi(argv[2]) : 7;
  std::cout << std::fixed << std::setprecision(6);
  std::cout << "case,layout,run,n,m,input_build_ms,solve_ms,checksum\n";
  for (const std::string name : {"chain", "backedge", "random", "dense", "unreachable"}) {
    const auto begin = Clock::now();
    const auto graph = make_graph(name);
    const double build_ms = std::chrono::duration<double, std::milli>(Clock::now() - begin).count();
    std::size_t m = 0;
    for (const auto& row : graph) m += row.size();
    if (mode == "compare") {
      if (blueberry::dominator_tree(graph, 0) != blueberry::dominator_tree_vector(graph, 0)) std::abort();
      continue;
    }
    for (int run = -1; run < repeats; ++run) {
      const auto start = Clock::now();
      const auto result = mode == "flat" ? blueberry::dominator_tree(graph, 0)
                                         : blueberry::dominator_tree_vector(graph, 0);
      const double solve_ms = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
      std::uint64_t checksum = 0;
      for (int x : result) checksum = checksum * 1000003 + static_cast<unsigned>(x);
      if (run >= 0) std::cout << name << ',' << mode << ',' << run << ',' << graph.size() << ',' << m
                              << ',' << build_ms << ',' << solve_ms << ',' << checksum << '\n';
    }
  }
}

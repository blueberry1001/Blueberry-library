// Standalone baseline: run each algorithm/shape in a fresh process under
// /usr/bin/time -v to record peak RSS. Input generation is outside the timer.
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "blueberry/graph/low-link.hpp"
#include "blueberry/graph/rerooting.hpp"

int main(int argc, char** argv) {
  if (argc != 5) {
    std::cerr << "usage: graph-additions lowlink|rerooting path|star|random N repeats\n";
    return 1;
  }
  const std::string algorithm = argv[1], shape = argv[2];
  const int n = std::atoi(argv[3]), repeats = std::atoi(argv[4]);
  if (n < 1 || repeats < 1 || (algorithm != "lowlink" && algorithm != "rerooting") ||
      (shape != "path" && shape != "star" && shape != "random")) return 1;
  std::mt19937_64 random(20260916);
  std::vector<std::pair<int, int>> edges;
  edges.reserve(n - 1);
  for (int v = 1; v < n; ++v) {
    const int parent = shape == "path" ? v - 1 : shape == "star" ? 0 : random() % v;
    edges.emplace_back(parent, v);
  }
  std::vector<double> elapsed;
  long long checksum = 0;
  for (int run = 0; run < repeats; ++run) {
    const auto start = std::chrono::steady_clock::now();
    if (algorithm == "lowlink") {
      const blueberry::LowLink graph(n, edges);
      checksum += graph.bridges().size() + graph.articulation_points().size() + graph.component(n - 1);
    } else {
      const auto result = blueberry::rerooting(n, edges, 0,
          [](int a, int b) { return std::max(a, b); },
          [](int value, int) { return value; },
          [](int value, int, int, int) { return value + 1; });
      for (int value : result) checksum += value;
    }
    elapsed.push_back(std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count());
  }
  std::cout << "algorithm=" << algorithm << " shape=" << shape << " n=" << n
            << " m=" << edges.size() << " seed=20260916 repeats=" << repeats << " samples_seconds=";
  for (double seconds : elapsed) std::cout << seconds << ',';
  std::sort(elapsed.begin(), elapsed.end());
  std::cout << " median_seconds=" << elapsed[elapsed.size() / 2]
            << " min_seconds=" << elapsed.front() << " checksum=" << checksum << '\n';
}

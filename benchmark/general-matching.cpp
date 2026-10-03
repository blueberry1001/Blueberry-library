// Run through general-matching.py, which supplies the two generated candidate headers
// and the unmodified Library Checker reference from the local official-problem cache.
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <queue>
#include <random>
#include <string>
#include <utility>
#include <vector>
#include "blueberry/graph/general-matching.hpp"
#include GENERAL_MATCHING_NO_GREEDY
#include GENERAL_MATCHING_NO_EARLY_STOP

namespace official_reference {
#define main reference_main
#include GENERAL_MATCHING_REFERENCE
#undef main
}  // namespace official_reference

using Edges = std::vector<std::pair<int, int>>;
using Clock = std::chrono::steady_clock;
using Solver = std::vector<int> (*)(int, const Edges&);

std::vector<int> reference(int n, const Edges& edges) {
  struct Edge { int to; };
  std::vector<std::vector<Edge>> adj(n);
  // Match the public API's self-loop/duplicate semantics and include this cost.
  std::vector<unsigned char> seen(static_cast<std::size_t>(n) * n);
  for (auto [u, v] : edges) if (u != v && !seen[static_cast<std::size_t>(u) * n + v]) {
    seen[static_cast<std::size_t>(u) * n + v] = true;
    seen[static_cast<std::size_t>(v) * n + u] = true;
    adj[u].push_back({v});
    adj[v].push_back({u});
  }
  return official_reference::MaxMatching<Edge>(adj).mt;
}

int main() {
  constexpr int repeats = 7, batch = 20;
  struct Case { std::string name; int n, expected; Edges edges; };
  std::vector<Case> cases;
  std::mt19937 rng(20261003);
  Case dense{"dense", 500, 250, {}}, sparse{"sparse_planted", 500, 250, {}};
  for (int u = 0; u < 500; ++u) {
    if (u % 2 == 0) sparse.edges.emplace_back(u, u + 1);
    for (int v = u + 1; v < 500; ++v) if (rng() % 2) dense.edges.emplace_back(u, v);
  }
  for (int i = 0; i < 2000; ++i) sparse.edges.emplace_back(rng() % 500, rng() % 500);
  std::shuffle(dense.edges.begin(), dense.edges.end(), rng);
  std::shuffle(sparse.edges.begin(), sparse.edges.end(), rng);
  cases.push_back(dense);
  cases.push_back(sparse);
  Case odd_complete{"odd_complete", 499, 249, {}};
  for (int u = 0; u < 499; ++u) for (int v = u + 1; v < 499; ++v)
    odd_complete.edges.emplace_back(u, v);
  cases.push_back(std::move(odd_complete));
  Case nested{"nested_triangles", 499, 249, {}};
  for (int v = 0; v + 2 < 499; v += 2) {
    nested.edges.emplace_back(v, v + 1);
    nested.edges.emplace_back(v + 1, v + 2);
    nested.edges.emplace_back(v + 2, v);
  }
  cases.push_back(std::move(nested));
  Case odd_components{"odd_components", 498, 166, {}};
  for (int v = 0; v < 498; v += 3) {
    odd_components.edges.emplace_back(v, v + 1);
    odd_components.edges.emplace_back(v + 1, v + 2);
    odd_components.edges.emplace_back(v + 2, v);
  }
  cases.push_back(std::move(odd_components));
  Case augmentation{"augmenting_blossoms", 498, 249, {}};
  for (int v = 0; v < 498; v += 6) {
    for (auto [a, b] : Edges{{0, 1}, {1, 2}, {2, 0}, {1, 3}, {3, 4}, {4, 5}})
      augmentation.edges.emplace_back(v + a, v + b);
  }
  cases.push_back(std::move(augmentation));
  Case repeated = sparse;
  repeated.name = "duplicates_and_loops";
  for (int repeat = 0; repeat < 20; ++repeat)
    repeated.edges.insert(repeated.edges.end(), sparse.edges.begin(), sparse.edges.end());
  for (int v = 0; v < 500; ++v) repeated.edges.emplace_back(v, v);
  cases.push_back(std::move(repeated));

  const std::vector<std::pair<std::string, Solver>> solvers{
    {"without_greedy", benchmark_no_greedy::general_matching},
    {"without_early_stop", benchmark_no_early_stop::general_matching},
    {"selected", blueberry::general_matching},
    {"official_gabow_adapter", reference},
  };
  std::cout << "case,variant,n,m,repeats,batch,median_us,min_us,checksum\n" << std::fixed << std::setprecision(3);
  for (const auto& test : cases) for (const auto& [name, solve] : solvers) {
    std::vector<double> times;
    long long checksum = 0;
    for (int run = -1; run < repeats; ++run) {
      const auto start = Clock::now();
      for (int iter = 0; iter < batch; ++iter) {
        const auto mate = solve(test.n, test.edges);
        int count = 0;
        for (int u = 0; u < test.n; ++u) count += u < mate[u];
        if (count != test.expected) {
          std::cerr << test.name << '/' << name << ": expected " << test.expected << ", got " << count << '\n';
          return 1;
        }
        checksum += count;
      }
      const auto finish = Clock::now();
      if (run >= 0) times.push_back(std::chrono::duration<double, std::micro>(finish - start).count() / batch);
    }
    std::sort(times.begin(), times.end());
    std::cout << test.name << ',' << name << ',' << test.n << ',' << test.edges.size() << ','
              << repeats << ',' << batch << ',' << times[times.size() / 2] << ',' << times.front()
              << ',' << checksum << '\n';
  }
}

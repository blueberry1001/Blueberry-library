#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <tuple>
#include <vector>

#ifdef BLUEBERRY_BASELINE_DIJKSTRA
#include "baseline-dijkstra.hpp"
#else
#include "blueberry/graph/dijkstra.hpp"
#endif
#include "blueberry/data-structure/offline-fenwick-tree-2d.hpp"

using Clock = std::chrono::steady_clock;
double milliseconds(Clock::time_point start) {
  return std::chrono::duration<double, std::milli>(Clock::now() - start).count();
}

// Dense reference: useful only when the whole grid fits in memory.
struct DenseFenwick2D {
  int n;
  std::vector<long long> data;
  explicit DenseFenwick2D(int side) : n(side), data((n + 1) * (n + 1)) {}
  void add(int x, int y, long long delta) {
    for (int i = x + 1; i <= n; i += i & -i)
      for (int j = y + 1; j <= n; j += j & -j) data[i * (n + 1) + j] += delta;
  }
  long long pref(int x, int y) const {
    long long answer = 0;
    for (int i = x; i > 0; i -= i & -i)
      for (int j = y; j > 0; j -= j & -j) answer += data[i * (n + 1) + j];
    return answer;
  }
  long long sum(int l, int d, int r, int u) const {
    return pref(r, u) - pref(l, u) - pref(r, d) + pref(l, d);
  }
};

template <class Tree>
void run_rectangles(Tree& tree, const std::vector<std::pair<int, int>>& points,
                    const std::vector<std::tuple<int, int, int, int>>& rectangles,
                    double build_ms) {
  long long checksum = 0;
  const auto start = Clock::now();
  for (std::size_t i = 0; i < rectangles.size(); ++i) {
    const auto [x, y] = points[i % points.size()];
    tree.add(x, y, 1);
    const auto [l, d, r, u] = rectangles[i];
    checksum += tree.sum(l, d, r, u);
  }
  std::cout << "build_ms=" << build_ms << " operation_ms=" << milliseconds(start)
            << " checksum=" << checksum << '\n';
}

int main(int argc, char** argv) {
  if (argc != 2) return 1;
  const std::string candidate = argv[1];
  std::mt19937_64 random(1729);
  if (candidate == "dijkstra") {
    const int n = 100000, m = 500000;
    const auto build_start = Clock::now();
    blueberry::WeightedGraph<long long> graph(n);
    for (int i = 1; i < n; ++i) graph[i - 1].push_back({i, 1});
    for (int i = n - 1; i < m; ++i) {
      int u = random() % n, v = random() % n;
      graph[u].push_back({v, static_cast<long long>(random() % 1000)});
    }
    const auto build_elapsed = milliseconds(build_start);
    const auto start = Clock::now();
    auto result = blueberry::dijkstra(graph, 0);
    const auto elapsed = milliseconds(start);
    long long checksum = 0;
    for (auto distance : result.distance) checksum += distance;
    std::cout << "build_ms=" << build_elapsed << " operation_ms=" << elapsed
              << " checksum=" << checksum << '\n';
    return 0;
  }
  const int side = 1024, n = 30000, q = 60000;
  std::vector<std::pair<int, int>> points;
  std::vector<std::tuple<int, int, int, int>> rectangles;
  for (int i = 0; i < n; ++i) points.emplace_back(random() % side, random() % side);
  for (int i = 0; i < q; ++i) {
    int l = random() % (side + 1), r = random() % (side + 1);
    int d = random() % (side + 1), u = random() % (side + 1);
    if (l > r) std::swap(l, r);
    if (d > u) std::swap(d, u);
    rectangles.emplace_back(l, d, r, u);
  }
  const auto start = Clock::now();
  if (candidate == "compressed") {
    blueberry::OfflineFenwickTree2D<long long, int> tree(points);
    const auto elapsed = milliseconds(start);
    run_rectangles(tree, points, rectangles, elapsed);
  } else if (candidate == "dense") {
    DenseFenwick2D tree(side);
    const auto elapsed = milliseconds(start);
    run_rectangles(tree, points, rectangles, elapsed);
  } else return 1;
}

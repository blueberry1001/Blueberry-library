#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <limits>
#include <queue>
#include <type_traits>
#include <utility>
#include <vector>

namespace blueberry {

template <class Cost>
struct WeightedEdge {
  int to;
  Cost cost;
};

template <class Cost>
using WeightedGraph = std::vector<std::vector<WeightedEdge<Cost>>>;

template <class Cost>
struct ShortestPathResult {
  std::vector<Cost> distance;
  std::vector<int> parent;
  Cost infinity;

  std::vector<int> path_to(int target) const {
    assert(0 <= target && target < static_cast<int>(distance.size()));
    if (distance[target] == infinity) return {};
    std::vector<int> path;
    for (int v = target; v != -1; v = parent[v]) path.push_back(v);
    std::reverse(path.begin(), path.end());
    return path;
  }
};

/**
 * @brief Dijkstra法
 */
template <class Cost>
ShortestPathResult<Cost> dijkstra(
    const WeightedGraph<Cost>& graph, int source,
    Cost infinity = std::numeric_limits<Cost>::max()) {
  const int n = static_cast<int>(graph.size());
  assert(0 <= source && source < n);
  assert(Cost{} < infinity);
  std::vector<Cost> distance(n, infinity);
  std::vector<int> parent(n, -1);
  using QueueElement = std::pair<Cost, int>;
  std::priority_queue<QueueElement, std::vector<QueueElement>,
                      std::greater<QueueElement>>
      queue;
  distance[source] = Cost{};
  queue.emplace(distance[source], source);

  while (!queue.empty()) {
    const auto [current_distance, v] = queue.top();
    queue.pop();
    if (distance[v] != current_distance) continue;
    for (const auto& edge : graph[v]) {
      assert(0 <= edge.to && edge.to < n);
      assert(!(edge.cost < Cost{}));
      // Integral distances are clipped at infinity before adding, including
      // unsigned types. Keep the original addition-only contract for custom Cost.
      if constexpr (std::is_integral_v<Cost>) {
        if (edge.cost >= infinity - current_distance) continue;
      }
      const Cost candidate = current_distance + edge.cost;
      if (distance[edge.to] > candidate) {
        distance[edge.to] = candidate;
        parent[edge.to] = v;
        queue.emplace(distance[edge.to], edge.to);
      }
    }
  }
  return {std::move(distance), std::move(parent), infinity};
}

}  // namespace blueberry

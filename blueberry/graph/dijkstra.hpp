#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <climits>
#include <cstddef>
#include <functional>
#include <limits>
#include <queue>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace blueberry {

namespace detail {
template <class Cost, bool = std::is_integral_v<Cost> && !std::is_same_v<Cost, bool> && (sizeof(Cost) * CHAR_BIT <= 64)>
class DijkstraQueue {
  using Item = std::pair<Cost, int>;
  std::priority_queue<Item, std::vector<Item>, std::greater<Item>> queue_;
 public:
  void emplace(const Cost& d, int v) { queue_.emplace(d, v); }
  bool empty() const { return queue_.empty(); }
  Item pop() { Item x = queue_.top(); queue_.pop(); return x; }
};
template <class Cost>
class DijkstraQueue<Cost, true> {
  using U = std::make_unsigned_t<Cost>;
  using Item = std::pair<U, int>;
  std::array<std::vector<Item>, std::numeric_limits<U>::digits + 1> bins_;
  std::vector<int> same_;
  U last_ = 0;
  std::size_t size_ = 0;
  Item single_{};
  bool has_single_ = false;
  void add(U key, int v) {
    if (key == last_) { same_.push_back(v); std::push_heap(same_.begin(), same_.end(), std::greater<int>()); }
    else bins_[std::bit_width(U(key ^ last_))].emplace_back(key, v);
  }
 public:
  void emplace(Cost d, int v) {
    U key = static_cast<U>(d);
    if (!size_) { single_ = {key, v}; has_single_ = true; size_ = 1; return; }
    if (has_single_) { add(single_.first, single_.second); has_single_ = false; }
    add(key, v);
    ++size_;
  }
  bool empty() const { return size_ == 0; }
  std::pair<Cost, int> pop() {
    if (has_single_) {
      has_single_ = false; --size_; last_ = single_.first;
      return {static_cast<Cost>(last_), single_.second};
    }
    if (same_.empty()) {
      std::size_t i = 1;
      while (bins_[i].empty()) ++i;
      U minimum = bins_[i][0].first;
      for (auto [d, v] : bins_[i]) { (void)v; minimum = std::min(minimum, d); }
      last_ = minimum;
      for (auto [d, v] : bins_[i]) {
        if (d == last_) same_.push_back(v);
        else bins_[std::bit_width(U(d ^ last_))].emplace_back(d, v);
      }
      bins_[i].clear();
      std::make_heap(same_.begin(), same_.end(), std::greater<int>());
    }
    std::pop_heap(same_.begin(), same_.end(), std::greater<int>());
    int v = same_.back(); same_.pop_back(); --size_;
    return {static_cast<Cost>(last_), v};
  }
};
}

template <class Cost>
struct WeightedEdge {
  int to;
  Cost cost;
};

template <class Cost>
using WeightedGraph = std::vector<std::vector<WeightedEdge<Cost>>>;

template <class Cost>
class WeightedCsrGraph {
  std::vector<std::size_t> offsets_;
  std::vector<WeightedEdge<Cost>> edges_;
 public:
  WeightedCsrGraph() = default;
  WeightedCsrGraph(int n, const std::vector<std::tuple<int, int, Cost>>& edges)
      : offsets_(static_cast<std::size_t>(n) + 1), edges_(edges.size()) {
    for (const auto& edge : edges) ++offsets_[static_cast<std::size_t>(std::get<0>(edge)) + 1];
    for (int i = 0; i < n; ++i) offsets_[i + 1] += offsets_[i];
    auto position = offsets_;
    for (const auto& [from, to, cost] : edges) edges_[position[from]++] = {to, cost};
  }
  WeightedCsrGraph(const WeightedCsrGraph&) = default;
  WeightedCsrGraph(WeightedCsrGraph&&) noexcept = default;
  WeightedCsrGraph& operator=(const WeightedCsrGraph& other) {
    if (this != &other) {
      WeightedCsrGraph copy(other);
      offsets_.swap(copy.offsets_); edges_.swap(copy.edges_);
    }
    return *this;
  }
  WeightedCsrGraph& operator=(WeightedCsrGraph&& other) noexcept {
    if (this != &other) { offsets_ = std::move(other.offsets_); edges_ = std::move(other.edges_); }
    return *this;
  }
  int size() const { return offsets_.empty() ? 0 : static_cast<int>(offsets_.size() - 1); }
  std::span<const WeightedEdge<Cost>> operator[](int v) const {
    return std::span<const WeightedEdge<Cost>>(edges_).subspan(offsets_[v], offsets_[v + 1] - offsets_[v]);
  }
};

namespace detail {
template <class> struct CsrCost {};
template <class Cost> struct CsrCost<WeightedCsrGraph<Cost>> { using type = Cost; };
}

template <class Cost>
struct ShortestPathResult {
  std::vector<Cost> distance;
  std::vector<int> parent;
  Cost infinity;

  std::vector<int> path_to(int target) const {
    if (distance[target] == infinity) return {};
    std::vector<int> path;
    for (int v = target; v != -1; v = parent[v]) path.push_back(v);
    std::reverse(path.begin(), path.end());
    return path;
  }
};

namespace detail {
template <bool Single, class Cost, class Graph>
ShortestPathResult<Cost> dijkstra_impl(
    const Graph& graph, int source, Cost infinity, int target) {
  const int n = static_cast<int>(graph.size());
  std::vector<Cost> distance(n, infinity);
  std::vector<int> parent(n, -1);
  distance[source] = Cost{};
  auto run = [&](auto& queue) {
    queue.emplace(distance[source], source);
    while (!queue.empty()) {
      const auto [current_distance, v] = queue.pop();
      if (distance[v] != current_distance) continue;
      if constexpr (Single) if (v == target) break;
      for (const auto& edge : graph[v]) {
        Cost candidate{};
        if constexpr (std::is_integral_v<Cost>) {
          if (distance[edge.to] <= current_distance ||
              +edge.cost >= distance[edge.to] - current_distance) continue;
          candidate = current_distance + edge.cost;
        } else {
          candidate = current_distance + edge.cost;
          if (!(distance[edge.to] > candidate)) continue;
        }
        distance[edge.to] = candidate;
        parent[edge.to] = v;
        queue.emplace(candidate, edge.to);
      }
    }
  };
  if constexpr (std::is_integral_v<Cost> && !std::is_same_v<Cost, bool> && sizeof(Cost) * CHAR_BIT <= 64) {
    if (n <= 256) { DijkstraQueue<Cost, false> queue; run(queue); }
    else { DijkstraQueue<Cost, true> queue; run(queue); }
  } else { DijkstraQueue<Cost, false> queue; run(queue); }
  return {std::move(distance), std::move(parent), infinity};
}
}

template <class Cost>
ShortestPathResult<Cost> dijkstra(
    const WeightedGraph<Cost>& graph, int source,
    Cost infinity = std::numeric_limits<Cost>::max()) {
  return detail::dijkstra_impl<false>(graph, source, infinity, -1);
}

template <class Graph>
requires requires { typename detail::CsrCost<Graph>::type; }
auto dijkstra(
    const Graph& graph, int source,
    typename detail::CsrCost<Graph>::type infinity = std::numeric_limits<typename detail::CsrCost<Graph>::type>::max())
    -> ShortestPathResult<typename detail::CsrCost<Graph>::type> {
  return detail::dijkstra_impl<false>(graph, source, infinity, -1);
}

template <class Cost>
std::pair<Cost, std::vector<int>> dijkstra_path(
    const WeightedGraph<Cost>& graph, int source, int target,
    Cost infinity = std::numeric_limits<Cost>::max()) {
  if (source == target) return {Cost{}, {source}};
  auto result = detail::dijkstra_impl<true>(graph, source, infinity, target);
  return {result.distance[target], result.path_to(target)};
}

template <class Graph>
requires requires { typename detail::CsrCost<Graph>::type; }
auto dijkstra_path(
    const Graph& graph, int source, int target,
    typename detail::CsrCost<Graph>::type infinity = std::numeric_limits<typename detail::CsrCost<Graph>::type>::max())
    -> std::pair<typename detail::CsrCost<Graph>::type, std::vector<int>> {
  if (source == target) return {typename detail::CsrCost<Graph>::type{}, {source}};
  auto result = detail::dijkstra_impl<true>(graph, source, infinity, target);
  return {result.distance[target], result.path_to(target)};
}

}

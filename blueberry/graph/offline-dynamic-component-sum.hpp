#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <map>
#include <utility>
#include <vector>

#include "blueberry/data-structure/rollback-union-find.hpp"

namespace blueberry {

// Initially empty undirected multigraph; vertex additions persist across cuts.
// T needs copying and associative, commutative + (no zero or inverse required).
// All intermediate sums, including reordered partial sums, must fit in T.
template <class T>
class OfflineDynamicComponentSum {
 public:
  explicit OfflineDynamicComponentSum(const std::vector<T>& values = {}) {
    assert(values.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    initial_ = values;
  }

  int size() const { return static_cast<int>(initial_.size()); }

  void add_edge(int u, int v) {
    check_vertex(u);
    check_vertex(v);
    record_operation();
    if (u > v) std::swap(u, v);
    auto& edge = active_[{u, v}];
    if (edge.count++ == 0) edge.begin = static_cast<int>(queries_.size());
  }

  // Remove one copy. A missing edge leaves the recording unchanged.
  bool remove_edge(int u, int v) {
    check_vertex(u);
    check_vertex(v);
    if (u > v) std::swap(u, v);
    const auto it = active_.find({u, v});
    if (it == active_.end()) return false;
    record_operation();
    if (--it->second.count == 0) {
      const int end = static_cast<int>(queries_.size());
      if (u != v && it->second.begin < end)
        closed_.push_back({it->second.begin, end, u, v});
      active_.erase(it);
    }
    return true;
  }

  void add_value(int v, const T& delta) {
    check_vertex(v);
    record_operation();
    updates_.push_back({static_cast<int>(queries_.size()), v, delta});
  }

  // Observe all preceding calls; return the index in every subsequent solve().
  int query(int v) {
    check_vertex(v);
    record_operation();
    const int index = static_cast<int>(queries_.size());
    queries_.push_back(v);
    return index;
  }

  std::vector<T> solve() const {
    const int k = static_cast<int>(queries_.size());
    if (k == 0) return {};
    int base = 1;
    while (base < k) base *= 2;

    // Only query positions need leaves. Unobserved closed lifetimes vanish;
    // open edges and suffix updates remain in the recorder for future queries.
    std::vector<EdgeInterval> edges;
    edges.reserve(closed_.size() + active_.size());
    edges.insert(edges.end(), closed_.begin(), closed_.end());
    for (const auto& [ends, edge] : active_)
      if (ends.first != ends.second && edge.begin < k)
        edges.push_back({edge.begin, k, ends.first, ends.second});
    const std::size_t updates = static_cast<std::size_t>(
        std::lower_bound(updates_.begin(), updates_.end(), k,
                         [](const Update& update, int time) { return update.begin < time; }) -
        updates_.begin());

    const auto visit_interval = [base](int left, int right, auto&& visit) {
      for (left += base, right += base; left < right; left /= 2, right /= 2) {
        if (left & 1) visit(left++);
        if (right & 1) visit(--right);
      }
    };
    // Flat segment-tree buckets avoid a separate vector/allocation per node.
    // Nonnegative event IDs are edges; negative IDs encode ~update_index.
    const auto visit_events = [&](auto&& visit) {
      for (std::size_t i = 0; i < edges.size(); ++i)
        visit_interval(edges[i].left, edges[i].right,
                       [&](int node) { visit(node, static_cast<int>(i)); });
      for (std::size_t i = 0; i < updates; ++i)
        visit_interval(updates_[i].begin, k,
                       [&](int node) { visit(node, ~static_cast<int>(i)); });
    };
    std::vector<std::size_t> offsets(2 * base + 1);
    visit_events([&](int node, int) { ++offsets[node + 1]; });
    for (std::size_t i = 1; i < offsets.size(); ++i) offsets[i] += offsets[i - 1];
    std::vector<int> events(offsets.back());
    {
      auto cursor = offsets;
      visit_events([&](int node, int event) { events[cursor[node]++] = event; });
    }

    RollbackUnionFind uf(size());
    std::vector<T> sums = initial_;
    struct Change { int root; T old; };
    std::vector<Change> history;
    history.reserve(std::min(initial_.size(), edges.size()) + updates);
    std::vector<T> answers;
    answers.reserve(k);
    const auto dfs = [&](auto&& self, int node) -> void {
      const int uf_state = uf.state();
      const std::size_t sum_state = history.size();
      for (std::size_t pos = offsets[node]; pos < offsets[node + 1]; ++pos) {
        const int event = events[pos];
        if (event >= 0) {
          const auto& edge = edges[event];
          const int u = uf.leader(edge.u), v = uf.leader(edge.v);
          if (u == v) continue;
          T combined = sums[u] + sums[v];
          uf.merge(u, v);
          const int root = uf.leader(u);
          history.push_back({root, sums[root]});
          sums[root] = combined;
        } else {
          const auto& update = updates_[~event];
          const int root = uf.leader(update.vertex);
          history.push_back({root, sums[root]});
          T combined = sums[root] + update.value;
          sums[root] = combined;
        }
      }
      if (node < base) {
        self(self, 2 * node);
        self(self, 2 * node + 1);
      } else if (node - base < k) {
        answers.push_back(sums[uf.leader(queries_[node - base])]);
      }
      // Restore raw saved root slots, without looking up their current leader.
      while (history.size() > sum_state) {
        sums[history.back().root] = history.back().old;
        history.pop_back();
      }
      uf.rollback(uf_state);
    };
    dfs(dfs, 1);
    return answers;
  }

 private:
  struct ActiveEdge { int count = 0, begin = 0; };
  struct EdgeInterval { int left, right, u, v; };
  struct Update { int begin, vertex; T value; };

  void check_vertex([[maybe_unused]] int v) const { assert(0 <= v && v < size()); }
  void record_operation() {
    assert(operations_ < std::numeric_limits<int>::max() / 4);
    ++operations_;
  }

  std::vector<T> initial_;
  std::map<std::pair<int, int>, ActiveEdge> active_;
  std::vector<EdgeInterval> closed_;
  std::vector<Update> updates_;
  std::vector<int> queries_;
  int operations_ = 0;
};

}  // namespace blueberry

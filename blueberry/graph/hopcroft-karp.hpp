#pragma once

#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

namespace blueberry {

class HopcroftKarp {
 public:
  HopcroftKarp(int left_n, int right_n, const std::vector<std::pair<int, int>>& edges) {
    left_.assign(left_n, -1);
    right_.assign(right_n, -1);
    offset_.assign(static_cast<std::size_t>(left_n) + 1, 0);
    for (const auto& [u, v] : edges) {
      ++offset_[u + 1];
    }
    for (int u = 0; u < left_n; ++u) offset_[u + 1] += offset_[u];
    if (edges.empty()) return;
    adjacency_.resize(edges.size());
    auto next = offset_;
    for (const auto& [u, v] : edges) adjacency_[next[u]++] = v;

    for (int u = 0; u < left_n; ++u) {
      for (std::size_t i = offset_[u]; i < offset_[u + 1]; ++i) {
        const int v = adjacency_[i];
        if (right_[v] == -1) { left_[u] = v; right_[v] = u; ++size_; break; }
      }
    }
    if (size_ == std::min(left_n, right_n)) return;

    std::vector<int> distance(left_n), queue, path, path_right;
    queue.reserve(left_n);
    path.reserve(left_n);
    path_right.reserve(left_n);
    while (true) {
      std::fill(distance.begin(), distance.end(), -1);
      queue.clear();
      for (int u = 0; u < left_n; ++u) if (left_[u] == -1) {
        distance[u] = 0;
        queue.push_back(u);
      }
      int shortest = left_n + 1;
      for (std::size_t head = 0; head < queue.size(); ++head) {
        const int u = queue[head];
        if (distance[u] >= shortest) continue;
        for (std::size_t i = offset_[u]; i < offset_[u + 1]; ++i) {
          const int v = adjacency_[i], mate = right_[v];
          if (mate == -1) { shortest = distance[u] + 1; break; }
          else if (distance[mate] == -1) {
            distance[mate] = distance[u] + 1;
            queue.push_back(mate);
          }
        }
        if (shortest != left_n + 1) break;
      }
      if (shortest == left_n + 1) break;
      next = offset_;
      for (int root = 0; root < left_n; ++root) {
        if (left_[root] != -1 || distance[root] == -1) continue;
        path.assign(1, root);
        path_right.clear();
        while (!path.empty()) {
          const int u = path.back();
          bool descend = false, augmented = false;
          while (next[u] < offset_[u + 1]) {
            const int v = adjacency_[next[u]++], mate = right_[v];
            if (mate == -1) {
              if (distance[u] + 1 != shortest) continue;
              left_[u] = v;
              right_[v] = u;
              distance[u] = -1;
              for (int i = static_cast<int>(path_right.size()) - 1; i >= 0; --i) {
                left_[path[i]] = path_right[i];
                right_[path_right[i]] = path[i];
                distance[path[i]] = -1;
              }
              ++size_;
              augmented = true;
              break;
            }
            if (distance[mate] == distance[u] + 1 && distance[mate] < shortest) {
              path.push_back(mate);
              path_right.push_back(v);
              descend = true;
              break;
            }
          }
          if (augmented) break;
          if (!descend) {
            distance[u] = -1;
            path.pop_back();
            if (!path_right.empty()) path_right.pop_back();
          }
        }
      }
    }
  }

  int size() const { return size_; }
  int left_match(int u) const {
    return left_[u];
  }
  int right_match(int v) const {
    return right_[v];
  }
  std::vector<std::pair<int, int>> pairs() const {
    std::vector<std::pair<int, int>> result;
    result.reserve(size_);
    for (int u = 0; u < static_cast<int>(left_.size()); ++u)
      if (left_[u] != -1) result.emplace_back(u, left_[u]);
    return result;
  }
  std::pair<std::vector<int>, std::vector<int>> min_vertex_cover() const {
    std::vector<unsigned char> seen_left(left_.size()), seen_right(right_.size());
    std::vector<int> queue;
    queue.reserve(left_.size());
    for (int u = 0; u < static_cast<int>(left_.size()); ++u) if (left_[u] == -1) {
      seen_left[u] = true;
      queue.push_back(u);
    }
    for (std::size_t head = 0; head < queue.size(); ++head) {
      const int u = queue[head];
      for (std::size_t i = offset_[u]; i < offset_[u + 1]; ++i) {
        const int v = adjacency_[i];
        if (left_[u] == v || seen_right[v]) continue;
        seen_right[v] = true;
        const int mate = right_[v];
        if (mate != -1 && !seen_left[mate]) {
          seen_left[mate] = true;
          queue.push_back(mate);
        }
      }
    }
    std::pair<std::vector<int>, std::vector<int>> result;
    for (int u = 0; u < static_cast<int>(left_.size()); ++u) if (!seen_left[u]) result.first.push_back(u);
    for (int v = 0; v < static_cast<int>(right_.size()); ++v) if (seen_right[v]) result.second.push_back(v);
    return result;
  }

 private:
  int size_ = 0;
  std::vector<int> left_, right_, adjacency_;
  std::vector<std::size_t> offset_;
};

}

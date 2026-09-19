#pragma once
#include <algorithm>
#include <cassert>
#include <limits>
#include <vector>
#include <atcoder/fenwicktree>
#include "blueberry/graph/centroid-decomposition.hpp"

namespace blueberry {
namespace internal {
// Distances are unweighted. Flatten the centroid and excluded-branch arrays.
class ContourLayout {
 public:
  CentroidDecomposition decomposition;
  std::vector<int> offset, length;
  std::vector<std::vector<int>> branches;
  int cells = 0;
  explicit ContourLayout(const std::vector<std::vector<int>>& tree) : decomposition(tree), length(tree.size(), 1), branches(tree.size()) {
    for (int v = 0; v < decomposition.size(); ++v)
      for (auto p : decomposition.ancestors(v)) if (p.branch >= 0)
        if (static_cast<int>(branches[p.centroid].size()) <= p.branch) branches[p.centroid].resize(p.branch + 1, -1);
    for (auto& list : branches) for (int& id : list) {
      assert(length.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
      id = static_cast<int>(length.size()); length.push_back(1);
    }
    for (int v = 0; v < decomposition.size(); ++v)
      for (auto p : decomposition.ancestors(v)) {
        length[p.centroid] = std::max(length[p.centroid], p.distance + 1);
        if (p.branch >= 0) {
          int b = branches[p.centroid][p.branch]; length[b] = std::max(length[b], p.distance + 1);
        }
      }
    for (int n : length) {
      assert(cells <= std::numeric_limits<int>::max() - n);
      offset.push_back(cells); cells += n;
    }
  }
};
}  // namespace internal

// Vertex add / sum of vertices at distance in [l,r), including distance zero.
template <class T>
class ContourQuery {
  internal::ContourLayout layout_;
  atcoder::fenwick_tree<T> bit_;
  T range(int id, int l, int r) {
    l = std::clamp(l, 0, layout_.length[id]); r = std::clamp(r, 0, layout_.length[id]);
    return bit_.sum(layout_.offset[id] + l, layout_.offset[id] + r);
  }
 public:
  explicit ContourQuery(const std::vector<std::vector<int>>& tree) : layout_(tree), bit_(layout_.cells) {}
  int size() const { return layout_.decomposition.size(); }
  void add(int v, T value) {
    for (auto p : layout_.decomposition.ancestors(v)) {
      bit_.add(layout_.offset[p.centroid] + p.distance, value);
      if (p.branch >= 0) bit_.add(layout_.offset[layout_.branches[p.centroid][p.branch]] + p.distance, value);
    }
  }
  T sum(int v, int l, int r) {
    assert(0 <= l && l <= r);
    T answer = T{};
    for (auto p : layout_.decomposition.ancestors(v)) {
      answer += range(p.centroid, l - p.distance, r - p.distance);
      if (p.branch >= 0) answer -= range(layout_.branches[p.centroid][p.branch], l - p.distance, r - p.distance);
    }
    return answer;
  }
};

// Transposed operation: add to a distance range, then query a single vertex.
template <class T>
class ContourAdd {
  internal::ContourLayout layout_;
  atcoder::fenwick_tree<T> bit_;
  void range(int id, int l, int r, T value) {
    const int n = layout_.length[id], start = layout_.offset[id];
    l = std::clamp(l, 0, n); r = std::clamp(r, 0, n);
    if (l < n) bit_.add(start + l, value);
    if (r < n) bit_.add(start + r, -value);
  }
  T at(int id, int p) { return bit_.sum(layout_.offset[id], layout_.offset[id] + p + 1); }
 public:
  explicit ContourAdd(const std::vector<std::vector<int>>& tree) : layout_(tree), bit_(layout_.cells) {}
  int size() const { return layout_.decomposition.size(); }
  void add(int v, int l, int r, T value) {
    assert(0 <= l && l <= r);
    for (auto p : layout_.decomposition.ancestors(v)) {
      range(p.centroid, l - p.distance, r - p.distance, value);
      if (p.branch >= 0) range(layout_.branches[p.centroid][p.branch], l - p.distance, r - p.distance, value);
    }
  }
  T get(int v) {
    T answer = T{};
    for (auto p : layout_.decomposition.ancestors(v)) {
      answer += at(p.centroid, p.distance);
      if (p.branch >= 0) answer -= at(layout_.branches[p.centroid][p.branch], p.distance);
    }
    return answer;
  }
};
}  // namespace blueberry

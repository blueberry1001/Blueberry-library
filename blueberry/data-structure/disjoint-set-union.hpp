#pragma once

#include <algorithm>
#include <cassert>
#include <vector>

namespace blueberry {

/**
 * @brief 素集合データ構造 (Union Find)
 */
class DisjointSetUnion {
 public:
  explicit DisjointSetUnion(int n) {
    assert(n >= 0);
    parent_or_size_.assign(n, -1);
  }

  int leader(int v) {
    assert(0 <= v && v < size());
    if (parent_or_size_[v] < 0) return v;
    return parent_or_size_[v] = leader(parent_or_size_[v]);
  }

  bool merge(int u, int v) {
    u = leader(u);
    v = leader(v);
    if (u == v) return false;
    if (-parent_or_size_[u] < -parent_or_size_[v]) std::swap(u, v);
    parent_or_size_[u] += parent_or_size_[v];
    parent_or_size_[v] = u;
    return true;
  }

  bool same(int u, int v) { return leader(u) == leader(v); }

  int component_size(int v) { return -parent_or_size_[leader(v)]; }

  // Short spelling for use in tight contest code.
  int comp_size(int v) { return component_size(v); }

  int size() const { return static_cast<int>(parent_or_size_.size()); }

  std::vector<std::vector<int>> groups() {
    std::vector<int> leader_size(size());
    std::vector<std::vector<int>> result(size());
    for (int v = 0; v < size(); ++v) ++leader_size[leader(v)];
    for (int v = 0; v < size(); ++v) result[v].reserve(leader_size[v]);
    for (int v = 0; v < size(); ++v) result[leader(v)].push_back(v);
    result.erase(std::remove_if(result.begin(), result.end(),
                                [](const auto& group) { return group.empty(); }),
                 result.end());
    return result;
  }

 private:
  std::vector<int> parent_or_size_;
};

}  // namespace blueberry

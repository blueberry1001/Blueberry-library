#pragma once
#include <algorithm>
#include <cassert>
#include <limits>
#include <map>
#include <utility>
#include <vector>

namespace blueberry {
// Class IDs compare rooted subtrees within this call, without hash collisions.
inline std::vector<int> rooted_tree_isomorphism(const std::vector<int>& parent) {
  assert(parent.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
  int n = static_cast<int>(parent.size());
  if (n == 0) return {};
  std::vector<std::vector<int>> children(n);
  int root = -1;
  for (int v = 0; v < n; ++v) {
    if (parent[v] == -1) { assert(root == -1); root = v; }
    else { assert(0 <= parent[v] && parent[v] < n && parent[v] != v); children[parent[v]].push_back(v); }
  }
  assert(root != -1);
  std::vector<int> order{root}, id(n);
  for (std::size_t i = 0; i < order.size(); ++i)
    for (int v : children[order[i]]) order.push_back(v);
  assert(order.size() == parent.size());
  std::map<std::vector<int>, int> classes;
  for (auto it = order.rbegin(); it != order.rend(); ++it) {
    std::vector<int> key;
    key.reserve(children[*it].size());
    for (int v : children[*it]) key.push_back(id[v]);
    std::sort(key.begin(), key.end());
    id[*it] = classes.try_emplace(std::move(key), static_cast<int>(classes.size())).first->second;
  }
  return id;
}
}  // namespace blueberry

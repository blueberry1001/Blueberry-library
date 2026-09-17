#pragma once

#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

namespace blueberry {

// Min Cartesian tree, equal minima choose the leftmost index. Root is its own parent.
template <class T>
std::vector<int> cartesian_tree(const std::vector<T>& values) {
  assert(values.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
  const int n = static_cast<int>(values.size());
  std::vector<int> parent(n), stack;
  stack.reserve(n);
  for (int i = 0; i < n; ++i) {
    int left = -1;
    while (!stack.empty() && values[i] < values[stack.back()]) {
      left = stack.back();
      stack.pop_back();
    }
    parent[i] = stack.empty() ? i : stack.back();
    if (left != -1) parent[left] = i;
    stack.push_back(i);
  }
  return parent;
}

}  // namespace blueberry

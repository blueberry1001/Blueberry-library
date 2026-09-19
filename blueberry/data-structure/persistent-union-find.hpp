#pragma once
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>
#include "blueberry/data-structure/persistent-array.hpp"

namespace blueberry {

// Online full persistence via an immutable parent array, without path compression.
class PersistentUnionFind {
  PersistentArray<int> parents_;
  std::vector<int> roots_{0}, components_;
  int root(int version) const {
    assert(0 <= version && static_cast<std::size_t>(version) < roots_.size());
    return roots_[version];
  }
  std::pair<int, int> find(int version, int p) const {
    const int state = root(version);
    assert(0 <= p && p < size());
    int parent = parents_.get(state, p);
    while (parent >= 0) { p = parent; parent = parents_.get(state, p); }
    return {p, -parent};
  }

 public:
  explicit PersistentUnionFind(int n = 0) : parents_(n, -1), components_{n} { assert(n >= 0); }
  int size() const { return parents_.size(); }
  int versions() const { return static_cast<int>(roots_.size()); }
  int leader(int version, int p) const { return find(version, p).first; }
  int comp_size(int version, int p) const { return find(version, p).second; }
  bool same(int version, int a, int b) const { return leader(version, a) == leader(version, b); }
  int components(int version) const { (void)root(version); return components_[version]; }
  int merge(int version, int a, int b) {
    auto [x, sx] = find(version, a);
    auto [y, sy] = find(version, b);
    int state = root(version), count = components_[version];
    if (x != y) {
      if (sx < sy) { std::swap(x, y); std::swap(sx, sy); }
      state = parents_.set(state, x, -(sx + sy));
      state = parents_.set(state, y, x);
      --count;
    }
    assert(roots_.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
    roots_.push_back(state);
    components_.push_back(count);
    return versions() - 1;
  }
};

}  // namespace blueberry

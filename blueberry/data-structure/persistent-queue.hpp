#pragma once

#include <cassert>
#include <limits>
#include <vector>

namespace blueberry {

// A version is a suffix of a root-to-node path in the append tree.
template <class T>
class PersistentQueue {
  struct Node { T value; int parent, jump, depth; };
  struct Version { int tail, size; };
  std::vector<Node> nodes_;
  std::vector<Version> versions_{{-1, 0}};
  int depth(int p) const { return p == -1 ? 0 : nodes_[p].depth; }
  void check([[maybe_unused]] int v) const { assert(0 <= v && v < versions()); }

 public:
  PersistentQueue() = default;
  int versions() const { return static_cast<int>(versions_.size()); }
  int size(int v) const { check(v); return versions_[v].size; }
  bool empty(int v) const { return size(v) == 0; }
  int push(int v, const T& value) {
    check(v);
    assert(versions_.size() < static_cast<unsigned>(std::numeric_limits<int>::max()));
    const auto old = versions_[v];
    int p = old.tail, jump = p;
    if (p != -1) {
      int j = nodes_[p].jump;
      if (j != -1 && depth(p) - depth(j) == depth(j) - depth(nodes_[j].jump))
        jump = nodes_[j].jump;
    }
    nodes_.push_back({value, p, jump, depth(p) + 1});
    versions_.push_back({static_cast<int>(nodes_.size()) - 1, old.size + 1});
    return versions() - 1;
  }
  int pop(int v) {
    check(v);
    assert(!empty(v));
    assert(versions_.size() < static_cast<unsigned>(std::numeric_limits<int>::max()));
    const auto old = versions_[v];
    versions_.push_back({old.size == 1 ? -1 : old.tail, old.size - 1});
    return versions() - 1;
  }
  T front(int v) const {
    check(v);
    assert(!empty(v));
    int p = versions_[v].tail;
    const int target = depth(p) - versions_[v].size + 1;
    while (depth(p) > target) {
      int j = nodes_[p].jump;
      p = depth(j) >= target ? j : nodes_[p].parent;
    }
    return nodes_[p].value;
  }
};

}  // namespace blueberry

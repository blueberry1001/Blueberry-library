#pragma once
#include <algorithm>
#include <cassert>
#include <limits>
#include <numeric>
#include <utility>
#include <vector>

namespace blueberry {
// Fixed 2D points, point assignment, commutative rectangle aggregation.
// Median splits alternate axes; duplicate coordinates retain distinct IDs.
template <class Coord, class S, auto op, auto e>
class KDTree {
  struct Node {
    Coord xmin, xmax, ymin, ymax;
    int left, right, parent;
    S product;
  };
  std::vector<Node> nodes_;
  std::vector<int> where_;
  S query(int id, Coord xl, Coord yl, Coord xr, Coord yr) const {
    const auto& n = nodes_[id];
    if (n.xmax < xl || !(n.xmin < xr) || n.ymax < yl || !(n.ymin < yr)) return e();
    if (!(n.xmin < xl) && n.xmax < xr && !(n.ymin < yl) && n.ymax < yr) return n.product;
    return op(query(n.left, xl, yl, xr, yr), query(n.right, xl, yl, xr, yr));
  }
 public:
  KDTree(const std::vector<std::pair<Coord, Coord>>& points, const std::vector<S>& values) : where_(points.size()) {
    assert(points.size() == values.size());
    assert(points.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max() / 2));
    if (points.empty()) return;
    nodes_.reserve(2 * points.size() - 1);
    std::vector<int> ids(points.size()); std::iota(ids.begin(), ids.end(), 0);
    auto build = [&](auto&& self, int l, int r, int parent, int axis) -> int {
      const int id = static_cast<int>(nodes_.size()), p = ids[l];
      nodes_.push_back({points[p].first, points[p].first, points[p].second, points[p].second, -1, -1, parent, values[p]});
      if (r - l == 1) { where_[p] = id; return id; }
      const int middle = l + (r - l) / 2;
      std::nth_element(ids.begin() + l, ids.begin() + middle, ids.begin() + r, [&](int a, int b) {
        const Coord& x = axis ? points[a].second : points[a].first;
        const Coord& y = axis ? points[b].second : points[b].first;
        if (x < y) return true;
        if (y < x) return false;
        return a < b;
      });
      const int a = self(self, l, middle, id, axis ^ 1), b = self(self, middle, r, id, axis ^ 1);
      auto& n = nodes_[id]; n.left = a; n.right = b;
      n.xmin = std::min(nodes_[a].xmin, nodes_[b].xmin); n.xmax = std::max(nodes_[a].xmax, nodes_[b].xmax);
      n.ymin = std::min(nodes_[a].ymin, nodes_[b].ymin); n.ymax = std::max(nodes_[a].ymax, nodes_[b].ymax);
      n.product = op(nodes_[a].product, nodes_[b].product);
      return id;
    };
    build(build, 0, size(), -1, 0);
  }
  int size() const { return static_cast<int>(where_.size()); }
  S get(int p) const { assert(0 <= p && p < size()); return nodes_[where_[p]].product; }
  void set(int p, const S& value) {
    assert(0 <= p && p < size());
    int id = where_[p]; nodes_[id].product = value;
    while ((id = nodes_[id].parent) != -1)
      nodes_[id].product = op(nodes_[nodes_[id].left].product, nodes_[nodes_[id].right].product);
  }
  S prod(Coord xl, Coord yl, Coord xr, Coord yr) const {
    assert(!(xr < xl) && !(yr < yl));
    if (nodes_.empty() || !(xl < xr) || !(yl < yr)) return e();
    return query(0, xl, yl, xr, yr);
  }
  S all_prod() const { return nodes_.empty() ? e() : nodes_[0].product; }
};
}  // namespace blueberry

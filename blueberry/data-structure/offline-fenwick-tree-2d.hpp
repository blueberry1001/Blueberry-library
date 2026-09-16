#pragma once

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

namespace blueberry {

// Register all possible update positions once, then add weights and query
// arbitrary half-open rectangles. Coordinates need only comparison, no +/-1.
template <class T, class Coord = long long>
class OfflineFenwickTree2D {
 public:
  explicit OfflineFenwickTree2D(std::vector<std::pair<Coord, Coord>> points)
      : points_(std::move(points)) {
    std::sort(points_.begin(), points_.end());
    points_.erase(std::unique(points_.begin(), points_.end()), points_.end());
    // Erase does not release capacity: do not retain O(P) storage for duplicates.
    std::vector<std::pair<Coord, Coord>>(points_).swap(points_);
    for (const auto& [x, y] : points_) {
      (void)y;
      if (xs_.empty() || xs_.back() != x) xs_.push_back(x);
    }
    const int n = static_cast<int>(xs_.size());
    ys_.resize(n + 1);
    data_.resize(n + 1);
    for (const auto& [x, y] : points_) {
      for (int i = x_index(x) + 1; i <= n; i += i & -i) ys_[i].push_back(y);
    }
    for (int i = 1; i <= n; ++i) {
      auto& ys = ys_[i];
      std::sort(ys.begin(), ys.end());
      ys.erase(std::unique(ys.begin(), ys.end()), ys.end());
      data_[i].resize(ys.size() + 1);
    }
  }

  int size() const { return static_cast<int>(points_.size()); }

  void add(Coord x, Coord y, const T& value) {
    assert(std::binary_search(points_.begin(), points_.end(), std::make_pair(x, y)));
    const int n = static_cast<int>(xs_.size());
    for (int i = x_index(x) + 1; i <= n; i += i & -i) {
      const int m = static_cast<int>(ys_[i].size());
      for (int j = y_index(i, y) + 1; j <= m; j += j & -j) data_[i][j] += value;
    }
  }

  // Sum over X < x and Y < y.
  T pref(Coord x, Coord y) const {
    T result{};
    for (int i = x_index(x); i > 0; i -= i & -i)
      for (int j = y_index(i, y); j > 0; j -= j & -j) result += data_[i][j];
    return result;
  }

  // Sum over [left, right) x [down, up).
  T sum(Coord left, Coord down, Coord right, Coord up) const {
    assert(!(right < left) && !(up < down));
    if (left == right || down == up) return T{};
    return pref(right, up) - pref(left, up) - pref(right, down) + pref(left, down);
  }

 private:
  int x_index(const Coord& x) const {
    return static_cast<int>(std::lower_bound(xs_.begin(), xs_.end(), x) - xs_.begin());
  }
  int y_index(int i, const Coord& y) const {
    return static_cast<int>(std::lower_bound(ys_[i].begin(), ys_[i].end(), y) - ys_[i].begin());
  }

  std::vector<std::pair<Coord, Coord>> points_;
  std::vector<Coord> xs_;
  std::vector<std::vector<Coord>> ys_;
  std::vector<std::vector<T>> data_;
};

}  // namespace blueberry

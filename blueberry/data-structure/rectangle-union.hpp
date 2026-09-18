#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <vector>

namespace blueberry {

// Rectangle coordinates are {left, bottom, right, top}.
template <class Area = long long, class Coord>
Area rectangle_union_area(const std::vector<std::array<Coord, 4>>& rectangles) {
  assert(rectangles.size() <= static_cast<unsigned>(std::numeric_limits<int>::max() / 8));
  struct Event { Coord x, bottom, top; int delta; };
  std::vector<Event> events;
  std::vector<Coord> ys;
  for (const auto& a : rectangles) {
    assert(a[0] <= a[2] && a[1] <= a[3]);
    if (a[0] == a[2] || a[1] == a[3]) continue;
    events.push_back({a[0], a[1], a[3], 1});
    events.push_back({a[2], a[1], a[3], -1});
    ys.push_back(a[1]);
    ys.push_back(a[3]);
  }
  if (events.empty()) return Area{};
  std::sort(ys.begin(), ys.end());
  ys.erase(std::unique(ys.begin(), ys.end()), ys.end());
  std::sort(events.begin(), events.end(), [](const auto& a, const auto& b) { return a.x < b.x; });
  const int n = static_cast<int>(ys.size()) - 1;
  std::vector<int> cover(4 * n);
  std::vector<Area> length(4 * n);
  auto update = [&](auto&& self, int p, int l, int r, int ql, int qr, int delta) -> void {
    if (qr <= l || r <= ql) return;
    if (ql <= l && r <= qr) cover[p] += delta;
    else {
      int m = l + (r - l) / 2;
      self(self, 2 * p, l, m, ql, qr, delta);
      self(self, 2 * p + 1, m, r, ql, qr, delta);
    }
    length[p] = cover[p] ? Area(ys[r]) - Area(ys[l])
                         : r - l == 1 ? Area{} : length[2 * p] + length[2 * p + 1];
  };
  Area answer{};
  Coord last = events.front().x;
  for (const auto& e : events) {
    answer += (Area(e.x) - Area(last)) * length[1];
    int l = static_cast<int>(std::lower_bound(ys.begin(), ys.end(), e.bottom) - ys.begin());
    int r = static_cast<int>(std::lower_bound(ys.begin(), ys.end(), e.top) - ys.begin());
    update(update, 1, 0, n, l, r, e.delta);
    last = e.x;
  }
  return answer;
}

}  // namespace blueberry

#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

namespace blueberry {

// Strict convex hull, counterclockwise from the lexicographically smallest point.
// Signed integral coordinates must satisfy |coordinate| <= 2^62 - 1.
template <class Coord>
std::vector<std::pair<Coord, Coord>> convex_hull(
    std::vector<std::pair<Coord, Coord>> points) {
  static_assert(std::is_integral_v<Coord> && std::is_signed_v<Coord> &&
                    sizeof(Coord) <= sizeof(std::int64_t),
                "convex_hull requires signed integral coordinates up to 64 bits");
#ifndef NDEBUG
  constexpr __int128 limit = (__int128{1} << 62) - 1;
  for (const auto& [x, y] : points) {
    assert(-limit <= static_cast<__int128>(x) && static_cast<__int128>(x) <= limit);
    assert(-limit <= static_cast<__int128>(y) && static_cast<__int128>(y) <= limit);
  }
#endif
  std::sort(points.begin(), points.end());
  points.erase(std::unique(points.begin(), points.end()), points.end());
  if (points.size() <= 2) return points;

  using Point = std::pair<Coord, Coord>;
  const auto turn = [](const Point& a, const Point& b, const Point& c) {
    const __int128 ab_x = static_cast<__int128>(b.first) - a.first;
    const __int128 ab_y = static_cast<__int128>(b.second) - a.second;
    const __int128 ac_x = static_cast<__int128>(c.first) - a.first;
    const __int128 ac_y = static_cast<__int128>(c.second) - a.second;
    return ab_x * ac_y - ab_y * ac_x;
  };
  std::vector<Point> hull;
  hull.reserve(points.size() + 1);
  for (const Point& p : points) {
    while (hull.size() >= 2 && turn(hull[hull.size() - 2], hull.back(), p) <= 0)
      hull.pop_back();
    hull.push_back(p);
  }
  const auto lower_size = hull.size();
  for (auto it = points.rbegin() + 1; it != points.rend(); ++it) {
    while (hull.size() > lower_size &&
           turn(hull[hull.size() - 2], hull.back(), *it) <= 0)
      hull.pop_back();
    hull.push_back(*it);
  }
  hull.pop_back();
  return hull;
}

}  // namespace blueberry

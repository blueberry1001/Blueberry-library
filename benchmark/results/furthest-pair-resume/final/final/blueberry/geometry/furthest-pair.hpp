#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

#include "blueberry/geometry/convex-hull.hpp"

namespace blueberry {

// Original indices i < j of any farthest pair; {-1, -1} for fewer than two points.
// Signed integral coordinates satisfy |coordinate| <= 2^62 - 1, N <= INT_MAX.
template <class Coord>
std::pair<int, int> furthest_pair(
    const std::vector<std::pair<Coord, Coord>>& points) {
  static_assert(std::is_integral_v<Coord> && std::is_signed_v<Coord> &&
                    sizeof(Coord) <= sizeof(std::int64_t),
                "furthest_pair requires signed integral coordinates up to 64 bits");
  assert(points.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
  if (points.size() <= 2 ||
      std::all_of(points.begin() + 1, points.end(),
                  [&](const auto& point) { return point == points[0]; })) {
#ifndef NDEBUG
    constexpr __int128 limit = (__int128{1} << 62) - 1;
    for (const auto& [x, y] : points) {
      assert(-limit <= static_cast<__int128>(x) && static_cast<__int128>(x) <= limit);
      assert(-limit <= static_cast<__int128>(y) && static_cast<__int128>(y) <= limit);
    }
#endif
    return points.size() < 2 ? std::pair<int, int>{-1, -1} : std::pair<int, int>{0, 1};
  }
  const auto hull = convex_hull(points);

  std::size_t a = 0, b = 1;
  if (hull.size() > 2) {
    const auto next = [&](std::size_t i) { return i + 1 == hull.size() ? 0 : i + 1; };
    __int128 best = -1;
    const auto consider = [&](std::size_t i, std::size_t j) {
      const __int128 dx = static_cast<__int128>(hull[i].first) - hull[j].first;
      const __int128 dy = static_cast<__int128>(hull[i].second) - hull[j].second;
      const __int128 distance = dx * dx + dy * dy;
      if (distance > best) {
        best = distance;
        a = i;
        b = j;
      }
    };
    std::size_t j = 1;
    for (std::size_t i = 0; i < hull.size(); ++i) {
      const auto ni = next(i);
      const __int128 ex = static_cast<__int128>(hull[ni].first) - hull[i].first;
      const __int128 ey = static_cast<__int128>(hull[ni].second) - hull[i].second;
      const auto change = [&](std::size_t k) {
        const auto nk = next(k);
        const __int128 dx = static_cast<__int128>(hull[nk].first) - hull[k].first;
        const __int128 dy = static_cast<__int128>(hull[nk].second) - hull[k].second;
        return ex * dy - ey * dx;
      };
      // The opposite support vertex advances monotonically over all hull edges.
      while (change(j) > 0) j = next(j);
      consider(i, j);
      consider(ni, j);
      // Parallel supporting edges require both endpoints of the opposite edge.
      if (change(j) == 0) {
        consider(i, next(j));
        consider(ni, next(j));
      }
    }
  }

  int first = -1, second = -1;
  for (int i = 0; i < static_cast<int>(points.size()) && (first < 0 || second < 0); ++i) {
    if (first < 0 && points[i] == hull[a]) first = i;
    if (second < 0 && points[i] == hull[b]) second = i;
  }
  if (first > second) std::swap(first, second);
  return {first, second};
}

}  // namespace blueberry

#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

namespace blueberry {

// Lexicographically smallest original indices i < j of a closest pair.
// Fewer than two points: {-1, -1}. Signed integral coordinates: |c| <= 2^62 - 1.
template <class Coord>
std::pair<int, int> closest_pair(
    const std::vector<std::pair<Coord, Coord>>& points) {
  static_assert(std::is_integral_v<Coord> && std::is_signed_v<Coord> &&
                    sizeof(Coord) <= sizeof(std::int64_t),
                "closest_pair requires signed integral coordinates up to 64 bits");
  assert(points.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
#ifndef NDEBUG
  constexpr __int128 limit = (__int128{1} << 62) - 1;
  for (const auto& [x, y] : points) {
    assert(-limit <= static_cast<__int128>(x) && static_cast<__int128>(x) <= limit);
    assert(-limit <= static_cast<__int128>(y) && static_cast<__int128>(y) <= limit);
  }
#endif
  if (points.size() <= 2)
    return points.size() < 2 ? std::pair<int, int>{-1, -1} : std::pair<int, int>{0, 1};

  struct Point { Coord x, y; int id; };
  const int n = static_cast<int>(points.size());
  std::vector<Point> sorted;
  sorted.reserve(n);
  for (int i = 0; i < n; ++i) sorted.push_back({points[i].first, points[i].second, i});
  std::sort(sorted.begin(), sorted.end(), [](const Point& a, const Point& b) {
    if (a.x != b.x) return a.x < b.x;
    if (a.y != b.y) return a.y < b.y;
    return a.id < b.id;
  });

  // Handle zero distance before the strip scan: arbitrarily many duplicates
  // would otherwise defeat the positive-distance packing bound.
  std::pair<int, int> answer{n, n};
  for (int i = 1; i < n; ++i)
    if (sorted[i - 1].x == sorted[i].x && sorted[i - 1].y == sorted[i].y)
      answer = std::min(answer, std::pair<int, int>{sorted[i - 1].id, sorted[i].id});
  if (answer.first != n) return answer;

  const auto distance = [](const Point& a, const Point& b) {
    const __int128 dx = static_cast<__int128>(a.x) - b.x;
    const __int128 dy = static_cast<__int128>(a.y) - b.y;
    return dx * dx + dy * dy;
  };
  __int128 best = distance(sorted[0], sorted[1]);
  answer = std::minmax(sorted[0].id, sorted[1].id);
  const auto consider = [&](const Point& a, const Point& b) {
    const __int128 candidate = distance(a, b);
    const std::pair<int, int> ids = std::minmax(a.id, b.id);
    if (candidate < best || (candidate == best && ids < answer)) {
      best = candidate;
      answer = ids;
    }
  };
  const auto by_y = [](const Point& a, const Point& b) {
    if (a.y != b.y) return a.y < b.y;
    if (a.x != b.x) return a.x < b.x;
    return a.id < b.id;
  };
  std::vector<Point> scratch(n);
  const auto solve = [&](auto&& self, int left, int right) -> void {
    if (right - left <= 3) {
      for (int i = left; i < right; ++i)
        for (int j = i + 1; j < right; ++j) consider(sorted[i], sorted[j]);
      std::sort(sorted.begin() + left, sorted.begin() + right, by_y);
      return;
    }
    const int middle = left + (right - left) / 2;
    // Save the x split before recursive calls rearrange both halves by y.
    const Coord split = sorted[middle].x;
    self(self, left, middle);
    self(self, middle, right);
    std::merge(sorted.begin() + left, sorted.begin() + middle,
               sorted.begin() + middle, sorted.begin() + right,
               scratch.begin() + left, by_y);
    std::copy(scratch.begin() + left, scratch.begin() + right, sorted.begin() + left);

    // Fix D = best here. Each half has pairwise distance >= sqrt(D), and all
    // retained points lie within sqrt(D) of the split. The y window never grows
    // beyond sqrt(D), so packing bounds comparisons per point by a constant,
    // even when best shrinks and older strip entries remain. Include equality
    // in both bounds to preserve lexicographic index ties.
    int count = 0;
    for (int i = left; i < right; ++i) {
      const Point& point = sorted[i];
      const __int128 dx = static_cast<__int128>(point.x) - split;
      if (dx * dx > best) continue;
      for (int j = count - 1; j >= 0; --j) {
        const Point& other = scratch[left + j];
        const __int128 dy = static_cast<__int128>(point.y) - other.y;
        if (dy * dy > best) break;
        consider(point, other);
      }
      scratch[left + count++] = point;
    }
  };
  solve(solve, 0, n);
  return answer;
}

}  // namespace blueberry

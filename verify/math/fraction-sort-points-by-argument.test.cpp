#define PROBLEM "https://judge.yosupo.jp/problem/sort_points_by_argument"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

#include "blueberry/math/fraction.hpp"

using namespace std;

struct Point {
  int64_t x, y;
  int sector;
  blueberry::Fraction cotangent;
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  int n;
  cin >> n;
  vector<Point> points;
  points.reserve(n);
  for (int i = 0; i < n; ++i) {
    int64_t x, y;
    cin >> x >> y;
    // Argument order: lower half, nonnegative x axis (including origin),
    // upper half, negative x axis. cot(theta) decreases on each half.
    const int sector = y < 0 ? 0 : y > 0 ? 2 : x < 0 ? 3 : 1;
    points.push_back({x, y, sector, y ? blueberry::Fraction(x, y) : blueberry::Fraction()});
  }
  sort(points.begin(), points.end(), [](const Point& a, const Point& b) {
    if (a.sector != b.sector) return a.sector < b.sector;
    return a.cotangent > b.cotangent;
  });
  for (const auto& p : points) cout << p.x << ' ' << p.y << '\n';
}

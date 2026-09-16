#include "blueberry/data-structure/offline-fenwick-tree-2d.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <random>

// Compares independent point enumeration with coordinate-compressed rectangle sums.
int main(int argc, char** argv) {
  const auto seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  std::mt19937_64 rng(seed);
  using Tree = blueberry::OfflineFenwickTree2D<long long>;
  Tree empty({});
  assert(empty.size() == 0 && empty.sum(-1, -1, 1, 1) == 0);
  const auto low = std::numeric_limits<long long>::lowest();
  const auto high = std::numeric_limits<long long>::max();
  Tree extreme({{low, low}, {high, high}, {low, low}});
  assert(extreme.size() == 2);
  extreme.add(low, low, 7);
  extreme.add(high, high, 11);
  assert(extreme.pref(high, high) == 7);
  assert(extreme.sum(low, low, high, high) == 7);
  for (int trial = 0; trial < 80; ++trial) {
    std::vector<std::pair<long long, long long>> points;
    for (int i = 0; i < 70; ++i)
      points.emplace_back(static_cast<int>(rng() % 25) - 12, static_cast<int>(rng() % 25) - 12);
    Tree tree(points);
    std::map<std::pair<long long, long long>, long long> weights;
    for (auto p : points) weights[p] = 0;
    assert(tree.size() == static_cast<int>(weights.size()));
    for (int step = 0; step < 400; ++step) {
      auto [x, y] = points[rng() % points.size()];
      long long delta = static_cast<int>(rng() % 101) - 50;
      tree.add(x, y, delta);
      weights[{x, y}] += delta;
      long long l = static_cast<int>(rng() % 41) - 20, r = static_cast<int>(rng() % 41) - 20;
      long long d = static_cast<int>(rng() % 41) - 20, u = static_cast<int>(rng() % 41) - 20;
      if (l > r) std::swap(l, r);
      if (d > u) std::swap(d, u);
      long long expected = 0, prefix = 0;
      for (auto [p, w] : weights) {
        if (l <= p.first && p.first < r && d <= p.second && p.second < u) expected += w;
        if (p.first < r && p.second < u) prefix += w;
      }
      if (tree.sum(l, d, r, u) != expected || tree.pref(r, u) != prefix) {
        std::cerr << "seed=" << seed << " trial=" << trial << " step=" << step
                  << " rectangle=" << l << ',' << d << ',' << r << ',' << u
                  << " expected=" << expected << " actual=" << tree.sum(l, d, r, u) << '\n';
        for (auto [p, w] : weights) std::cerr << p.first << ' ' << p.second << ' ' << w << '\n';
        return 1;
      }
    }
    Tree copy = tree;
    const auto [x, y] = points.front();
    copy.add(x, y, 1);
    assert(copy.sum(-30, -30, 30, 30) == tree.sum(-30, -30, 30, 30) + 1);
  }
}

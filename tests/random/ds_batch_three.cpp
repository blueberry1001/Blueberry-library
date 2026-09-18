#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <random>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include "blueberry/data-structure/persistent-queue.hpp"
#include "blueberry/data-structure/static-range-distinct.hpp"
#include "blueberry/data-structure/rectangle-union.hpp"
int main(int argc, char** argv) {
  unsigned seed = argc > 1 ? std::strtoul(argv[1], nullptr, 10) : 1;
  std::mt19937 rng(seed);
  auto check = [&](bool ok, const char* label, int trial) {
    if (!ok) { std::cerr << "seed=" << seed << " trial=" << trial << " operation=" << label << " expected=equal actual=different (replay with this seed)\n"; std::abort(); }
  };
  auto equal = [&](long long actual, long long expected, const char* label, int trial) {
    if (actual != expected) {
      std::cerr << "expected=" << expected << " actual=" << actual << '\n';
      check(false, label, trial);
    }
  };
  blueberry::PersistentQueue<int> queue;
  std::vector<std::deque<int>> oracle(1);
  for (int i = 0; i < 10000; ++i) {
    int v = rng() % oracle.size();
    auto a = oracle[v];
    int version;
    if (a.empty() || rng() % 2) {
      int x = rng() % 1000; a.push_back(x); version = queue.push(v, x);
    } else { a.pop_front(); version = queue.pop(v); }
    oracle.push_back(a);
    check(version == i + 1, "queue version", i);
    for (int j : {v, version}) {
      equal(queue.size(j), oracle[j].size(), "queue size", i);
      check(queue.empty(j) == oracle[j].empty(), "queue empty", i);
      if (!oracle[j].empty()) equal(queue.front(j), oracle[j].front(), "queue front", i);
    }
  }
  int v = 0;
  for (int i = 0; i < 10000; ++i) v = queue.push(v, i);
  for (int i = 0; i < 10000; ++i) { check(queue.front(v) == i, "queue long chain", i); v = queue.pop(v); }
  check(queue.empty(v), "queue empty chain", 0);
  auto queue_copy = queue;
  auto queue_moved = std::move(queue_copy);
  queue_copy = queue_moved;
  queue = std::move(queue_copy);
  check(queue.empty(v), "queue copy/move", 0);
  blueberry::PersistentQueue<std::string> text;
  int tv = text.push(0, "value");
  check(text.front(tv) == "value", "queue nontrivial value", 0);
  blueberry::StaticRangeDistinct<int> empty;
  check(empty.count(0, 0) == 0, "distinct default", 0);
  for (int trial = 0; trial < 250; ++trial) {
    int n = rng() % 45;
    std::vector<int> a(n);
    for (int& x : a) x = int(rng() % 15) - 7;
    blueberry::StaticRangeDistinct<int> distinct(a);
    auto copied = distinct;
    auto moved = std::move(copied);
    copied = moved;
    distinct = std::move(copied);
    for (int l = 0; l <= n; ++l) for (int r = l; r <= n; ++r) {
      int actual = distinct.count(l, r);
      int expected = static_cast<int>(std::set<int>(a.begin() + l, a.begin() + r).size());
      if (actual != expected) {
        std::cerr << "l=" << l << " r=" << r << " array=";
        for (int x : a) std::cerr << x << ',';
        std::cerr << '\n';
      }
      equal(actual, expected, "distinct", trial);
    }
    std::vector<std::array<int, 4>> rects;
    std::set<std::pair<int, int>> covered;
    for (int i = 0; i < n; ++i) {
      int l = int(rng() % 15) - 7, r = int(rng() % 15) - 7;
      int b = int(rng() % 15) - 7, t = int(rng() % 15) - 7;
      if (l > r) std::swap(l, r);
      if (b > t) std::swap(b, t);
      rects.push_back({l, b, r, t});
      for (int x = l; x < r; ++x) for (int y = b; y < t; ++y) covered.emplace(x, y);
    }
    auto area = blueberry::rectangle_union_area(rects);
    if (area != static_cast<long long>(covered.size()))
      for (const auto& rect : rects) { for (int x : rect) std::cerr << x << ' '; std::cerr << '\n'; }
    equal(area, covered.size(), "rectangle union", trial);
  }
  std::vector<std::array<long long, 4>> wide{{-4000000000LL, -1, 4000000000LL, 1}};
  check(blueberry::rectangle_union_area(wide) == 16000000000LL, "wide coordinates", 0);
}

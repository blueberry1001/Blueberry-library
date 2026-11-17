#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <limits>
#include <random>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include "blueberry/data-structure/persistent-queue.hpp"
#include "blueberry/data-structure/static-range-distinct.hpp"
#include "blueberry/data-structure/rectangle-union.hpp"

struct DistinctKey {
  int value, tag;
  friend bool operator<(const DistinctKey& a, const DistinctKey& b) { return a.value < b.value; }
};

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
  {
    std::vector<bool> a{true, false, true, false};
    blueberry::StaticRangeDistinct<bool> d(a);
    equal(d.count(0, 4), 2, "distinct bool", 0);
    equal(d.count(1, 2), 1, "distinct bool single", 0);
  }
  {
    std::vector<DistinctKey> a{{1, 0}, {1, 1}, {2, 2}, {1, 3}};
    blueberry::StaticRangeDistinct<DistinctKey> d(a);
    equal(d.count(1, 4), 2, "distinct ordering equivalence", 0);
    equal(d.count(0, 2), 1, "distinct ordering only", 0);
  }
  for (int mode = 0; mode < 4; ++mode) {
    int n = 20000, window = n / 8;
    std::vector<std::int64_t> a(n);
    for (int i = 0; i < n; ++i) a[i] = i < 2048 ? i : rng() % 2048;
    int at = mode == 0 ? 2048 : mode == 1 ? window - 1 : mode == 2 ? window : n - 1;
    a[at] = std::numeric_limits<std::int64_t>::min();
    blueberry::StaticRangeDistinct<std::int64_t> d(a);
    std::vector<std::int32_t> b(n);
    std::vector<std::int16_t> c(n);
    for (int i = 0; i < n; ++i) {
      b[i] = i == at ? std::numeric_limits<std::int32_t>::min() : static_cast<std::int32_t>(a[i]);
      c[i] = i == at ? std::numeric_limits<std::int16_t>::min() : static_cast<std::int16_t>(a[i]);
    }
    blueberry::StaticRangeDistinct<std::int32_t> d32(b);
    blueberry::StaticRangeDistinct<std::int16_t> d16(c);
    for (int q = 0; q < 50; ++q) {
      int l = rng() % (n + 1), r = rng() % (n + 1);
      if (l > r) std::swap(l, r);
      equal(d.count(l, r), std::set<std::int64_t>(a.begin() + l, a.begin() + r).size(), "distinct radix boundary", mode);
      equal(d32.count(l, r), std::set<std::int32_t>(b.begin() + l, b.begin() + r).size(), "distinct radix 32", mode);
      equal(d16.count(l, r), std::set<std::int16_t>(c.begin() + l, c.begin() + r).size(), "distinct radix 16", mode);
    }
  }
  {
    std::vector<std::uint64_t> a(20000);
    for (auto& x : a) x = (std::uint64_t(rng()) << 32) | rng();
    a[0] = 0; a[1] = std::numeric_limits<std::uint64_t>::max(); a[10000] = a[1];
    blueberry::StaticRangeDistinct<std::uint64_t> d(a);
    for (int q = 0; q < 50; ++q) {
      int l = rng() % (a.size() + 1), r = rng() % (a.size() + 1);
      if (l > r) std::swap(l, r);
      equal(d.count(l, r), std::set<std::uint64_t>(a.begin() + l, a.begin() + r).size(), "distinct unsigned radix", q);
    }
  }
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

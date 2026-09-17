#include "blueberry/data-structure/dynamic-fenwick-tree.hpp"
#include "blueberry/data-structure/dynamic-fenwick-tree-2d.hpp"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <random>
#include <utility>
#include <vector>
#include <atcoder/modint>

// Independent enumeration checks online updates, sparse domains and value ownership.
int main(int argc, char** argv) {
  const auto seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  std::mt19937_64 rng(seed);
  using One = blueberry::DynamicFenwickTree<long long>;
  using Two = blueberry::DynamicFenwickTree2D<long long>;
  One empty;
  Two empty2;
  assert(empty.size() == 0 && empty.pref(0) == 0 && empty.lower_bound(1) == 0);
  assert(empty2.height() == 0 && empty2.width() == 0 && empty2.sum(0, 0, 0, 0) == 0);
  Two flat(0, 7), flat2(9, 0);
  assert(flat.pref(0, 7) == 0 && flat2.pref(9, 0) == 0);
  using Mint = atcoder::modint998244353;
  blueberry::DynamicFenwickTree<Mint, int> modular(8);
  blueberry::DynamicFenwickTree2D<Mint, int> modular2(8, 9);
  modular.add(7, -3); modular2.add(7, 8, -3);
  assert(modular.get(7) == Mint(-3) && modular2.sum(0, 0, 8, 9) == Mint(-3));
  const long long high = std::numeric_limits<long long>::max();
  One huge(high);
  Two huge2(high, high);
  huge.add(high - 1, 5);
  huge.add(0, 2);
  huge2.add(high - 1, high - 1, 5);
  huge2.add(0, 0, -2);
  assert(huge.pref(high) == 7 && huge.get(high - 1) == 5);
  assert(huge.lower_bound(3) == high - 1 && huge.lower_bound(8) == high);
  assert(huge2.pref(high, high) == 3 && huge2.get(high - 1, high - 1) == 5);
  for (int trial = 0; trial < 35; ++trial) {
    long long n = trial % 2 ? 37 : high, m = trial % 2 ? 43 : high;
    One one(n);
    Two two(n, m);
    std::map<long long, long long> a;
    std::map<std::pair<long long, long long>, long long> b;
    for (int step = 0; step < 300; ++step) {
      long long x = static_cast<long long>(rng() % n), y = static_cast<long long>(rng() % m);
      long long delta = static_cast<int>(rng() % 101) - 50;
      one.add(x, delta); a[x] += delta;
      two.add(x, y, delta); b[{x, y}] += delta;
      long long l = static_cast<long long>(rng() % n), r = static_cast<long long>(rng() % n);
      long long d = static_cast<long long>(rng() % m), u = static_cast<long long>(rng() % m);
      if (l > r) std::swap(l, r);
      if (d > u) std::swap(d, u);
      long long expected1 = 0, expected2 = 0;
      for (auto [p, w] : a) if (l <= p && p < r) expected1 += w;
      for (auto [p, w] : b)
        if (l <= p.first && p.first < r && d <= p.second && p.second < u) expected2 += w;
      if (one.sum(l, r) != expected1 || two.sum(l, d, r, u) != expected2) {
        std::cerr << "seed=" << seed << " trial=" << trial << " step=" << step
                  << " rectangle=" << l << ',' << d << ',' << r << ',' << u
                  << " expected=" << expected1 << ',' << expected2
                  << " actual=" << one.sum(l, r) << ',' << two.sum(l, d, r, u) << '\n';
        for (auto [p, w] : b) std::cerr << p.first << ' ' << p.second << ' ' << w << '\n';
        return 1;
      }
      assert(one.get(x) == a[x] && two.get(x, y) == b[std::make_pair(x, y)]);
      assert(two.sum(l, d, l, u) == 0 && two.sum(l, d, r, d) == 0);
    }
    One copy = one, assigned(1); assigned = one;
    Two copy2 = two, assigned2(1, 1); assigned2 = two;
    copy.add(0, 7); copy2.add(0, 0, 7);
    assert(copy.pref(n) == one.pref(n) + 7 && copy2.pref(n, m) == two.pref(n, m) + 7);
    One moved(std::move(assigned)); Two moved2(std::move(assigned2));
    assert(assigned.size() == 0 && assigned.pref(0) == 0);
    assert(assigned2.height() == 0 && assigned2.width() == 0 && assigned2.pref(0, 0) == 0);
    assigned = std::move(moved); assigned2 = std::move(moved2);
    assert(assigned.pref(n) == one.pref(n) && assigned2.pref(n, m) == two.pref(n, m));
    assert(moved.size() == 0 && moved2.height() == 0);
    auto* self = &assigned; auto* self2 = &assigned2;
    assigned = *self; assigned2 = *self2;
    assigned = std::move(*self); assigned2 = std::move(*self2);
    assert(assigned.pref(n) == one.pref(n) && assigned2.pref(n, m) == two.pref(n, m));
  }
  // Sorted insertion exercises AVL balancing; weighted search includes zero gaps.
  Two sorted(1, 10000);
  for (int i = 0; i < 10000; ++i) sorted.add(0, i, 1);
  assert(sorted.sum(0, 17, 1, 9999) == 9982);
  One weights(80);
  assert(weights.lower_bound(1) == 80 && weights.lower_bound(0) == 0);
  std::vector<long long> a(80);
  for (int i = 0; i < 80; ++i) { a[i] = rng() % 5; weights.add(i, a[i]); }
  for (long long w = -1; w < 400; ++w) {
    long long expected = 0, total = 0;
    if (w > 0) { while (expected < 80 && total + a[expected] < w) total += a[expected++]; }
    assert(weights.lower_bound(w) == expected);
  }
}

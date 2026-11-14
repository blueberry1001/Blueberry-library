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

// Insertion and rehash must not require assigning a stored value.
struct CopyConstructibleValue {
  long long value = 0;
  CopyConstructibleValue() = default;
  explicit CopyConstructibleValue(long long x) : value(x) {}
  CopyConstructibleValue(const CopyConstructibleValue&) = default;
  CopyConstructibleValue& operator=(const CopyConstructibleValue&) = delete;
  CopyConstructibleValue& operator+=(const CopyConstructibleValue& other) {
    value += other.value;
    return *this;
  }
  friend CopyConstructibleValue operator-(const CopyConstructibleValue& a,
                                          const CopyConstructibleValue& b) {
    return CopyConstructibleValue(a.value - b.value);
  }
};

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
  // Fenwick cells can remain representable even when a point value or an
  // AVL subtree sum is outside T's range. Wider internal aggregates are needed.
  One extremes(3);
  extremes.add(0, std::numeric_limits<long long>::min());
  extremes.add(1, high);
  extremes.add(1, 1);
  assert(extremes.get(0) == std::numeric_limits<long long>::min());
  assert(extremes.pref(2) == 0 && extremes.sum(0, 2) == 0);
  extremes.add(1, -1);
  assert(extremes.get(1) == high && extremes.pref(2) == -1);
  One large_weights(7);
  large_weights.add(1, high);
  large_weights.add(4, high);
  large_weights.add(6, high);
  assert(large_weights.lower_bound(1) == 1 && large_weights.lower_bound(high) == 1);
  assert(large_weights.get(4) == high && large_weights.sum(4, 5) == high);
  blueberry::DynamicFenwickTree<unsigned, int> unsigned_values(9);
  unsigned_values.add(2, 7); unsigned_values.add(8, 4);
  assert(unsigned_values.sum(2, 9) == 11 && unsigned_values.lower_bound(8) == 8);
  blueberry::DynamicFenwickTree<CopyConstructibleValue, int> copy_only(1024);
  for (int i = 0; i < 1024; ++i) copy_only.add(i, CopyConstructibleValue(i));
  assert(copy_only.sum(123, 456).value == (123 + 455) * 333 / 2);
  auto copy_only_copy = copy_only;
  blueberry::DynamicFenwickTree<CopyConstructibleValue, int> copy_only_assigned;
  copy_only_assigned = copy_only_copy;
  auto copy_only_moved = std::move(copy_only_assigned);
  assert(copy_only_moved.get(345).value == 345 && copy_only_assigned.size() == 0);
  // Concentrated prefix-sum coordinates in a huge domain exercise rehash and
  // short ranges with long common Fenwick paths.
  constexpr long long offset = 1500000000000000000LL;
  One concentrated(offset * 2);
  std::map<long long, long long> concentrated_values;
  long long position = offset;
  for (int i = 0; i < 2000; ++i) {
    position += static_cast<int>(rng() % 2001) - 1000;
    const long long delta = static_cast<int>(rng() % 11) - 5;
    concentrated.add(position, delta);
    concentrated_values[position] += delta;
    long long left = position - static_cast<long long>(rng() % 5000);
    long long right = position + static_cast<long long>(rng() % 5000);
    long long expected = 0;
    for (auto it = concentrated_values.lower_bound(left);
         it != concentrated_values.end() && it->first < right; ++it)
      expected += it->second;
    if (concentrated.sum(left, right) != expected) {
      std::cerr << "seed=" << seed << " step=" << i << " range=" << left << ','
                << right << " expected=" << expected
                << " actual=" << concentrated.sum(left, right) << '\n';
      return 1;
    }
  }
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

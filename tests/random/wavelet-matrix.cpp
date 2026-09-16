#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <optional>
#include <random>
#include <vector>
#include "blueberry/data-structure/wavelet-matrix.hpp"

using namespace std;

template <class T>
void check(const vector<T>& a, mt19937_64& rng, unsigned long long seed) {
  const int n = static_cast<int>(a.size());
  const blueberry::WaveletMatrix<T> wm(a);
  assert(wm.size() == n);
  for (int i = 0; i < n; ++i) assert(wm.get(i) == a[i]);
  for (int trial = 0; trial < 200; ++trial) {
    int l = static_cast<int>(rng() % (n + 1)), r = static_cast<int>(rng() % (n + 1));
    if (l > r) swap(l, r);
    vector<T> sorted(a.begin() + l, a.begin() + r);
    sort(sorted.begin(), sorted.end());
    T x = static_cast<T>(rng() % 41 - 20);
    if (trial % 4 == 0) x = numeric_limits<T>::min();
    if (trial % 4 == 1) x = numeric_limits<T>::max();
    if (trial % 4 == 2 && n) x = a[rng() % n];
    T y = static_cast<T>(rng() % 41 - 20);
    if (y < x) swap(x, y);
    const int less = static_cast<int>(lower_bound(sorted.begin(), sorted.end(), x) - sorted.begin());
    const int freq = static_cast<int>(count(sorted.begin(), sorted.end(), x));
    const int between = static_cast<int>(lower_bound(sorted.begin(), sorted.end(), y) -
                                         lower_bound(sorted.begin(), sorted.end(), x));
    const optional<T> prev = less == 0 ? nullopt : optional<T>(sorted[less - 1]);
    const optional<T> next = less == r - l ? nullopt : optional<T>(sorted[less]);
    auto require = [&](bool ok, const char* operation, auto expected, auto actual) {
      if (ok) return;
      cerr << "seed=" << seed << " operation=" << operation << " l=" << l << " r=" << r
           << " x=" << x << " y=" << y << " expected=" << expected << " actual=" << actual << " input=";
      for (T value : a) cerr << value << ',';
      cerr << '\n';
      exit(1);
    };
    require(wm.count(l, r, x) == freq, "count", freq, wm.count(l, r, x));
    require(wm.range_freq(l, r, x) == less, "less", less, wm.range_freq(l, r, x));
    require(wm.range_freq(l, r, x, y) == between, "between", between, wm.range_freq(l, r, x, y));
    require(wm.prev_value(l, r, x) == prev, "prev", prev.value_or(T{}), wm.prev_value(l, r, x).value_or(T{}));
    require(wm.next_value(l, r, x) == next, "next", next.value_or(T{}), wm.next_value(l, r, x).value_or(T{}));
    for (int k = 0; k < r - l; ++k) {
      require(wm.kth_smallest(l, r, k) == sorted[k], "kth_smallest", sorted[k], wm.kth_smallest(l, r, k));
      require(wm.kth_largest(l, r, k) == sorted[r - l - 1 - k], "kth_largest", sorted[r - l - 1 - k], wm.kth_largest(l, r, k));
    }
  }
}

int main(int argc, char** argv) {
  const auto seed = argc > 1 ? strtoull(argv[1], nullptr, 10) : 1;
  mt19937_64 rng(seed);
  blueberry::WaveletMatrix<long long> empty;
  assert(empty.size() == 0 && empty.count(0, 0, 0) == 0);
  assert(!empty.prev_value(0, 0, 0) && !empty.next_value(0, 0, 0));
  for (int n : {0, 1, 2, 3, 31, 32, 63, 64, 65, 127, 128, 129}) {
    check(vector<long long>(n, -1), rng, seed);
    vector<long long> a(n);
    for (int i = 0; i < n; ++i) a[i] = i - n / 2;
    check(a, rng, seed);
    reverse(a.begin(), a.end());
    check(a, rng, seed);
    for (auto& x : a) x = static_cast<long long>(rng() % 21) - 10;
    if (n > 1) { a[0] = numeric_limits<long long>::min(); a[1] = numeric_limits<long long>::max(); }
    check(a, rng, seed);
  }
  check(vector<unsigned long long>{0, 1, 1, numeric_limits<unsigned long long>::max()}, rng, seed);
  check(vector<int>{3, -1, 4, 1, 5}, rng, seed);
  // Copy owns its storage and remains usable after replacing the source.
  blueberry::WaveletMatrix<int> original(vector<int>{4, 2});
  auto copied = original;
  original = blueberry::WaveletMatrix<int>();
  assert(copied.get(0) == 4 && copied.kth_smallest(0, 2, 0) == 2);
}

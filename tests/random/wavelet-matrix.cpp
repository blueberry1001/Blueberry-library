#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <optional>
#include <random>
#include <string>
#include <type_traits>
#include <vector>
#include "blueberry/data-structure/wavelet-matrix.hpp"

using namespace std;

template <class T> string decimal(T value) {
  using U = make_unsigned_t<T>;
  const bool negative = is_signed_v<T> && value < 0;
  U magnitude = negative ? U(0) - U(value) : U(value);
  string result;
  do { result.push_back(char('0' + magnitude % 10)); magnitude /= 10; } while (magnitude);
  if (negative) result.push_back('-');
  reverse(result.begin(), result.end());
  return result;
}

template <class T>
void check(const vector<T>& a, mt19937_64& rng, unsigned long long seed,
           int trials = 200, bool exhaustive_ranks = true) {
  const int n = static_cast<int>(a.size());
  const blueberry::WaveletMatrix<T> wm(a);
  assert(wm.size() == n);
  for (int i = 0; i < n; ++i) assert(wm.get(i) == a[i]);
  for (int trial = 0; trial < trials; ++trial) {
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
           << " x=" << decimal(x) << " y=" << decimal(y) << " expected=" << decimal(expected)
           << " actual=" << decimal(actual) << " input=";
      for (T value : a) cerr << decimal(value) << ',';
      cerr << '\n';
      exit(1);
    };
    require(wm.count(l, r, x) == freq, "count", freq, wm.count(l, r, x));
    require(wm.range_freq(l, r, x) == less, "less", less, wm.range_freq(l, r, x));
    require(wm.range_freq(l, r, x, y) == between, "between", between, wm.range_freq(l, r, x, y));
    require(wm.prev_value(l, r, x) == prev, "prev", prev.value_or(T{}), wm.prev_value(l, r, x).value_or(T{}));
    require(wm.next_value(l, r, x) == next, "next", next.value_or(T{}), wm.next_value(l, r, x).value_or(T{}));
    for (int k = 0; k < r - l; k += exhaustive_ranks ? 1 : max(1, (r - l) / 7)) {
      require(wm.kth_smallest(l, r, k) == sorted[k], "kth_smallest", sorted[k], wm.kth_smallest(l, r, k));
      require(wm.kth_largest(l, r, k) == sorted[r - l - 1 - k], "kth_largest", sorted[r - l - 1 - k], wm.kth_largest(l, r, k));
    }
  }
}

template <class T> void radix_boundaries(mt19937_64& rng, unsigned long long seed) {
  vector<T> a(129);
  for (int i = 0; i < int(a.size()); ++i) {
    if (i % 5 == 0) a[i] = numeric_limits<T>::min();
    else if (i % 5 == 1) a[i] = numeric_limits<T>::max();
    else a[i] = T(rng());
  }
  check(a, rng, seed);
  // The full 128-bit radix path starts at N=2048; keep its query sampling bounded.
  a.resize(2051);
  for (int i = 0; i < int(a.size()); ++i) a[i] = T(rng());
  a[0] = numeric_limits<T>::min(); a[1] = numeric_limits<T>::max();
  check(a, rng, seed, 16, false);
  a.resize(129);
  for (int i = 0; i < int(a.size()); ++i) a[i] = T(i % 3);
  check(a, rng, seed);
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
  check(vector<unsigned>{0, numeric_limits<unsigned>::max(), 1, 0}, rng, seed);
  check(vector<signed char>{-128, 127, 0, -128, 1}, rng, seed);
  check(vector<short>{numeric_limits<short>::min(), numeric_limits<short>::max(), 0}, rng, seed);
  radix_boundaries<signed char>(rng, seed);
  radix_boundaries<unsigned char>(rng, seed);
  radix_boundaries<char>(rng, seed);
  radix_boundaries<wchar_t>(rng, seed);
  radix_boundaries<char8_t>(rng, seed);
  radix_boundaries<char16_t>(rng, seed);
  radix_boundaries<char32_t>(rng, seed);
  radix_boundaries<short>(rng, seed);
  radix_boundaries<unsigned short>(rng, seed);
  radix_boundaries<unsigned long long>(rng, seed);
#if defined(__SIZEOF_INT128__)
  radix_boundaries<__int128_t>(rng, seed);
  radix_boundaries<__uint128_t>(rng, seed);
#endif
  // Copy owns its storage and remains usable after replacing the source.
  blueberry::WaveletMatrix<int> original(vector<int>{4, 2});
  auto copied = original;
  original = blueberry::WaveletMatrix<int>();
  assert(copied.get(0) == 4 && copied.kth_smallest(0, 2, 0) == 2);
}

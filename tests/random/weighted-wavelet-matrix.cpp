#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>
#include "blueberry/data-structure/weighted-wavelet-matrix.hpp"
using namespace std;
struct Key {
  int value;
  explicit Key(int x) : value(x) {}
  friend bool operator<(const Key& a, const Key& b) { return a.value < b.value; }
  friend bool operator==(const Key& a, const Key& b) { return a.value == b.value; }
};
struct MoveAssignedKey {
  int value;
  explicit MoveAssignedKey(int x) : value(x) {}
  MoveAssignedKey(const MoveAssignedKey&) = default;
  MoveAssignedKey(MoveAssignedKey&&) = default;
  MoveAssignedKey& operator=(const MoveAssignedKey&) = delete;
  MoveAssignedKey& operator=(MoveAssignedKey&&) = default;
  friend bool operator<(const MoveAssignedKey& a, const MoveAssignedKey& b) { return a.value < b.value; }
  friend bool operator==(const MoveAssignedKey& a, const MoveAssignedKey& b) { return a.value == b.value; }
};
template <class Weight>
void check_narrow_overflow() {
  vector<int> values(16);
  vector<Weight> weights(16);
  for (int i = 0; i < 16; ++i) values[i] = i;
  weights[0] = -1; weights[8] = 1; weights[12] = numeric_limits<Weight>::max();
  blueberry::WeightedWaveletMatrix<int, Weight> wm(values, weights);
  assert(wm.sum(0, 16, 10, 14) == numeric_limits<Weight>::max());
}
int main(int argc, char** argv) {
  auto seed = argc > 1 ? strtoull(argv[1], nullptr, 10) : 1;
  mt19937_64 rng(seed);
  blueberry::WeightedWaveletMatrix<long long> empty;
  assert(empty.size() == 0 && empty.sum(0, 0, -1, 1) == 0);
  blueberry::WeightedWaveletMatrix<string> strings({"pear", "apple", "pear", "plum"}, {1, 2, 3, 4});
  assert(strings.sum(0, 4, string("pear"), string("plum")) == 4);
  strings.add(2, -2);
  assert(strings.sum(1, 4, string("pear"), string("plum")) == 1);
  blueberry::WeightedWaveletMatrix<Key> keys({Key(3), Key(1), Key(3)}, {2, 4, 6});
  assert(keys.sum(0, 3, Key(2), Key(4)) == 8);
  blueberry::WeightedWaveletMatrix<MoveAssignedKey> move_keys({MoveAssignedKey(3), MoveAssignedKey(1), MoveAssignedKey(3)}, {2, 4, 6});
  assert(move_keys.sum(0, 3, MoveAssignedKey(2), MoveAssignedKey(4)) == 8);
  blueberry::WeightedWaveletMatrix<bool> flags({true, false, true}, {3, 5, 7});
  assert(flags.sum(0, 3, false, true) == 5);
  check_narrow_overflow<int8_t>();
  check_narrow_overflow<int16_t>();
  vector<int> overflow_values(16), overflow_weights(16);
  for (int i = 0; i < 16; ++i) overflow_values[i] = i;
  overflow_weights[0] = -1; overflow_weights[8] = 1; overflow_weights[12] = numeric_limits<int>::max();
  blueberry::WeightedWaveletMatrix<int, int> overflow(overflow_values, overflow_weights);
  assert(overflow.sum(0, 16, 10, 14) == numeric_limits<int>::max());
  blueberry::WeightedWaveletMatrix<int, double> fractions({2, 0, 1}, {0.25, 1.5, -0.5});
  assert(fractions.sum(0, 3, 0, 2) == 1.0);
  vector<int8_t> small_values(256);
  vector<int> small_weights(256, 1);
  for (int i = 0; i < 256; ++i) small_values[i] = static_cast<int8_t>((i * 73) % 256 - 128);
  blueberry::WeightedWaveletMatrix<int8_t, int> small(small_values, small_weights);
  for (int low = -128; low < 127; low += 17) for (int high = low; high <= 127; high += 19)
    assert(small.sum(0, 256, static_cast<int8_t>(low), static_cast<int8_t>(high)) == high - low);
  for (int n : {0, 1, 2, 63, 64, 65, 128, 129, 1024, 1100}) for (int mode = 0; mode < 5; ++mode) {
    vector<long long> a(n), w(n);
    for (int i = 0; i < n; ++i) {
      a[i] = mode == 0 ? -3 : static_cast<long long>(rng() % 21) - 10;
      if (mode == 3) a[i] = static_cast<long long>(rng());
      if (mode == 4) a[i] = i;
      w[i] = static_cast<long long>(rng() % 21) - 10;
    }
    if (mode == 2 && n > 1) { a[0] = numeric_limits<long long>::min(); a[1] = numeric_limits<long long>::max(); }
    blueberry::WeightedWaveletMatrix<long long> wm(a, w);
    assert(wm.size() == n);
    for (int step = 0; step < 1000; ++step) {
      if (n && rng() % 3 == 0) {
        int p = rng() % n;
        long long d = static_cast<long long>(rng() % 21) - 10;
        wm.add(p, d); w[p] += d;
      } else {
        int l = rng() % (n + 1), r = rng() % (n + 1);
        if (l > r) swap(l, r);
        long long low = static_cast<long long>(rng() % 31) - 15, high = static_cast<long long>(rng() % 31) - 15;
        if (step % 5 == 0) low = numeric_limits<long long>::min();
        if (step % 7 == 0) high = numeric_limits<long long>::max();
        if (low > high) swap(low, high);
        long long all = 0, less = 0, between = 0;
        for (int i = l; i < r; ++i) { all += w[i]; if (a[i] < high) less += w[i]; if (low <= a[i] && a[i] < high) between += w[i]; }
        if (wm.sum(l, r) != all || wm.sum(l, r, high) != less || wm.sum(l, r, low, high) != between) {
          cerr << "seed=" << seed << " n=" << n << " step=" << step << " l=" << l << " r=" << r << " low=" << low << " high=" << high << " expected=" << between << " actual=" << wm.sum(l, r, low, high) << '\n';
          for (int i = 0; i < n; ++i) cerr << a[i] << ':' << w[i] << ' ';
          return 1;
        }
      }
    }
  }
}

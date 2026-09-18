#include <cassert>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <vector>
#include "blueberry/data-structure/weighted-wavelet-matrix.hpp"
using namespace std;
int main(int argc, char** argv) {
  auto seed = argc > 1 ? strtoull(argv[1], nullptr, 10) : 1;
  mt19937_64 rng(seed);
  blueberry::WeightedWaveletMatrix<long long> empty;
  assert(empty.size() == 0 && empty.sum(0, 0, -1, 1) == 0);
  for (int n : {0, 1, 2, 63, 64, 65, 128, 129}) for (int mode = 0; mode < 3; ++mode) {
    vector<long long> a(n), w(n);
    for (int i = 0; i < n; ++i) {
      a[i] = mode == 0 ? -3 : static_cast<long long>(rng() % 21) - 10;
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

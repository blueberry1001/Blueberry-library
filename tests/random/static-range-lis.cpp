#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>
#include "blueberry/data-structure/static-range-lis.hpp"

int main(int argc, char** argv) {
  const unsigned seed = argc > 1 ? std::stoul(argv[1]) : 1;
  std::mt19937 rng(seed);
  for (int n = 0; n <= 100; ++n) for (int trial = 0; trial < 20; ++trial) {
    std::vector<int> a(n);
    for (int i = 0; i < n; ++i)
      a[i] = trial == 0 ? i : trial == 1 ? -i : trial == 2 ? 0 : int(rng() % 17) - 8;
    blueberry::StaticRangeLIS<int> ds(a);
    for (int l = 0; l <= n; ++l) {
      std::vector<int> tails;
      for (int r = l; r <= n; ++r) {
        if (r != l) {
          auto it = std::lower_bound(tails.begin(), tails.end(), a[r - 1]);
          if (it == tails.end()) tails.push_back(a[r - 1]);
          else *it = a[r - 1];
        }
        if (ds.lis(l, r) != int(tails.size())) {
          std::cerr << "seed=" << seed << " n=" << n << " l=" << l << " r=" << r
                    << " expected=" << tails.size() << " actual=" << ds.lis(l, r) << '\n';
          for (int x : a) std::cerr << x << ' ';
          return 1;
        }
      }
    }
  }
}

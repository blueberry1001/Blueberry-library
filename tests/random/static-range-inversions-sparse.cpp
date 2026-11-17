#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "blueberry/data-structure/static-range-inversions.hpp"
int main(int argc, char** argv) {
  unsigned seed = argc > 1 ? static_cast<unsigned>(std::stoul(argv[1])) : 20261117;
  std::mt19937 rng(seed);
  for (int n = 0; n <= 80; ++n) for (int rep = 0; rep < 16; ++rep) {
    std::vector<int> a(n); for (int& x : a) x = static_cast<int>(rng() % 13) - 6;
    int q = rep % 4 == 0 ? 0 : rep % 4 == 1 ? 1 : rep % 4 == 2 ? n / 2 : n + 1;
    std::vector<std::pair<int, int>> queries(q);
    for (auto& [l, r] : queries) {
      l = rng() % (n + 1); r = rng() % (n + 1);
      if (l > r) std::swap(l, r);
      if (rep % 3 == 0) r = std::min(n, l + 2);
    }
    auto actual = blueberry::static_range_inversions(a, queries);
    for (int k = 0; k < q; ++k) {
      auto [l, r] = queries[k]; long long expected = 0;
      for (int i = l; i < r; ++i) for (int j = i + 1; j < r; ++j) expected += a[j] < a[i];
      if (actual[k] != expected) {
        std::cerr << "seed=" << seed << " n=" << n << " q=" << q << " l=" << l << " r=" << r << " expected=" << expected << " actual=" << actual[k] << " input=";
        for (int x : a) std::cerr << x << ',';
        std::cerr << '\n'; return 1;
      }
    }
  }
}

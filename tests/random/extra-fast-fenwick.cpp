#include "blueberry/extra-fast/fenwick-tree.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

using U64 = std::uint64_t;

int main(int argc, char** argv) {
  const U64 seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  std::mt19937_64 rng(seed);
  int step = 0;
  std::vector<U64> a;
  auto fail = [&](const char* operation, int l, int r, U64 expected, U64 actual) {
    std::cerr << "seed=" << seed << " step=" << step << " op=" << operation
              << " n=" << a.size() << " l=" << l << " r=" << r
              << " expected=" << expected << " actual=" << actual << " input=";
    for (U64 x : a) std::cerr << x << ',';
    std::cerr << '\n';
    std::exit(1);
  };
  for (int n : {0, 1, 2, 15, 16, 17, 255, 256, 257, 1023, 1024, 1025, 4096, 65537}) {
    a.resize(n);
    for (auto& x : a) x = rng();
    blueberry::extra_fast::FenwickTree tree(a), zero(n);
    if (tree.size() != n || zero.all_prod() != 0)
      fail("construct", 0, n, n, tree.size());
    for (step = 0; step < 200; ++step) {
      if (n) {
        const int p = rng() % n;
        const U64 x = rng();
        if (step & 1) {
          tree.add(p, x);
          a[p] += x;
        } else {
          tree.set(p, x);
          a[p] = x;
        }
      }
      int l = rng() % (n + 1), r = rng() % (n + 1);
      if (l > r) std::swap(l, r);
      U64 expected = std::accumulate(a.begin() + l, a.begin() + r, U64(0));
      if (tree.prod(l, r) != expected)
        fail("prod", l, r, expected, tree.prod(l, r));
      expected = std::accumulate(a.begin(), a.end(), U64(0));
      if (tree.all_prod() != expected)
        fail("all_prod", 0, n, expected, tree.all_prod());
      if (n) {
        const int p = rng() % n;
        if (tree.get(p) != a[p]) fail("get", p, p + 1, a[p], tree.get(p));
      }
      if (step % 20 == 0) {
        auto copy = tree;
        auto* self = &copy;
        copy = *self;
        tree = std::move(copy);
      }
    }
    // Prefix search uses nonnegative, non-wrapping sums, including zero runs.
    for (auto& x : a) x = rng() % 5;
    tree = blueberry::extra_fast::FenwickTree(a);
    std::vector<U64> pref(n + 1);
    for (int i = 0; i < n; ++i) pref[i + 1] = pref[i] + a[i];
    for (step = 0; step < 100; ++step) {
      U64 x = step == 0 ? 0 : step == 1 ? pref.back() : step == 2 ? pref.back() + 1
              : step == 3 ? ~U64(0) : rng() % (pref.back() + 2);
      const int expected = std::lower_bound(pref.begin(), pref.end(), x) - pref.begin();
      if (tree.lower_bound(x) != expected)
        fail("lower_bound", 0, n, expected, tree.lower_bound(x));
    }
  }
  // Unsigned SIMD comparisons must also work above INT64_MAX.
  a = {0, U64(1) << 63, 0, 5, 0};
  blueberry::extra_fast::FenwickTree high(a);
  if (high.lower_bound(U64(1) << 63) != 2 ||
      high.lower_bound((U64(1) << 63) + 1) != 4 ||
      high.lower_bound(~U64(0)) != 6)
    fail("unsigned-search", 0, 5, 4, high.lower_bound((U64(1) << 63) + 1));
  high.set(1, ~U64(0) - 5);
  if (high.lower_bound(~U64(0)) != 4)
    fail("max-search-after-set", 0, 5, 4, high.lower_bound(~U64(0)));
}

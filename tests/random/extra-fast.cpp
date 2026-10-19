#include "blueberry/extra-fast/segment-tree.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <utility>
#include <vector>

using Tree = blueberry::extra_fast::SumSegmentTree;
using U64 = std::uint64_t;
U64 seed;
int step;

void fail(const char* operation, const std::vector<U64>& a, int l, int r,
          U64 expected, U64 actual) {
  std::cerr << "seed=" << seed << " step=" << step << " op=" << operation
            << " n=" << a.size() << " l=" << l << " r=" << r
            << " expected=" << expected << " actual=" << actual << " input=";
  for (U64 x : a) std::cerr << x << ',';
  std::cerr << '\n';
  std::exit(1);
}

U64 sum(const std::vector<U64>& a, int l, int r) {
  U64 answer = 0;
  for (int i = l; i < r; ++i) answer += a[i];
  return answer;
}

void query(const Tree& tree, const std::vector<U64>& a, int l, int r) {
  const U64 expected = sum(a, l, r);
  if (tree.prod(l, r) != expected)
    fail("prod", a, l, r, expected, tree.prod(l, r));
  if (tree.pref(r) != sum(a, 0, r))
    fail("pref", a, 0, r, sum(a, 0, r), tree.pref(r));
  if (tree.all_prod() != sum(a, 0, static_cast<int>(a.size())))
    fail("all_prod", a, 0, a.size(), sum(a, 0, a.size()), tree.all_prod());
}

int main(int argc, char** argv) {
  seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  std::mt19937_64 rng(seed);
  for (bool use_simd : {false, true}) {
    for (int n : {0, 1, 2, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65,
                  255, 256, 257, 4095, 4096, 4097, 65535, 65536, 65537}) {
      std::vector<U64> a(n);
      for (auto& value : a) value = rng();
      Tree initialized(n, use_simd);
      if (initialized.size() != n || initialized.all_prod() != 0 ||
          initialized.prod(0, n) != 0 || initialized.pref(n) != 0)
        fail("size constructor", std::vector<U64>(n), 0, n, 0,
             initialized.all_prod());
      Tree tree(a, use_simd);
      if (tree.size() != n) fail("size", a, 0, n, n, tree.size());
      if (tree.uses_simd() && (!use_simd || !Tree::simd_available()))
        fail("dispatch", a, 0, n, 0, 1);
      for (step = 0; step < (n > 4096 ? 80 : 180); ++step) {
        if (n) {
          const int p = step % 5 == 0 ? 0 : step % 5 == 1 ? n - 1
                      : step % 5 == 2 ? n / 2 : rng() % n;
          const U64 value = step % 6 == 0 ? 0 : step % 6 == 1
              ? std::numeric_limits<U64>::max() : rng();
          if (step & 1) { a[p] += value; tree.add(p, value); }
          else { a[p] = value; tree.set(p, value); }
          if (tree.get(p) != a[p])
            fail("get after update", a, p, p + 1, a[p], tree.get(p));
        }
        int l = rng() % (n + 1), r = rng() % (n + 1);
        if (l > r) std::swap(l, r);
        query(tree, a, l, r);
        query(tree, a, l, l);
        query(tree, a, 0, n);
        if (n <= 17)
          for (l = 0; l <= n; ++l)
            for (r = l; r <= n; ++r) query(tree, a, l, r);
      }
      Tree copied(tree);
      Tree assigned(0, use_simd);
      assigned = tree;
      auto* self = &assigned;
      assigned = *self;
      Tree moved(std::move(copied));
      Tree move_assigned(0, use_simd);
      move_assigned = std::move(assigned);
      for (auto* other : {&moved, &move_assigned}) {
        query(*other, a, 0, n);
        if (n) {
          other->add(n / 2, 1);
          auto changed = a;
          ++changed[n / 2];
          query(*other, changed, 0, n);
          query(tree, a, 0, n);  // Copied storage is independent.
        }
      }
    }
  }
}

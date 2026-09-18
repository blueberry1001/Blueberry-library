#include "blueberry/data-structure/sqrt-tree.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>
constexpr long long mod = 998244353;
struct Affine {
  long long a, b;
  Affine() = delete;
  Affine(long long slope, long long intercept) : a(slope), b(intercept) {}
  bool operator==(const Affine&) const = default;
};
int calls;
Affine op(Affine f, Affine g) { ++calls; return {f.a * g.a % mod, (g.a * f.b + g.b) % mod}; }
Affine e() { return {1, 0}; }
using Tree = blueberry::SqrtTree<Affine, op, e>;
int main(int argc, char** argv) {
  auto seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  std::mt19937_64 rng(seed);
  std::vector<int> sizes{0,1,2,3,4,5,7,8,9,15,16,17,31,32,33,63,64,65,127,128,129,255,256,257,1023,1024,1025,4095,4096,4097};
  Tree empty;
  if (empty.size() || empty.all_prod() != e() || empty.prod(0,0) != e()) return 1;
  for (int n : sizes) {
    std::vector<Affine> a(n, e());
    for (auto& f : a) f = {static_cast<long long>(rng() % mod), static_cast<long long>(rng() % mod)};
    Tree tree(a);
    auto copy = tree;
    auto moved = std::move(copy);
    if (tree.size() != n || moved.size() != n) return 1;
    for (int trial = 0; trial < 600; ++trial) {
      int l = rng() % (n + 1), r = rng() % (n + 1);
      if (l > r) std::swap(l,r);
      if (trial == 0) { l = 0; r = n; }
      auto expected = e();
      for (int i = l; i < r; ++i) expected = op(expected, a[i]);
      calls = 0;
      auto actual = tree.prod(l,r);
      if (actual != expected || calls > 2 || moved.prod(l,r) != expected) {
        std::cerr << "seed=" << seed << " n=" << n << " l=" << l << " r=" << r
                  << " expected=" << expected.a << ',' << expected.b
                  << " actual=" << actual.a << ',' << actual.b << " input=";
        for (auto f : a) std::cerr << '(' << f.a << ',' << f.b << ')';
        std::cerr << '\n'; return 1;
      }
      if (l < n && tree.get(l) != a[l]) return 1;
      if (trial == 0 && tree.all_prod() != expected) return 1;
    }
  }
}

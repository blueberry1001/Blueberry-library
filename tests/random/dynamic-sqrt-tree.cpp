#include "blueberry/data-structure/dynamic-sqrt-tree.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <random>
#include <utility>
#include <vector>

constexpr long long mod = 998244353;
struct Affine {
  long long a, b;
  Affine() = delete;
  Affine(long long x, long long y) : a(x), b(y) {}
  bool operator==(const Affine&) const = default;
};
int calls;
Affine op(Affine f, Affine g) { ++calls; return {f.a * g.a % mod, (g.a * f.b + g.b) % mod}; }
Affine e() { return {1, 0}; }
using Tree = blueberry::DynamicSqrtTree<Affine, op, e>;
unsigned long long seed;
int step;
void fail(const char* why, const std::vector<Affine>& a, int l, int r, Affine expected, Affine actual) {
  std::cerr << "seed=" << seed << " step=" << step << " reason=" << why << " n=" << a.size()
            << " l=" << l << " r=" << r << " expected=" << expected.a << ',' << expected.b
            << " actual=" << actual.a << ',' << actual.b << " input=";
  for (auto f : a) std::cerr << '(' << f.a << ',' << f.b << ')';
  std::cerr << '\n'; std::exit(1);
}
void query(const Tree& tree, const std::vector<Affine>& a, int l, int r) {
  auto expected = e();
  for (int i = l; i < r; ++i) expected = op(expected, a[i]);
  calls = 0;
  auto actual = tree.prod(l, r);
  if (actual != expected || calls > 4) fail("prod/call bound", a, l, r, expected, actual);
  if (l == 0 && r == tree.size() && tree.all_prod() != expected)
    fail("all_prod", a, l, r, expected, tree.all_prod());
}
int main(int argc, char** argv) {
  seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  std::mt19937_64 rng(seed);
  auto random_value = [&] { return Affine(rng() % mod, rng() % mod); };
  Tree empty;
  if (empty.size() || empty.prod(0, 0) != e() || empty.all_prod() != e()) return 1;
  for (int n : {0,1,2,3,4,5,7,8,9,15,16,17,31,32,33,63,64,65,127,128,129,
                255,256,257,1023,1024,1025,4095,4096,4097,16384,16385,65536,65537}) {
    std::vector<Affine> a(n, e());
    for (auto& f : a) f = random_value();
    Tree tree(a);
    if (tree.size() != n) return 1;
    query(tree, a, 0, n);
    for (step = 0; step < (n > 10000 ? 60 : 200); ++step) {
      if (n) {
        const int p = step % 5 == 0 ? 0 : step % 5 == 1 ? n - 1 : step % 5 == 2 ? n / 2 : rng() % n;
        a[p] = random_value();
        calls = 0;
        tree.set(p, a[p]);
        // A naive root-between rebuild is linear and violates this at large N.
        if (calls > 32 * static_cast<int>(std::ceil(std::sqrt(n + 1))))
          fail("set call bound", a, p, p + 1, a[p], tree.get(p));
        if (tree.get(p) != a[p]) fail("get", a, p, p + 1, a[p], tree.get(p));
      }
      query(tree, a, 0, n);
      int l = rng() % (n + 1), r = rng() % (n + 1);
      if (l > r) std::swap(l, r);
      query(tree, a, l, r);
      query(tree, a, l, l);
      if (n <= 17)
        for (l = 0; l <= n; ++l)
          for (r = l; r <= n; ++r) query(tree, a, l, r);
    }
    Tree copied(tree);
    Tree assigned(std::vector<Affine>{e()});
    assigned = tree;
    auto* self = &assigned;
    assigned = *self;
    Tree moved(std::move(copied));
    if (copied.size() || copied.all_prod() != e()) return 1;
    copied = tree;
    Tree move_assigned(std::vector<Affine>{e(), e()});
    move_assigned = std::move(copied);
    if (copied.size() || copied.prod(0,0) != e()) return 1;
    self = &move_assigned;
    move_assigned = std::move(*self);
    for (auto* other : {&assigned, &moved, &move_assigned}) {
      query(*other, a, 0, n);
      if (n) {
        other->set(n / 2, e());
        query(tree, a, 0, n); // Copies/moves do not share mutable storage.
        auto changed = a; changed[n / 2] = e(); query(*other, changed, 0, n);
      }
    }
  }
}

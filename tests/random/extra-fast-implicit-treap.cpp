#include "blueberry/extra-fast/implicit-treap.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>
template<std::uint32_t Mod> void run(std::uint64_t seed) {
  using Tree = blueberry::extra_fast::ImplicitTreap<Mod>;
  using V = typename Tree::value_type;
  static_assert(std::is_same_v<V, std::conditional_t<Mod == 0, std::uint64_t, std::uint32_t>>);
  auto norm = [](V x) -> V { if constexpr (Mod) return x % Mod; else return x; };
  auto add = [&](V x, V y) -> V {
    if constexpr (Mod) return (std::uint64_t(x) + y) % Mod;
    else return x + y;
  };
  auto affine = [&](V x, V a, V b) -> V {
    if constexpr (Mod) return (std::uint64_t(norm(a)) * x + norm(b)) % Mod;
    else return a * x + b;
  };
  std::mt19937_64 rng(seed);
  std::vector<V> a;
  int step = -1, l = 0, r = 0;
  const char* op = "construct";
  auto fail = [&](const char* what, std::uint64_t expected, std::uint64_t actual) {
    std::cerr << "seed=" << seed << " mod=" << Mod << " step=" << step << " op=" << op
              << " check=" << what << " l=" << l << " r=" << r << " expected=" << expected
              << " actual=" << actual << " input=";
    for (auto x : a) std::cerr << x << ',';
    std::cerr << '\n'; std::exit(1);
  };
  auto sum = [&](int first, int last) {
    V s = 0;
    for (int i = first; i < last; ++i) s = add(s, a[i]);
    return s;
  };
  auto check = [&](Tree& tree, bool full) {
    if (tree.size() != int(a.size())) fail("size", a.size(), tree.size());
    if (tree.empty() != a.empty()) fail("empty", a.empty(), tree.empty());
    if (tree.all_prod() != sum(0, int(a.size()))) fail("all_prod", sum(0, int(a.size())), tree.all_prod());
    if (full) {
      auto actual = tree.to_vector();
      if (actual.size() != a.size()) fail("to_vector size", a.size(), actual.size());
      for (int i = 0; i < int(a.size()); ++i)
        if (a[i] != actual[i]) { l = i; fail("to_vector", a[i], actual[i]); }
    }
  };
  Tree empty;
  check(empty, true);
  empty.reserve(0); empty.apply(0, 0, V(-1), V(-1)); empty.reverse(0, 0);
  if (empty.prod(0, 0)) fail("empty prod", 0, empty.prod(0, 0));
  for (int initial : {0, 1, 2, 15, 16, 17, 255, 256, 257, 4097, 65537}) {
    std::vector<V> supplied(initial);
    a.resize(initial);
    for (int i = 0; i < initial; ++i) { supplied[i] = i % 3 ? V(rng()) : V(-1); a[i] = norm(supplied[i]); }
    op = "construct"; step = -1;
    Tree tree(supplied, seed + initial);
    check(tree, true);
    tree.reserve(initial + 1024);
    for (step = 0; step < (initial > 4096 ? 60 : 700); ++step) {
      int n = int(a.size());
      l = int(rng() % (n + 1)); r = int(rng() % (n + 1));
      if (l > r) std::swap(l, r);
      if (step % 17 == 0) { l = 0; r = n; }
      if (step % 19 == 0) r = l;
      int kind = int(rng() % 9);
      if (!n) kind = 0;
      V x = step % 7 == 0 ? V(-1) : V(rng());
      if (kind == 0) {
        op = "insert"; tree.insert(l, x); a.insert(a.begin() + l, norm(x));
      } else if (kind == 1) {
        op = "erase"; l = int(rng() % n); tree.erase(l); a.erase(a.begin() + l);
      } else if (kind == 2) {
        op = "set"; l = int(rng() % n); tree.set(l, x); a[l] = norm(x);
      } else if (kind == 3) {
        op = "get"; l = int(rng() % n);
        if (tree.get(l) != a[l]) fail("get", a[l], tree.get(l));
      } else if (kind == 4) {
        op = "prod";
        if (tree.prod(l, r) != sum(l, r)) fail("prod", sum(l, r), tree.prod(l, r));
      } else if (kind == 5) {
        op = "reverse"; tree.reverse(l, r); std::reverse(a.begin() + l, a.begin() + r);
      } else if (kind == 6) {
        op = "apply";
        V b = step % 5 == 0 ? V(-1) : V(rng());
        x = step % 3 == 0 ? 0 : step % 3 == 1 ? 1 : x;
        tree.apply(l, r, x, b);
        for (int i = l; i < r; ++i) a[i] = affine(a[i], x, b);
      } else if (kind == 7) {
        op = "identity and reverse twice";
        tree.apply(l, r, 1, 0); tree.reverse(l, r); tree.reverse(l, r);
      } else {
        op = "copy and move";
        auto copy = tree;
        check(copy, true);
        if (n) copy.set(0, norm(a[0] + V(1)));
        check(tree, true);
        copy = tree;
        auto* own = &copy; copy = *own;
        Tree moved(std::move(copy));
        if (!copy.empty() || copy.size() || copy.all_prod() || copy.prod(0, 0) || !copy.to_vector().empty())
          fail("move constructor source", 0, copy.size());
        copy.insert(0, V(-1));
        if (copy.get(0) != norm(V(-1))) fail("reinsert moved source", norm(V(-1)), copy.get(0));
        copy = std::move(moved);
        if (!moved.empty() || moved.size() || moved.all_prod()) fail("move assignment source", 0, moved.size());
        moved.insert(0, 0); moved.erase(0);
        check(copy, true);
      }
      check(tree, step % 31 == 0);
    }
    op = "clear and reuse"; tree.clear(); a.clear(); check(tree, true);
    tree.reserve(32);
    for (int i = 0; i < 33; ++i) { tree.insert(0, V(-1)); a.insert(a.begin(), norm(V(-1))); }
    tree.apply(0, 33, 0, V(-1));
    tree.reverse(0, 33); check(tree, true);
    while (!a.empty()) { int p = int(a.size() / 2); tree.erase(p); a.erase(a.begin() + p); }
    check(tree, true);
  }
}
int main(int argc, char** argv) {
  auto seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 20261022ULL;
  run<998244353>(seed); run<0>(seed); run<1>(seed); run<15>(seed);
  run<2147483647U>(seed); run<2147483648U>(seed);
}

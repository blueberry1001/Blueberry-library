#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <utility>
#include <vector>
#include "blueberry/data-structure/persistent-array.hpp"
#include "blueberry/data-structure/persistent-union-find.hpp"
using namespace std;

struct Value {
  int x = 0;
  Value() = default;
  explicit Value(int v) : x(v) {}
  Value(const Value&) = default;
  Value(Value&&) = default;
  Value& operator=(const Value&) = default;
  Value& operator=(Value&&) = default;
};
struct CopyOnly {
  int x = 0;
  CopyOnly() = default;
  explicit CopyOnly(int v) : x(v) {}
  CopyOnly(const CopyOnly&) = default;
  CopyOnly& operator=(const CopyOnly&) = default;
  CopyOnly(CopyOnly&&) = delete;
  CopyOnly& operator=(CopyOnly&&) = delete;
};

int main(int argc, char** argv) {
  const unsigned long long seed = argc > 1 ? strtoull(argv[1], nullptr, 10) : 1;
  mt19937_64 rng(seed);
  auto fail = [&](const char* what, int n, int step, int version, int a, int b) {
    cerr << "seed=" << seed << " n=" << n << " step=" << step << " version=" << version
         << " a=" << a << " b=" << b << " operation=" << what << '\n';
    exit(1);
  };
  {
    blueberry::PersistentUnionFind u(9);
    int a = u.merge(0, 5, 1), b = u.merge(0, 2, 6), c = u.merge(a, 5, 7);
    int d = u.merge(b, 2, 8), e = u.merge(c, 5, 1);
    if (u.leader(a, 1) != 5 || u.leader(b, 6) != 2 || u.leader(c, 7) != 5 ||
        u.leader(d, 8) != 2 || u.same(a, 5, 7) || u.same(b, 2, 8) ||
        u.versions() != 6 || e != 5 || u.components(e) != u.components(c))
      fail("tie/branch/self", 9, 0, 0, 0, 0);
    int f = u.merge(a, 1, 6);
    if (u.leader(f, 6) != 5 || u.leader(b, 6) != 2 || u.same(f, 5, 2))
      fail("historical branch", 9, 0, 0, 0, 0);
  }
  for (int n : {0, 1, 2, 3, 7, 8, 9, 31, 32, 33, 65, 129}) {
    blueberry::PersistentUnionFind uf(n);
    vector<vector<int>> expected(1, vector<int>(n));
    iota(expected[0].begin(), expected[0].end(), 0);
    if (uf.size() != n || uf.versions() != 1 || uf.components(0) != n) fail("initial", n, 0, 0, 0, 0);
    for (int step = 0; n && step < 1200; ++step) {
      int v = rng() % expected.size(), a = rng() % n, b = step % 4 == 0 ? a : rng() % n;
      auto next = expected[v];
      int x = next[a], y = next[b];
      for (int& e : next) if (e == y) e = x;
      if (uf.merge(v, a, b) != static_cast<int>(expected.size())) fail("version", n, step, v, a, b);
      expected.push_back(move(next));
      for (int k = 0; k < 12; ++k) {
        int w = k == 0 ? v : (k == 1 ? static_cast<int>(expected.size()) - 1 : rng() % expected.size());
        int p = rng() % n, q = rng() % n;
        const auto& e = expected[w];
        if (uf.same(w, p, q) != (e[p] == e[q])) fail("same", n, step, w, p, q);
        if (uf.comp_size(w, p) != count(e.begin(), e.end(), e[p])) fail("size", n, step, w, p, q);
        auto unique = e;
        sort(unique.begin(), unique.end());
        unique.erase(std::unique(unique.begin(), unique.end()), unique.end());
        if (uf.components(w) != static_cast<int>(unique.size())) fail("components", n, step, w, p, q);
        int leader = uf.leader(w, p);
        if (leader < 0 || leader >= n || e[leader] != e[p] || uf.leader(w, leader) != leader) fail("leader", n, step, w, p, q);
      }
    }
    auto copied = uf;
    auto moved = std::move(copied);
    if (moved.versions() != uf.versions() || moved.components(0) != n) fail("copy/move", n, 0, 0, 0, 0);
    if (n > 1) {
      int old = uf.versions();
      moved.merge(0, 0, 1);
      if (uf.versions() != old || uf.same(0, 0, 1) || !moved.same(old, 0, 1)) fail("copy independence", n, 0, 0, 0, 1);
    }
  }
  blueberry::PersistentArray<Value> a(9, Value(7));
  int v = a.set(0, 3, Value(42)), f = a.fork(v);
  auto copy = a;
  auto moved = std::move(copy);
  if (a.get(0, 3).x != 7 || a.get(v, 3).x != 42 || moved.get(f, 3).x != 42) fail("generic array copy/move", 9, 0, v, 3, 0);
  CopyOnly initial(3), updated(8);
  blueberry::PersistentArray<CopyOnly> ca(5, initial);
  int cv = ca.set(0, 4, updated);
  auto ca_copy = ca;
  auto ca_moved = std::move(ca_copy);
  if (ca.get(0, 4).x != 3 || ca_moved.get(cv, 4).x != 8) fail("copy-only array", 5, 0, cv, 4, 0);
  cout << "ok seed=" << seed << '\n';
}

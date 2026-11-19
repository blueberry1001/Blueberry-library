#include <cstdlib>
#include <iostream>
#include <random>
#include "blueberry/data-structure/persistent-union-find.hpp"
using namespace std;
int main(int argc, char** argv) {
  unsigned long long seed = argc > 1 ? strtoull(argv[1], nullptr, 10) : 1;
  mt19937_64 rng(seed);
  for (int n : {1, 2, 3, 7, 9, 31, 33, 65}) {
    blueberry::PersistentUnionFind uf(n);
    cout << n << ' ' << uf.size() << ' ' << uf.versions() << ' ' << uf.components(0) << '\n';
    for (int step = 0; step < 1000; ++step) {
      int version = rng() % uf.versions(), a = rng() % n, b = step % 4 ? rng() % n : a;
      int next = uf.merge(version, a, b);
      cout << step << ' ' << next << ' ' << uf.versions() << '\n';
      for (int v : {version, next}) {
        cout << v << ' ' << uf.components(v) << '\n';
        for (int p = 0; p < n; ++p)
          cout << p << ' ' << uf.leader(v, p) << ' ' << uf.comp_size(v, p) << ' ' << uf.same(v, p, a) << '\n';
      }
    }
  }
}

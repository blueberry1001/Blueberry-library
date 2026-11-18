#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/multipoint-evaluation.hpp"

template <class mint>
bool verify(unsigned seed) {
  std::mt19937 rng(seed);
  for (int n : {0, 1, 2, 8, 31, 32, 33, 64, 129})
    for (int m : {0, 1, 2, 31, 32, 33, 100})
      for (int mode = 0; mode < 4; ++mode) {
        std::vector<mint> f(n), x(m);
        for (auto& v : f) v = rng();
        for (auto& v : x) v = mode == 1 ? rng() % 3 : rng();
        if (mode == 2) for (int i = n / 2; i < n; ++i) f[i] = 0;
        if (mode == 3) for (auto& v : f) v = 0;
        auto y = blueberry::multipoint_evaluation(f, x);
        if (y.size() != x.size()) return false;
        for (int i = 0; i < m; ++i) {
          mint expected = 0;
          for (auto it = f.rbegin(); it != f.rend(); ++it) expected = expected * x[i] + *it;
          if (y[i] != expected) {
            std::cerr << "seed=" << seed << " mod=" << mint::mod() << " n=" << n << " m=" << m << " mode=" << mode << " i=" << i << '\n';
            return false;
          }
        }
      }
  return true;
}
int main(int argc, char** argv) {
  const unsigned seed = argc > 1 ? std::strtoul(argv[1], nullptr, 10) : 1729;
  return !(verify<atcoder::modint998244353>(seed) && verify<atcoder::static_modint<167772161>>(seed) && verify<atcoder::static_modint<469762049>>(seed));
}

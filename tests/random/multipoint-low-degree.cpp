#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/multipoint-evaluation.hpp"
using namespace std;

template<class Mint>
bool check(const vector<Mint>& f, const vector<Mint>& x, unsigned seed) {
  auto f_before = f, x_before = x;
  auto y = blueberry::multipoint_evaluation(f, x);
  if (f != f_before || x != x_before || y.size() != x.size()) return false;
  for (size_t i = 0; i < x.size(); ++i) {
    Mint expected = 0;
    for (auto it = f.rbegin(); it != f.rend(); ++it) expected = expected * x[i] + *it;
    if (y[i] != expected) {
      cerr << "seed=" << seed << " mod=" << Mint::mod() << " n=" << f.size()
           << " m=" << x.size() << " i=" << i << " expected=" << expected.val()
           << " actual=" << y[i].val() << '\n';
      return false;
    }
  }
  return true;
}
template<class Mint>
bool run(unsigned seed) {
  mt19937 rng(seed);
  for (int n : {0, 1, 2, 8, 31, 32, 33, 64, 129})
    for (int m : {0, 1, 2, 31, 32, 33, 100})
      for (int mode = 0; mode < 4; ++mode) {
        vector<Mint> f(n), x(m);
        for (auto& v : f) v = rng();
        for (auto& v : x) v = mode == 1 ? rng() % 3 : rng();
        if (mode == 2) for (int i = n / 2; i < n; ++i) f[i] = 0;
        if (mode == 3) for (auto& v : f) v = 0;
        if (!check(f, x, seed) || !check(f, f, seed)) return false;
      }
  for (int trial = 0; trial < 100; ++trial) {
    vector<Mint> f(rng() % 70), x(rng() % 250);
    for (auto& v : f) v = rng();
    for (auto& v : x) v = rng() % 13;
    if (!f.empty() && trial % 2 == 0) f.back() = 0;
    if (!check(f, x, seed)) return false;
  }
  vector<Mint> f(32), x(10000);
  for (auto& v : f) v = rng();
  for (auto& v : x) v = rng();
  if (!check(f, x, seed)) return false;
  f.resize(8192);
  for (auto& v : f) v = rng();
  return check(f, vector<Mint>{Mint(0)}, seed) && check(f, vector<Mint>{Mint(1), Mint(-1)}, seed);
}
int main(int argc, char** argv) {
  unsigned seed = argc > 1 ? strtoul(argv[1], nullptr, 10) : 1729;
  return !(run<atcoder::modint998244353>(seed) && run<atcoder::static_modint<167772161>>(seed)
           && run<atcoder::static_modint<469762049>>(seed));
}

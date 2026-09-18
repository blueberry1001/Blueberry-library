#include <cassert>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/multipoint-evaluation.hpp"
#include "blueberry/math/polynomial-interpolation.hpp"
#include "blueberry/math/sample-point-shift.hpp"
using mint = atcoder::modint998244353;
mint eval(const std::vector<mint>& f, mint x) {
  mint y = 0;
  for (auto it = f.rbegin(); it != f.rend(); ++it) y = y * x + *it;
  return y;
}
int main(int argc, char** argv) {
  const unsigned seed = argc > 1 ? std::strtoul(argv[1], nullptr, 10) : 1;
  std::cerr << "seed=" << seed << '\n';
  std::mt19937 rng(seed);
  for (int trial = 0; trial < 180; ++trial) {
    int n = rng() % 180, m = rng() % 200;
    std::vector<mint> f(n), x(m), distinct(n), samples(n);
    for (auto& v : f) v = rng();
    for (auto& v : x) v = rng() % 13;
    const auto actual = blueberry::multipoint_evaluation(f, x);
    for (int i = 0; i < m; ++i) assert(actual[i] == eval(f, x[i]));
    for (int i = 0; i < n; ++i) { distinct[i] = mint(i * 3 + 5); samples[i] = eval(f, distinct[i]); }
    assert(blueberry::polynomial_interpolation(distinct, samples) == f);
    for (int i = 0; i < n; ++i) samples[i] = eval(f, mint(i));
    for (mint start : {mint(0), mint(n / 2), mint(n), mint(1000), mint(-50), mint(-1)}) {
      const auto shifted = blueberry::sample_point_shift(samples, start, m);
      for (int i = 0; i < m; ++i) assert(shifted[i] == eval(f, start + mint(i)));
    }
  }
}

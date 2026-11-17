#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>
#include "blueberry/math/divisor-convolution.hpp"

struct Matrix {
  std::array<int, 4> x{};
  Matrix& operator+=(const Matrix& b) { for (int i = 0; i < 4; ++i) x[i] = (x[i] + b.x[i]) % 17; return *this; }
  Matrix& operator-=(const Matrix& b) { for (int i = 0; i < 4; ++i) x[i] = (x[i] + 17 - b.x[i]) % 17; return *this; }
  Matrix& operator*=(const Matrix& b) {
    auto a = x;
    x = {(a[0]*b.x[0]+a[1]*b.x[2])%17, (a[0]*b.x[1]+a[1]*b.x[3])%17,
         (a[2]*b.x[0]+a[3]*b.x[2])%17, (a[2]*b.x[1]+a[3]*b.x[3])%17};
    return *this;
  }
  bool operator==(const Matrix&) const = default;
};
int main(int argc, char** argv) {
  const auto seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 20261117;
  std::mt19937_64 rng(seed);
  std::vector<int> sizes{0, 1, 2, 31, 127, 511, 512, 513, 1023, 1024, 1025};
  for (int n : sizes) {
    std::vector<Matrix> a(n), b(n), gcd(n), lcm(n);
    for (auto& v : a) for (auto& x : v.x) x = rng() % 17;
    for (auto& v : b) for (auto& x : v.x) x = rng() % 17;
    for (int i = 1; i < n; ++i) for (int j = 1; j < n; ++j) {
      auto product = a[i]; product *= b[j];
      gcd[std::gcd(i, j)] += product;
      int k = std::lcm(i, j); if (k < n) lcm[k] += product;
    }
    if (blueberry::gcd_convolution(a, b) != gcd || blueberry::lcm_convolution(a, b) != lcm) {
      std::cerr << "seed=" << seed << " n=" << n << '\n'; return 1;
    }
  }
}

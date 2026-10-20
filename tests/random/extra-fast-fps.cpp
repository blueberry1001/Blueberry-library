#ifdef BLUEBERRY_FPS_HEADER
#include BLUEBERRY_FPS_HEADER
#else
#include "blueberry/extra-fast/formal-power-series.hpp"
#endif
#include "blueberry/math/formal-power-series.hpp"
#include <atcoder/modint>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <vector>

using U = std::uint32_t;
using V = std::vector<U>;
constexpr U mod = 998244353;
std::uint64_t seed;
U multiply(U a, U b) { return std::uint64_t(a) * b % mod; }
U power(U a, U e) { U r = 1; for (; e; e >>= 1, a = multiply(a, a)) if (e & 1) r = multiply(r, a); return r; }
void compare(const V& actual, const V& expected, const V& input, const std::string& operation, bool simd) {
  if (actual == expected) return;
  std::cerr << "seed=" << seed << " op=" << operation << " simd=" << simd << " input=";
  for (U x : input) std::cerr << x << ',';
  std::cerr << "\nexpected=";
  for (U x : expected) std::cerr << x << ',';
  std::cerr << "\nactual=";
  for (U x : actual) std::cerr << x << ',';
  std::cerr << '\n';
  std::exit(1);
}
V direct_product(const V& a, const V& b) {
  if (a.empty() || b.empty()) return {};
  V c(a.size() + b.size() - 1);
  for (std::size_t i = 0; i < a.size(); ++i)
    for (std::size_t j = 0; j < b.size(); ++j) c[i + j] = (c[i + j] + multiply(a[i], b[j])) % mod;
  return c;
}
V direct_inverse(const V& f, int n) {
  V g(n);
  if (!n) return g;
  g[0] = power(f[0], mod - 2);
  for (int i = 1; i < n; ++i) {
    U value = 0;
    for (int j = 1; j <= i && j < int(f.size()); ++j) value = (value + multiply(f[j], g[i - j])) % mod;
    g[i] = multiply(value ? mod - value : 0, g[0]);
  }
  return g;
}
V direct_exp(const V& f, int n) {
  V g(n);
  if (!n) return g;
  g[0] = 1;
  for (int i = 1; i < n; ++i) {
    U value = 0;
    for (int j = 1; j <= i && j < int(f.size()); ++j)
      value = (value + multiply(multiply(j, f[j]), g[i - j])) % mod;
    g[i] = multiply(value, power(i, mod - 2));
  }
  return g;
}
int main(int argc, char** argv) {
  seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 20261020;
  std::mt19937_64 rng(seed);
  for (bool simd : {false, true}) {
    blueberry::extra_fast::FPS998244353 fps(simd);
    if (fps.uses_simd() != (simd && fps.simd_available())) return 2;
    auto copied = fps;
    auto moved = std::move(copied);
    copied = fps;
    copied = std::move(moved);
    auto* self = &copied;
    copied = *self;
    if (copied.uses_simd() != fps.uses_simd() || moved.uses_simd() != fps.uses_simd()) return 3;
    compare(fps.convolution({}, V{mod}), {}, {}, "convolution-empty-ignored", simd);
    compare(fps.inv(V{1, mod}, 1), V{1}, {}, "inv-truncated", simd);
    compare(fps.log(V{1, mod}, 1), V{0}, {}, "log-truncated", simd);
    compare(fps.exp(V{0, mod}, 1), V{1}, {}, "exp-truncated", simd);
    for (int n : {0, 1, 2, 3, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65, 127, 128, 129, 255, 256, 257, 511, 512, 513, 1023, 1024, 1025}) {
      for (int distribution = 0; distribution < 4; ++distribution) {
        V f(n), b(n / 2 + (n != 0));
        for (U& x : f) x = distribution == 0 ? rng() % mod : distribution == 1 ? mod - 1 : distribution == 2 ? 0 : 1;
        for (U& x : b) x = rng() % mod;
        compare(fps.convolution(f, b), direct_product(f, b), f, "convolution", simd);
        if (n) f[0] = distribution == 0 ? rng() % (mod - 1) + 1 : 1;
        const int degree = n + (n < 260 ? 3 : 1);
        if (n) {
          compare(fps.inv(f, degree), direct_inverse(f, degree), f, "inv-padded", simd);
          compare(fps.inv(f, (n + 1) / 2), direct_inverse(f, (n + 1) / 2), f, "inv-prefix", simd);
        }
        if (n) {
          f[0] = 1;
          const V logarithm = fps.log(f, degree);
          V derivative(n ? n - 1 : 0), expected(degree);
          for (int i = 1; i < n; ++i) derivative[i - 1] = multiply(i, f[i]);
          const V product = direct_product(derivative, direct_inverse(f, degree));
          for (int i = 1; i < degree; ++i)
            expected[i] = i <= int(product.size()) ? multiply(product[i - 1], power(i, mod - 2)) : 0;
          compare(logarithm, expected, f, "log-padded", simd);
        }
        if (n) f[0] = 0;
        compare(fps.exp(f, degree), direct_exp(f, degree), f, "exp-padded", simd);
        compare(fps.exp(f), direct_exp(f, n), f, "exp-default", simd);
        compare(fps.exp(f, 0), {}, f, "exp-empty", simd);
        compare(fps.inv(f, 0), {}, f, "inv-empty", simd);
        compare(fps.log(f, 0), {}, f, "log-empty", simd);
      }
    }
    // Larger non-power-of-two transforms, compared against the existing ACL-backed FPS.
    for (int n : {4095, 4096, 4097, 16385}) {
      V f(n);
      using Mint = atcoder::modint998244353;
      blueberry::FPS<Mint> reference(n);
      for (int i = 0; i < n; ++i) reference[i] = f[i] = rng() % mod;
      f[0] = 1; reference[0] = 1;
      for (const std::string op : {"inv", "log", "exp"}) {
        if (op == "exp") f[0] = 0, reference[0] = 0;
        auto actual = op == "inv" ? fps.inv(f) : op == "log" ? fps.log(f) : fps.exp(f);
        auto expected_mint = op == "inv" ? reference.inv() : op == "log" ? reference.log() : reference.exp();
        V expected(n);
        for (int i = 0; i < n; ++i) expected[i] = expected_mint[i].val();
        compare(actual, expected, f, op + "-large", simd);
      }
    }
  }
  std::cout << "extra-fast FPS OK seed=" << seed << '\n';
}

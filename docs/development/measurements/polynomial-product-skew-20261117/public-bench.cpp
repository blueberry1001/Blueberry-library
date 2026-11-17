#define NOMINMAX
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif
#include <atcoder/modint>
#define blueberry product_original
#include "baseline-original.hpp.txt"
#undef blueberry
#include "blueberry/math/polynomial-product.hpp"
using Mint = atcoder::modint998244353;
using Poly = std::vector<Mint>;
using Factors = std::vector<Poly>;

Poly run(int mode, Factors f) {
  if (mode == 0) return product_original::polynomial_product(std::move(f));
  return blueberry::polynomial_product(std::move(f));
}
Poly naive(const Factors& f) {
  Poly a{1};
  for (const auto& b : f) {
    if (b.empty()) return {};
    Poly c(a.size() + b.size() - 1);
    for (std::size_t i = 0; i < a.size(); ++i)
      for (std::size_t j = 0; j < b.size(); ++j) c[i+j] += a[i] * b[j];
    a = std::move(c);
  }
  return a;
}
int check() {
  std::mt19937_64 rng(20261117);
  for (int trial = 0; trial < 5000; ++trial) {
    Factors f(rng() % 24);
    for (auto& a : f) {
      a.resize(trial % 2 ? 1 + rng() % 11 : rng() % 12);
      for (auto& x : a) x = rng() % 11;
    }
    auto expected = naive(f);
    for (int mode : {0, 4}) if (run(mode, f) != expected) {
      std::cerr << "seed=20261117 trial=" << trial << " mode=" << mode << '\n'; return 1;
    }
  }
  Factors f(150, Poly{1}); f[0] = Poly(500, 7); f[4] = {0}; f[7] = {1, 0, 3};
  auto expected = naive(f);
  for (int mode : {0, 4}) if (run(mode, f) != expected) return 1;
  f[4] = {11}; expected = naive(f);
  for (int mode : {0, 4}) if (run(mode, f) != expected) return 1;
  std::cout << "all checks passed\n";
  return 0;
}
Factors input(int category, std::uint64_t seed) {
  std::mt19937_64 rng(seed);
  Factors f;
  if (category == 0) return f;
  if (category == 1) f.resize(1, Poly(64));
  if (category == 2) f.resize(2, Poly(8));
  if (category == 3) f.resize(8192, Poly(2));
  if (category == 4) f.resize(1024, Poly(16));
  if (category == 5 || category == 6) {
    f.resize(1024, Poly(2));
    (category == 5 ? f.front() : f.back()).resize(65536);
  }
  if (category == 7 || category == 10) {
    f.resize(8192, Poly(1)); f.front().resize(65536);
  }
  if (category == 8) f.resize(8192, Poly(1));
  if (category == 9) { f.resize(32, Poly(2)); f.front().resize(256); }
  if (category == 11) { f.resize(512, Poly(2)); for (int i = 0; i < 512; i += 4) f[i].resize(1); }
  if (category == 12) { f.resize(512, Poly(2)); for (int i = 0; i < 512; i += 2) f[i].resize(16); }
  if (category == 13) { f.resize(512, Poly(2)); for (int i = 0; i < 512; i += 2) f[i].resize(8); }
  if (category == 14) { f.resize(512, Poly(2)); for (int i = 0; i < 512; i += 2) f[i].resize(3); }
  if (category == 15) { f.resize(4, Poly(2)); f[0].resize(128); }
  if (category == 16) { f.resize(8, Poly(2)); f[0].resize(128); }
  if (category == 17) { f.resize(512, Poly(2)); for (int i = 0; i < 512; i += 2) f[i].resize(1); }
  if (category == 18) { f.resize(512, Poly(2)); for (int i = 0; i < 512; i += 64) f[i].resize(1); }
  if (category == 19) { f.resize(512, Poly(1)); for (int i = 0; i < 512; i += 4) f[i].resize(2); }
  for (auto& a : f) for (auto& x : a) x = rng() % Mint::mod();
  if (category == 10) f[f.size()/2][0] = 0;
  return f;
}
int main(int argc, char** argv) {
  if (argc > 1 && std::string(argv[1]) == "check") return check();
  if (argc != 4) return 2;
  int mode = std::atoi(argv[1]), category = std::atoi(argv[2]), count = std::atoi(argv[3]);
  std::vector<Factors> inputs; inputs.reserve(count);
  for (int i = 0; i < count; ++i) inputs.push_back(input(category, 20261117 + i));
  std::vector<Poly> output; output.reserve(count);
  auto start = std::chrono::steady_clock::now();
  for (auto& f : inputs) output.push_back(run(mode, std::move(f)));
  double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
  std::uint64_t hash = 0;
  for (auto& a : output) { hash = hash * 37 + a.size(); for (auto x : a) hash = hash * 37 + x.val(); }
  std::cout << ms << ' ' << hash;
#ifdef _WIN32
  PROCESS_MEMORY_COUNTERS info{};
  if (GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info))) std::cout << ' ' << info.PeakWorkingSetSize;
  else std::cout << " missing";
#endif
  std::cout << '\n';
}

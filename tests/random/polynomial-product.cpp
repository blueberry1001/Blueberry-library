#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <utility>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/polynomial-product.hpp"

template<class Mint>
std::vector<Mint> naive(const std::vector<std::vector<Mint>>& factors) {
  std::vector<Mint> result{1};
  for (const auto& factor : factors) {
    if (factor.empty()) return {};
    std::vector<Mint> next(result.size() + factor.size() - 1);
    for (std::size_t i = 0; i < result.size(); ++i)
      for (std::size_t j = 0; j < factor.size(); ++j) next[i + j] += result[i] * factor[j];
    result = std::move(next);
  }
  return result;
}

template<class Mint>
void check(const std::vector<std::vector<Mint>>& factors, std::uint64_t seed, int trial) {
  const auto expected = naive(factors);
  const auto actual = blueberry::polynomial_product(factors);
  if (actual == expected && blueberry::polynomial_product(std::vector(factors)) == expected) return;
  std::cerr << "seed=" << seed << " trial=" << trial << " mod=" << Mint::mod() << '\n';
  for (const auto& factor : factors) {
    std::cerr << factor.size();
    for (auto value : factor) std::cerr << ' ' << value.val();
    std::cerr << '\n';
  }
  std::cerr << "expected";
  for (auto value : expected) std::cerr << ' ' << value.val();
  std::cerr << "\nactual";
  for (auto value : actual) std::cerr << ' ' << value.val();
  std::cerr << '\n';
  std::exit(1);
}

template<class Mint>
void random_cases(std::mt19937_64& rng, std::uint64_t seed) {
  for (int trial = 0; trial < 1500; ++trial) {
    std::vector<std::vector<Mint>> f(rng() % 24);
    for (auto& factor : f) {
      factor.resize(trial % 3 == 0 ? rng() % 10 : 1 + rng() % 9);
      for (auto& value : factor) value = rng() % Mint::mod();
      if (!factor.empty() && rng() % 4 == 0) factor.back() = 0;
    }
    check(f, seed, trial);
  }
  for (int count : {3, 4, 7, 8, 9, 16, 32}) for (int big : {0, count - 1}) {
    std::vector<std::vector<Mint>> f(count, std::vector<Mint>{1, 1});
    f[big].resize(257, Mint(2)); f[big].back() = 0;
    check(f, seed, 2000 + count);
    for (int i = 0; i < count; i += 2) if (i != big) f[i] = {Mint(i + 1)};
    check(f, seed, 2100 + count);
    if (count > 3) { f[(big + 1) % count] = {0}; check(f, seed, 2200 + count); }
  }
}

int main(int argc, char** argv) {
  const std::uint64_t seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 20261117;
  std::mt19937_64 rng(seed);
  random_cases<atcoder::modint998244353>(rng, seed);
  random_cases<atcoder::static_modint<167772161>>(rng, seed);
  for (int trial = 0; trial < 1000; ++trial) {
    std::vector<std::vector<atcoder::static_modint<17>>> f(rng() % 8);
    for (auto& factor : f) {
      factor.resize(1 + rng() % 3);
      for (auto& value : factor) value = rng() % 17;
    }
    check(f, seed, 3000 + trial);
  }
}

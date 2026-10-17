#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/bernoulli.hpp"
#include "blueberry/math/bell.hpp"
#include "blueberry/math/partition.hpp"

unsigned seed = 20261017;

template <class Mint>
void check(int n) {
  const auto b = blueberry::bernoulli<Mint>(n);
  const auto c = blueberry::bell<Mint>(n);
  const auto p = blueberry::partition<Mint>(n);
  std::vector<Mint> bern(n + 1), bells(n + 1), parts(n + 1);
  bern[0] = bells[0] = parts[0] = 1;
  for (int i = 1; i <= n; ++i) {
    Mint choose = 1;
    for (int j = 0; j < i; ++j) {
      bern[i] -= choose * bern[j];
      choose *= Mint(i + 1 - j) / Mint(j + 1);
    }
    bern[i] /= Mint(i + 1);
    choose = 1;
    for (int j = 0; j < i; ++j) {
      bells[i] += choose * bells[j];
      choose *= Mint(i - 1 - j) / Mint(j + 1);
    }
  }
  // Unbounded knapsack gives an independent definition of integer partitions.
  for (int part = 1; part <= n; ++part) {
    for (int total = part; total <= n; ++total) parts[total] += parts[total - part];
  }
  const std::vector<std::vector<Mint>> actual{b, c, p}, expected{bern, bells, parts};
  for (int kind = 0; kind < 3; ++kind) {
    if (actual[kind].size() != static_cast<std::size_t>(n) + 1) std::abort();
    for (int i = 0; i <= n; ++i) {
      if (actual[kind][i] != expected[kind][i]) {
        std::cerr << "seed=" << seed << " mod=" << Mint::mod() << " n=" << n
                  << " kind=" << kind << " index=" << i
                  << " expected=" << expected[kind][i].val()
                  << " actual=" << actual[kind][i].val() << '\n';
        std::abort();
      }
    }
  }
}

int main(int argc, char** argv) {
  if (argc > 1) seed = static_cast<unsigned>(std::stoul(argv[1]));
  for (int n : {0, 1, 2, 3, 4, 5, 31, 32, 33, 62, 63, 64, 65, 127, 128, 129, 255, 256, 257})
    check<atcoder::modint998244353>(n);
  for (int n : {0, 1, 2, 63, 64, 65, 127, 255}) check<atcoder::static_modint<257>>(n);
  for (int n : {14, 15}) check<atcoder::static_modint<17>>(n);
  check<atcoder::static_modint<2>>(0);
  std::mt19937 random(seed);
  for (int run = 0; run < 80; ++run) check<atcoder::modint998244353>(static_cast<int>(random() % 300));
  std::cout << "fps_sequences seed=" << seed << " passed\n";
}

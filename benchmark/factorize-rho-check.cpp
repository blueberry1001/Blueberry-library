#include <cstdint>
#include <iostream>
#include <random>
#define blueberry blueberry_baseline
#include "factorize-rho-baseline.hpp.txt"
#undef blueberry
#include "../blueberry/math/factorize.hpp"
int main() {
  std::mt19937_64 rng(738517);
  for (int i = 0; i < 1000000; ++i) {
    auto n = rng() | 1, c = rng() % n, x = rng() % n;
    auto square = blueberry::factorize_detail::mul_mod(x, x, n);
    auto next = square >= n - c ? square - (n - c) : square + c;
    auto expected = (static_cast<unsigned __int128>(square) + c) % n;
    if (next != expected) return 1;
  }
  for (int i = 0; i < 2000; ++i) {
    auto n = rng() | 1;
    if (blueberry::factorize(n) != blueberry_baseline::factorize(n)) return 2;
  }
  std::cout << "seed 738517: 1000000 updates and 2000 uint64 factorizations passed\n";
}

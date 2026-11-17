#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <vector>
#include "blueberry/math/prime-sieve.hpp"

int main(int argc, char** argv) {
  unsigned seed = argc > 1 ? static_cast<unsigned>(std::stoul(argv[1])) : 20261117;
  std::mt19937 rng(seed);
  constexpr int max_n = 100000;
  std::vector<bool> prime(max_n + 1, true);
  for (int x = 0; x <= max_n; ++x) {
    if (x < 2) prime[x] = false;
    for (int d = 2; d <= x / d && prime[x]; ++d) if (x % d == 0) prime[x] = false;
  }
  auto check = [&](int n) {
    blueberry::PrimeSieve sieve(n);
    const auto& list = sieve.primes();
    std::vector<int> expected;
    for (int x = 0; x <= n; ++x) {
      if (sieve.is_prime(x) != prime[x]) {
        std::cerr << "seed=" << seed << " limit=" << n << " value=" << x << '\n';
        std::exit(1);
      }
      if (prime[x]) expected.push_back(x);
    }
    if (sieve.limit() != n || list != expected || &list != &sieve.primes()) {
      std::cerr << "seed=" << seed << " limit=" << n << " list or limit mismatch\n";
      std::exit(1);
    }
    auto copy = sieve;
    auto moved = std::move(copy);
    if (moved.limit() != n || moved.primes() != expected) {
      std::cerr << "seed=" << seed << " limit=" << n << " copy or move mismatch\n";
      std::exit(1);
    }
  };
  for (int n = 0; n <= 300; ++n) check(n);
  for (int n : {511, 512, 513, 1023, 1024, 1025, 4095, 4096, 4097, 65535, 65536, 65537, 100000}) check(n);
  for (int i = 0; i < 20; ++i) check(static_cast<int>(rng() % (max_n + 1)));
}

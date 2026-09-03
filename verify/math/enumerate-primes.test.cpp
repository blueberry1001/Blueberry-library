#define PROBLEM "https://judge.yosupo.jp/problem/enumerate_primes"

#include <iostream>

#include "blueberry/math/prime-sieve.hpp"

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  int n, step, offset;
  std::cin >> n >> step >> offset;
  const blueberry::PrimeSieve sieve(n);
  const auto& primes = sieve.primes();
  int selected = 0;
  for (int i = offset; i < static_cast<int>(primes.size()); i += step) ++selected;

  std::cout << primes.size() << ' ' << selected << '\n';
  bool first = true;
  for (int i = offset; i < static_cast<int>(primes.size()); i += step) {
    if (!first) std::cout << ' ';
    first = false;
    std::cout << primes[i];
  }
  std::cout << '\n';
}

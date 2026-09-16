#define PROBLEM "https://judge.yosupo.jp/problem/primality_test"
#include <cstdint>
#include <iostream>
#include "blueberry/math/factorize.hpp"

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  int q;
  std::cin >> q;
  while (q--) {
    std::uint64_t n;
    std::cin >> n;
    std::cout << (blueberry::is_prime(n) ? "Yes\n" : "No\n");
  }
}

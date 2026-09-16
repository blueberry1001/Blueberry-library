#define PROBLEM "https://judge.yosupo.jp/problem/factorize"
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
    const auto factors = blueberry::factorize(n);
    std::cout << factors.size();
    for (auto p : factors) std::cout << ' ' << p;
    std::cout << '\n';
  }
}

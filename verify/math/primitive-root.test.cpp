#define PROBLEM "https://judge.yosupo.jp/problem/primitive_root"
#include <cstdint>
#include <iostream>
#include "blueberry/math/primitive-root.hpp"

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  int q;
  std::cin >> q;
  // The checker accepts any generator of the multiplicative group modulo p.
  while (q--) {
    std::uint64_t p;
    std::cin >> p;
    std::cout << blueberry::primitive_root(p) << '\n';
  }
}

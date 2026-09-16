#define PROBLEM "https://judge.yosupo.jp/problem/sqrt_mod"
#include <cstdint>
#include <iostream>
#include "blueberry/math/mod-sqrt.hpp"

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  int q;
  std::cin >> q;
  while (q--) {
    std::int64_t a, p;
    std::cin >> a >> p;
    std::cout << blueberry::mod_sqrt(a, p).value_or(-1) << '\n';
  }
}

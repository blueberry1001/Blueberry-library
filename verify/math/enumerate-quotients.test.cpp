#define PROBLEM "https://judge.yosupo.jp/problem/enumerate_quotients"
#include <cstdint>
#include <iostream>
#include "blueberry/math/enumerate-quotients.hpp"
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  std::int64_t n; std::cin >> n;
  auto a = blueberry::enumerate_quotients(n);
  std::cout << a.size() << '\n';
  for (auto q : a) std::cout << q << ' ';
  std::cout << '\n';
}

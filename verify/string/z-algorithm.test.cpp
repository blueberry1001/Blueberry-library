#define PROBLEM "https://judge.yosupo.jp/problem/zalgorithm"

#include <iostream>
#include <string>

#include "blueberry/string/z-algorithm.hpp"

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  std::string text;
  std::cin >> text;
  const auto z = blueberry::z_algorithm(text);
  for (int i = 0; i < static_cast<int>(z.size()); ++i) {
    if (i > 0) std::cout << ' ';
    std::cout << z[i];
  }
  std::cout << '\n';
}

#define PROBLEM "https://judge.yosupo.jp/problem/lyndon_factorization"
#include <iostream>
#include <string>
#include "blueberry/string/lyndon-factorization.hpp"

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::string text;
  std::cin >> text;
  const auto bounds = blueberry::lyndon_factorization(text);
  for (std::size_t i = 0; i < bounds.size(); ++i)
    std::cout << bounds[i] << (i + 1 == bounds.size() ? '\n' : ' ');
}

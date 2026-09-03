#define PROBLEM "https://judge.yosupo.jp/problem/many_aplusb"

#include <iostream>

#include "blueberry/all.hpp"

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  int cases;
  std::cin >> cases;
  while (cases--) {
    long long a, b;
    std::cin >> a >> b;
    std::cout << a + b << '\n';
  }
}

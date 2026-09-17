#define PROBLEM "https://judge.yosupo.jp/problem/kth_root_integer"
#include <cstdint>
#include <iostream>
#include "blueberry/math/kth-root-integer.hpp"
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int t; std::cin >> t;
  while (t--) { std::uint64_t a; int k; std::cin >> a >> k;
    std::cout << blueberry::kth_root_integer(a, k) << '\n'; }
}

#define PROBLEM "https://judge.yosupo.jp/problem/discrete_logarithm_mod"
#include <cstdint>
#include <iostream>
#include "blueberry/math/discrete-log.hpp"
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int t; std::cin >> t;
  while (t--) { std::int64_t a, b, m; std::cin >> a >> b >> m;
    std::cout << blueberry::discrete_log(a, b, m).value_or(-1) << '\n'; }
}

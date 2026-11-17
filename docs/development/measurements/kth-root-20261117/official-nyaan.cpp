#include <cmath>
#include <cstdint>
#include <iostream>
#include "nyaan.hpp"
int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  int t;
  std::cin >> t;
  while (t--) {
    std::uint64_t n;
    int k;
    std::cin >> n >> k;
    std::cout << kth_root_integral(n,k) << '\n';
  }
}

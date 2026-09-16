#define PROBLEM "https://judge.yosupo.jp/problem/find_linear_recurrence"
#include <iostream>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/linear-recurrence.hpp"

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  int n;
  std::cin >> n;
  std::vector<atcoder::modint998244353> a(n);
  for (auto& x : a) { int value; std::cin >> value; x = value; }
  const auto coefficients = blueberry::berlekamp_massey(a);
  std::cout << coefficients.size() << '\n';
  for (std::size_t i = 0; i < coefficients.size(); ++i)
    std::cout << coefficients[i].val() << (i + 1 == coefficients.size() ? '\n' : ' ');
}

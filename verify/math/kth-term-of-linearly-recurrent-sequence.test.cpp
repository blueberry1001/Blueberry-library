#define PROBLEM "https://judge.yosupo.jp/problem/kth_term_of_linearly_recurrent_sequence"
#include <cstdint>
#include <iostream>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/linear-recurrence.hpp"

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  int d;
  std::uint64_t k;
  std::cin >> d >> k;
  std::vector<atcoder::modint998244353> a(d), c(d);
  for (auto& x : a) { int value; std::cin >> value; x = value; }
  for (auto& x : c) { int value; std::cin >> value; x = value; }
  std::cout << blueberry::linear_recurrence_kth(a, c, k).val() << '\n';
}

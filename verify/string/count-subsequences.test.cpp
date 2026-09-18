#define PROBLEM "https://judge.yosupo.jp/problem/number_of_subsequences"
#include <iostream>
#include <vector>
#include <atcoder/modint>
#include "blueberry/string/count-subsequences.hpp"

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  int n;
  std::cin >> n;
  std::vector<int> a(n);
  for (auto& x : a) std::cin >> x;
  // Equal subsequences are counted once; the empty subsequence is excluded.
  std::cout << blueberry::count_subsequences<atcoder::modint998244353>(a).val() << '\n';
}

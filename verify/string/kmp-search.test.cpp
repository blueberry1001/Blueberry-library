#define PROBLEM "https://onlinejudge.u-aizu.ac.jp/problems/ALDS1_14_B"
#include <iostream>
#include <string>
#include "blueberry/string/prefix-function.hpp"
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  std::string text, pattern; std::cin >> text >> pattern;
  for (int i : blueberry::kmp_search(text, pattern)) std::cout << i << '\n';
}

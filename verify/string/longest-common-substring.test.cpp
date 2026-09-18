#define PROBLEM "https://judge.yosupo.jp/problem/longest_common_substring"
#include <iostream>
#include <string>
#include "blueberry/string/longest-common-substring.hpp"
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  std::string s, t; std::cin >> s >> t;
  auto [a, b, c, d] = blueberry::longest_common_substring(s, t);
  std::cout << a << ' ' << b << ' ' << c << ' ' << d << '\n';
}

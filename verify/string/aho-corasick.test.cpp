#define PROBLEM "https://judge.yosupo.jp/problem/aho_corasick"
#include <iostream>
#include <string>
#include <vector>
#include "blueberry/string/aho-corasick.hpp"

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  int n;
  std::cin >> n;
  blueberry::AhoCorasick<> ac;
  std::vector<int> terminal(n);
  for (int& v : terminal) {
    std::string s;
    std::cin >> s;
    v = ac.add(s);
  }
  ac.build();
  std::cout << ac.size() << '\n';
  for (int v = 1; v < ac.size(); ++v)
    std::cout << ac.parent(v) << ' ' << ac.link(v) << '\n';
  for (int i = 0; i < n; ++i) std::cout << terminal[i] << (i + 1 == n ? '\n' : ' ');
}

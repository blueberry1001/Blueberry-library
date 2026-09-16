#define PROBLEM "https://judge.yosupo.jp/problem/eertree"
#include <iostream>
#include <string>
#include <vector>
#include "blueberry/string/eertree.hpp"

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::string text;
  std::cin >> text;
  blueberry::Eertree<> tree(static_cast<int>(text.size()));
  std::vector<int> suffix;
  for (char c : text) suffix.push_back(tree.add(c));
  std::cout << tree.size() << '\n';
  for (int v = 1; v <= tree.size(); ++v)
    std::cout << tree.parent(v) << ' ' << tree.link(v) << '\n';
  for (std::size_t i = 0; i < suffix.size(); ++i)
    std::cout << suffix[i] << (i + 1 == suffix.size() ? '\n' : ' ');
}

#define PROBLEM "https://judge.yosupo.jp/problem/set_xor_min"
#include <cstdint>
#include <iostream>
#include "blueberry/data-structure/persistent-binary-trie.hpp"
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int q, version = 0; std::cin >> q;
  blueberry::PersistentBinaryTrie<std::uint32_t, 30> trie;
  while (q--) {
    int type; std::uint32_t x; std::cin >> type >> x;
    // The problem uses a set; historical branches are covered by randomized tests.
    if (type == 0) { if (!trie.count(version, x)) version = trie.insert(version, x); }
    else if (type == 1) version = trie.erase(version, x);
    else std::cout << trie.xor_min(version, x).value() << '\n';
  }
}

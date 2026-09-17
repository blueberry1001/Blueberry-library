#define PROBLEM "https://judge.yosupo.jp/problem/set_xor_min"
#include <cstdint>
#include <iostream>
#include "blueberry/data-structure/binary-trie.hpp"
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int q; std::cin >> q;
  blueberry::BinaryTrie<std::uint32_t, 30> trie;
  while (q--) {
    int type; std::uint32_t x; std::cin >> type >> x;
    // This task is a set; the underlying trie deliberately supports duplicates.
    if (type == 0) { if (!trie.count(x)) trie.insert(x); }
    else if (type == 1) trie.erase(x);
    else std::cout << trie.xor_min(x).value() << '\n';
  }
}

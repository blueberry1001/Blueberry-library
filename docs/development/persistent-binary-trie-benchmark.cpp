// Compare identical retained branching histories; compile with -O2 -std=gnu++20 -I.
#include "blueberry/data-structure/binary-trie.hpp"
#include "blueberry/data-structure/persistent-binary-trie.hpp"
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>
int main() {
  constexpr int n = 20000;
  std::mt19937 rng(20260918);
  std::vector<unsigned> values(n);
  std::vector<int> parents(n);
  for (int i = 0; i < n; ++i) { values[i] = rng() & ((1u << 20) - 1); parents[i] = rng() % (i + 1); }
  std::cout << "updates=" << n << " Bits=20 seed=20260918 branching=uniform prior version\n";
  for (int run = 0; run < 5; ++run) {
    auto start = std::chrono::steady_clock::now();
    std::uint64_t sum = 0;
    blueberry::PersistentBinaryTrie<unsigned, 20> p;
    for (int i = 0; i < n; ++i) { int v = p.insert(parents[i], values[i]); sum += *p.xor_min(v, values[i] ^ 12345); }
    auto middle = std::chrono::steady_clock::now();
    std::vector<blueberry::BinaryTrie<unsigned, 20>> history(1);
    history.reserve(n + 1);
    std::uint64_t copied_sum = 0;
    for (int i = 0; i < n; ++i) { history.push_back(history[parents[i]]); history.back().insert(values[i]); copied_sum += *history.back().xor_min(values[i] ^ 12345); }
    auto end = std::chrono::steady_clock::now();
    if (sum != copied_sum) return 1;
    std::cout << "run=" << run << " persistent_ms=" << std::chrono::duration<double, std::milli>(middle-start).count()
              << " copied_ms=" << std::chrono::duration<double, std::milli>(end-middle).count() << " checksum=" << sum << '\n';
  }
}

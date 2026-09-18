#include "blueberry/data-structure/persistent-binary-trie.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

template<class U, int B> void run(std::uint64_t seed) {
  std::mt19937_64 rng(seed);
  blueberry::PersistentBinaryTrie<U, B> trie;
  std::vector<std::vector<U>> versions(1);
  assert(trie.empty(0) && !trie.kth(0, -1) && !trie.xor_min(0, 0));
  const U mask = std::numeric_limits<U>::max() >> (std::numeric_limits<U>::digits - B);
  for (int step = 0; step < 5000; ++step) {
    int old = rng() % versions.size();
    U x = step % 11 == 0 ? mask : step % 7 == 0 ? U{0} : U(rng()) & mask;
    auto next = versions[old];
    bool insert = rng() % 3 != 0;
    int v;
    if (insert) { next.insert(std::lower_bound(next.begin(), next.end(), x), x); v = trie.insert(old, x); }
    else {
      auto it = std::lower_bound(next.begin(), next.end(), x);
      bool found = it != next.end() && *it == x;
      if (found) next.erase(it);
      v = trie.erase(old, x);
      assert(found || v == old);
    }
    if (v == int(versions.size())) versions.push_back(next);
    else assert(versions[v] == next);
    for (int check : {old, v, int(rng() % versions.size())}) {
      const auto& a = versions[check];
      auto fail = [&](const char* op) {
        std::cerr << "seed=" << seed << " step=" << step << " version=" << check
                  << " x=" << std::uint64_t(x) << " operation=" << op << " input=";
        for (U y : a) std::cerr << std::uint64_t(y) << ',';
        std::cerr << '\n'; std::abort();
      };
      if (trie.size(check) != int(a.size()) || trie.empty(check) != a.empty()) fail("size/empty");
      if (trie.count(check, x) != std::count(a.begin(), a.end(), x)) fail("count");
      if (trie.rank(check, x) != std::lower_bound(a.begin(), a.end(), x) - a.begin()) fail("rank");
      if (trie.kth(check, -1) || trie.kth(check, a.size())) fail("kth bounds");
      for (int k = 0; k < int(a.size()); ++k) if (trie.kth(check, k) != a[k]) fail("kth");
      std::optional<U> best;
      for (U y : a) if (!best || U(y ^ x) < *best) best = U(y ^ x);
      if (trie.xor_min(check, x) != best) fail("xor_min");
    }
  }
  auto copy = trie;
  int v = copy.insert(0, mask);
  assert(copy.count(v, mask) == 1 && trie.empty(0));
}
int main(int argc, char** argv) {
  auto seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  run<std::uint64_t, 64>(seed);
  run<std::uint32_t, 30>(seed);
  run<unsigned char, 1>(seed);
}

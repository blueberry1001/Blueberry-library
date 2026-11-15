#include "blueberry/data-structure/fast-set.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <random>
#include <set>
#include <string>
#include <vector>

int main(int argc, char** argv) {
  const auto seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  std::mt19937_64 rng(seed);
  std::vector<int> sizes;
  for (int n = 0; n <= 260; ++n) sizes.push_back(n);
  for (int n : {4095, 4096, 4097}) sizes.push_back(n);
  for (int n : sizes) for (int mode = 0; mode < 5; ++mode) {
    std::string bits(n, '0');
    std::set<int> expected;
    for (int i = 0; i < n; ++i) {
      if (mode == 1 || (mode == 2 && (i & 1)) || (mode == 3 && rng() % 3 == 0) ||
          (mode == 4 && ((i / 16) & 1))) {
        bits[i] = '1'; expected.insert(i);
      }
    }
    blueberry::FastSet tree(bits);
    auto fail = [&](const char* operation, int x) {
      std::cerr << "seed=" << seed << " n=" << n << " mode=" << mode
                << " operation=" << operation << " x=" << x << '\n';
      std::exit(1);
    };
    if (tree.size() != static_cast<int>(expected.size())) fail("size", 0);
    for (int i = -1; i <= n; ++i) {
      auto it = expected.lower_bound(i), jt = expected.upper_bound(i);
      if (tree.next(i) != (it == expected.end() ? n : *it)) fail("next", i);
      if (tree.prev(i) != (jt == expected.begin() ? -1 : *std::prev(jt))) fail("prev", i);
      if (i >= 0 && i < n && tree.contains(i) != expected.contains(i)) fail("contains", i);
    }
    auto copy = tree;
    if (n && copy.insert(0) != (bits[0] == '0')) fail("copy insert", 0);
    if (tree.size() != static_cast<int>(expected.size())) fail("copy independence", 0);
  }
}

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <random>
#include <set>
#include <string>
#include <vector>
#include <atcoder/modint>
#include "blueberry/string/count-subsequences.hpp"

int main(int argc, char** argv) {
  const unsigned seed = argc > 1 ? std::strtoul(argv[1], nullptr, 10) : 1;
  std::mt19937 rng(seed);
  assert(blueberry::count_subsequences<long long>(std::string{}) == 0);
  assert(blueberry::count_subsequences<long long>(std::string("aba")) == 6);
  assert(blueberry::count_subsequences<long long>(std::vector<int>(100, -1)) == 100);
  for (int trial = 0; trial < 400; ++trial) {
    const int n = rng() % 13;
    std::vector<int> a(n);
    for (auto& x : a) x = static_cast<int>(rng() % 7) - 3;
    std::set<std::vector<int>> distinct;
    for (int mask = 1; mask < (1 << n); ++mask) {
      std::vector<int> s;
      for (int i = 0; i < n; ++i) if (mask >> i & 1) s.push_back(a[i]);
      distinct.insert(s);
    }
    const auto actual = blueberry::count_subsequences<long long>(a);
    using Mint = atcoder::static_modint<7>;
    const auto modular = blueberry::count_subsequences<Mint>(a).val();
    if (actual != static_cast<long long>(distinct.size()) || modular != actual % 7) {
      std::cerr << "seed=" << seed << " trial=" << trial << " input=";
      for (auto x : a) std::cerr << x << ',';
      std::cerr << " expected=" << distinct.size() << " actual=" << actual
                << " mod7=" << modular << '\n';
      return 1;
    }
  }
}

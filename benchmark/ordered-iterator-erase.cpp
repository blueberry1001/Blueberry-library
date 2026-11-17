#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <random>
#include <string>
#include <vector>
#include "blueberry/extra-fast/ordered-map.hpp"
using Clock = std::chrono::steady_clock;
using Key = std::uint64_t;
template <bool Map, class Tree> int run(int n, const std::string &mode, std::uint64_t seed) {
  std::vector<Key> order(n);
  std::iota(order.begin(), order.end(), Key{0});
  std::mt19937_64 rng(seed);
  if (seed) std::shuffle(order.begin(), order.end(), rng);
  Tree tree;
  const auto build_begin = Clock::now();
  for (auto key : order) {
    if constexpr (Map) tree.insert(key, key * 17 + 3);
    else tree.insert(key);
  }
  const auto build_end = Clock::now();
  std::shuffle(order.begin(), order.end(), rng);
  std::uint64_t digest = 0;
  const auto start = Clock::now();
  if (mode == "random") {
    for (Key key : order) {
      auto next = tree.erase(tree.find(key));
      if (next != tree.end()) {
        if constexpr (Map) digest += next->first + 1;
        else digest += *next + 1;
      }
    }
  } else if (mode == "reverse") {
    while (!tree.empty()) {
      auto next = tree.erase(--tree.end());
      digest += next == tree.end();
    }
  } else {
    auto it = tree.begin();
    while (it != tree.end()) {
      it = tree.erase(it);
      if (it != tree.end()) {
        if constexpr (Map) digest += it->first + 1;
        else digest += *it + 1;
        if (mode == "alternate") ++it;
      }
    }
  }
  const auto end = Clock::now();
  for (const auto &item : tree) {
    if constexpr (Map) digest ^= item.first * 0x9e3779b97f4a7c15ULL + item.second;
    else digest ^= item * 0x9e3779b97f4a7c15ULL;
  }
  const auto ms = [](auto a, auto b) { return std::chrono::duration<double, std::milli>(b - a).count(); };
  std::printf("%.6f %.6f %llu %d\n", ms(build_begin, build_end), ms(start, end),
              static_cast<unsigned long long>(digest), tree.size());
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 5) return 1;
  const int n = std::atoi(argv[1]);
  const std::string mode = argv[2], kind = argv[3];
  const std::uint64_t seed = std::strtoull(argv[4], nullptr, 10);
  if (kind == "map") return run<true, blueberry::extra_fast::OrderedMap<Key, Key>>(n, mode, seed);
  return run<false, blueberry::extra_fast::OrderedSet<Key>>(n, mode, seed);
}

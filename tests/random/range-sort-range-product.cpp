#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "blueberry/data-structure/range-sort-range-product.hpp"
std::string op(std::string a, std::string b) { return a + b; }
std::string e() { return {}; }
int main(int argc, char** argv) {
  unsigned seed = argc > 1 ? std::stoul(argv[1]) : 1;
  std::mt19937 rng(seed);
  for (int n : {0, 1, 2, 3, 31, 32, 33, 100}) {
    std::vector<std::pair<unsigned, std::string>> a(n);
    unsigned next = 0;
    for (auto& x : a) x = {next++ * 2654435761u, std::string(1, char('a' + rng() % 26))};
    std::shuffle(a.begin(), a.end(), rng);
    blueberry::RangeSortRangeProduct<std::string, op, e> ds(a);
    for (int step = 0; step < 5000; ++step) {
      int l = rng() % (n + 1), r = rng() % (n + 1);
      if (l > r) std::swap(l, r);
      int type = rng() % 4;
      if (type <= 1) {
        ds.sort(l, r, type != 0);
        std::sort(a.begin() + l, a.begin() + r, [type](const auto& x, const auto& y) {
          return type ? x.first > y.first : x.first < y.first;
        });
      } else if (type == 2 && n) {
        int p = rng() % n;
        a[p] = {next++ * 2654435761u, std::string(1, char('a' + rng() % 26))};
        ds.set(p, a[p].first, a[p].second);
      }
      std::string expected, whole;
      for (int i = 0; i < n; ++i) {
        if (l <= i && i < r) expected += a[i].second;
        whole += a[i].second;
        if (ds.get(i) != a[i]) {
          std::cerr << "seed=" << seed << " n=" << n << " step=" << step << " get=" << i << '\n';
          return 1;
        }
      }
      if (ds.prod(l, r) != expected || ds.all_prod() != whole) {
        std::cerr << "seed=" << seed << " n=" << n << " step=" << step << " l=" << l << " r=" << r << " expected=" << expected << " actual=" << ds.prod(l,r) << '\n';
        return 1;
      }
    }
    auto copied = ds;
    auto moved = std::move(copied);
    if (moved.all_prod() != ds.all_prod()) return 1;
  }
  blueberry::RangeSortRangeProduct<std::string, op, e, std::uint64_t> high(
      {{~std::uint64_t{0}, "z"}, {0, "a"}, {std::uint64_t{1} << 63, "m"}});
  high.sort(0, 3);
  if (high.prod(0, 3) != "amz") return 1;
  high.sort(0, 3, true);
  if (high.prod(0, 3) != "zma") return 1;
}

#include <chrono>
#include <iostream>
#include <map>
#include <random>
#include <vector>
#include <atcoder/modint>
#include "blueberry/string/count-subsequences.hpp"

using Mint = atcoder::modint998244353;
// Independent map-state candidate: one last-prefix value per symbol.
Mint map_count(const std::vector<int>& a) {
  std::map<int, Mint> previous;
  Mint total = 1;
  for (int x : a) {
    const Mint old = total;
    total += total - previous[x];
    previous[x] = old;
  }
  return total - 1;
}
int main() {
  std::mt19937 rng(20260918);
  for (const int alphabet : {8, 500000}) {
    std::vector<int> a(500000);
    for (auto& x : a) x = rng() % alphabet;
    int expected = -1;
    for (int run = 0; run < 5; ++run) {
      for (int method = 0; method < 2; ++method) {
        const auto start = std::chrono::steady_clock::now();
        const int answer = (method ? map_count(a) : blueberry::count_subsequences<Mint>(a)).val();
        const double ms = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - start).count();
        if (expected != -1 && expected != answer) return 1;
        expected = answer;
        std::cout << "n=" << a.size() << " alphabet=" << alphabet << " run=" << run
                  << " candidate=" << (method ? "map" : "compressed")
                  << " ms=" << ms << " checksum=" << answer << '\n';
      }
    }
  }
}

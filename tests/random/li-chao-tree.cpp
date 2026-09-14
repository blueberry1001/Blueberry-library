#include "blueberry/data-structure/li-chao-tree.hpp"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <string>

int main(int argc, char** argv) {
  const auto seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  std::mt19937_64 random(seed);
  using Tree = blueberry::LiChaoTree<long long>;
  Tree empty({});
  assert(empty.size() == 0);
  empty.add_line(3, 4);
  empty.add_segment(-10, 10, 2, 3);
  const auto low = std::numeric_limits<long long>::lowest();
  const auto high = std::numeric_limits<long long>::max();
  Tree extreme({high, low, 0, high});
  assert(extreme.size() == 3);
  assert(!extreme.query(high));
  extreme.add_line(0, high);
  assert(extreme.query(low) == high);
  extreme.add_segment(low, high, 0, low);
  assert(extreme.query(low) == low);
  assert(extreme.query(0) == low);
  assert(extreme.query(high) == high);
  Tree copy = extreme;
  copy.add_line(0, low);
  assert(copy.query(high) == low && extreme.query(high) == high);
  blueberry::LiChaoTree<int> single({7, 7});
  single.add_segment(7, 7, 0, -5);
  assert(!single.query(7));
  single.add_segment(6, 8, 2, 1);
  assert(single.query(7) == 15);

  for (int trial = 0; trial < 60; ++trial) {
    const int n = 1 + static_cast<int>(random() % 40);
    std::vector<long long> xs(n);
    for (auto& x : xs) x = static_cast<long long>(random() % 101) - 50;
    Tree tree(xs);
    std::sort(xs.begin(), xs.end());
    xs.erase(std::unique(xs.begin(), xs.end()), xs.end());
    assert(tree.size() == static_cast<int>(xs.size()));
    std::vector<std::optional<long long>> expected(xs.size());
    std::vector<std::string> history;
    for (int step = 0; step < 250; ++step) {
      long long l = static_cast<long long>(random() % 141) - 70;
      long long r = static_cast<long long>(random() % 141) - 70;
      if (l > r) std::swap(l, r);
      const long long a = static_cast<long long>(random() % 61) - 30;
      const long long b = static_cast<long long>(random() % 201) - 100;
      const bool whole = random() % 4 == 0;
      if (whole) tree.add_line(a, b);
      else tree.add_segment(l, r, a, b);
      history.push_back(std::string(whole ? "line " : "segment ") +
                        std::to_string(l) + " " + std::to_string(r) + " " +
                        std::to_string(a) + " " + std::to_string(b));
      for (std::size_t i = 0; i < xs.size(); ++i) {
        if (whole || (l <= xs[i] && xs[i] < r)) {
          const auto y = a * xs[i] + b;
          if (!expected[i] || y < *expected[i]) expected[i] = y;
        }
        const auto actual = tree.query(xs[i]);
        if (actual != expected[i]) {
          std::cerr << "seed=" << seed << " trial=" << trial << " step=" << step
                    << " x=" << xs[i] << " expected="
                    << (expected[i] ? std::to_string(*expected[i]) : "empty")
                    << " actual=" << (actual ? std::to_string(*actual) : "empty") << '\n';
          std::cerr << "xs:";
          for (auto x : xs) std::cerr << ' ' << x;
          std::cerr << '\n';
          for (const auto& operation : history) std::cerr << operation << '\n';
          return 1;
        }
      }
    }
  }
}

#include "blueberry/data-structure/dynamic-li-chao-tree.hpp"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <optional>
#include <random>
#include <string>
#include <utility>
#include <vector>

int main(int argc, char** argv) {
  const auto seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  std::mt19937_64 random(seed);
  using Tree = blueberry::DynamicLiChaoTree<>;
  Tree empty(3, 3);
  empty.add_line(1, 2);
  empty.add_segment(-10, 10, 1, 2);
  Tree single(-7, -6);
  assert(!single.query(-7));
  single.add_segment(-7, -7, 0, -10);
  assert(!single.query(-7));
  single.add_segment(-9, -6, 2, 1);
  assert(single.query(-7) == -13);
  single.add_line(2, 1);
  single.add_line(2, 2);
  assert(single.query(-7) == -13);

  const auto low = std::numeric_limits<long long>::lowest();
  const auto high = std::numeric_limits<long long>::max();
  Tree extreme(low, high);
  assert(!extreme.query(low) && !extreme.query(high - 1));
  extreme.add_line(0, high);
  assert(extreme.query(low) == high && extreme.query(high - 1) == high);
  extreme.add_segment(low, low + 1, 0, low);
  extreme.add_segment(high - 1, high, 0, -1);
  assert(extreme.query(low) == low);
  assert(extreme.query(low + 1) == high);
  assert(extreme.query(high - 1) == -1);
  // Evaluation outside a segment's support would overflow: never do it.
  extreme.add_segment(-2, 3, high / 4, 0);
  assert(extreme.query(-2) == -(high / 4) * 2);
  assert(extreme.query(2) == (high / 4) * 2);
  Tree copied = extreme;
  copied.add_line(0, low);
  assert(copied.query(1) == low && extreme.query(1) == high / 4);
  Tree assigned(0, 1);
  assigned = extreme;
  assigned.add_line(0, low);
  assert(assigned.query(1) == low && extreme.query(1) == high / 4);
  Tree moved(std::move(copied));
  assert(moved.query(1) == low && !copied.query(1));
  copied.add_segment(0, 2, 0, 9);
  assert(copied.query(1) == 9 && moved.query(1) == low);
  assigned = std::move(copied);
  assert(assigned.query(1) == 9 && !copied.query(1));
  copied.add_line(0, 8);
  auto* self = &copied;
  copied = std::move(*self);
  assert(copied.query(1) == 8);
  blueberry::DynamicLiChaoTree<signed char> small(-4, 5);
  small.add_line(2, -1);
  assert(small.query(-4) == -9 && small.query(4) == 7);
  // Unsigned narrow operands promote to int before subtraction; conversion
  // back to the unsigned coordinate width must restore modular subtraction.
  const auto small_low = std::numeric_limits<signed char>::min();
  const auto small_high = std::numeric_limits<signed char>::max();
  blueberry::DynamicLiChaoTree<signed char> full_small(small_low, small_high);
  full_small.add_line(0, small_high);
  for (int x = small_low; x < small_high; ++x) full_small.add_segment(x, x + 1, 1, 0);
  for (int x = small_low; x < small_high; ++x) assert(full_small.query(x) == x);
  blueberry::DynamicLiChaoTree<int> negative(-100, -90);
  negative.add_line(-1, -10);
  assert(negative.query(-99) == 89);
  // Every line is useful somewhere: exercise growth through many reallocations.
  Tree envelope(-2048, 2049);
  for (long long x = -2048; x <= 2048; ++x) envelope.add_line(-2 * x, x * x);
  for (long long x = -2048; x <= 2048; ++x) assert(envelope.query(x) == -x * x);

  for (int trial = 0; trial < 60; ++trial) {
    const int left = static_cast<int>(random() % 100) - 100;
    const int right = left + 1 + static_cast<int>(random() % 100);
    Tree tree(left, right);
    std::vector<std::optional<long long>> expected(right - left);
    std::vector<std::string> history;
    for (int step = 0; step < 250; ++step) {
      long long l = static_cast<long long>(random() % 250) - 150;
      long long r = static_cast<long long>(random() % 250) - 150;
      if (l > r) std::swap(l, r);
      const long long a = static_cast<long long>(random() % 61) - 30;
      const long long b = static_cast<long long>(random() % 201) - 100;
      const bool whole = random() % 4 == 0;
      if (whole) tree.add_line(a, b);
      else tree.add_segment(l, r, a, b);
      history.push_back(std::string(whole ? "line " : "segment ") +
                        std::to_string(l) + " " + std::to_string(r) + " " +
                        std::to_string(a) + " " + std::to_string(b));
      for (int x = left; x < right; ++x) {
        auto& answer = expected[x - left];
        if (whole || (l <= x && x < r)) {
          const auto y = a * x + b;
          if (!answer || y < *answer) answer = y;
        }
        const auto actual = tree.query(x);
        if (actual != answer) {
          std::cerr << "seed=" << seed << " trial=" << trial << " step=" << step
                    << " domain=[" << left << ',' << right << ") x=" << x
                    << " expected=" << (answer ? std::to_string(*answer) : "empty")
                    << " actual=" << (actual ? std::to_string(*actual) : "empty") << '\n';
          for (const auto& operation : history) std::cerr << operation << '\n';
          return 1;
        }
      }
    }
  }
}

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <type_traits>
#include <vector>
#include "blueberry/data-structure/dynamic-fenwick-tree.hpp"

// Deliberately no assignment, -=, +, ordering, or implicit numeric conversion.
struct Additive {
  long long value = 0;
  Additive() = default;
  explicit Additive(long long x) : value(x) {}
  Additive(const Additive&) = default;
  Additive& operator=(const Additive&) = delete;
  Additive& operator+=(const Additive& rhs) { value += rhs.value; return *this; }
  Additive& operator-=(const Additive&) = delete;
  friend Additive operator-(const Additive& lhs, const Additive& rhs) {
    return Additive(lhs.value - rhs.value);
  }
};
static_assert(std::is_copy_constructible_v<Additive>);
static_assert(!std::is_copy_assignable_v<Additive>);
static_assert(!std::is_move_assignable_v<Additive>);
static std::uint64_t checksum = 1469598103934665603ULL;
static long long checks = 0;
static void expect(long long actual, long long expected, const char* label) {
  if (actual != expected) {
    std::cerr << label << " expected=" << expected << " actual=" << actual << '\n';
    std::abort();
  }
  ++checks;
  checksum = (checksum ^ std::uint64_t(actual)) * 1099511628211ULL;
}

template<class Coord>
void test_coord() {
  static_assert(std::is_signed_v<Coord>);
  const long long limit = std::numeric_limits<Coord>::max();
  std::vector<long long> sizes{0, 1, 2, 3, 7, 8, 9, limit - 1, limit};
  std::sort(sizes.begin(), sizes.end());
  sizes.erase(std::unique(sizes.begin(), sizes.end()), sizes.end());
  for (long long n : sizes) {
    blueberry::DynamicFenwickTree<Additive, Coord> tree(static_cast<Coord>(n));
    expect(tree.size(), n, "size");
    expect(tree.pref(Coord(0)).value, 0, "empty prefix");
    expect(tree.sum(Coord(0), static_cast<Coord>(n)).value, 0, "initial sum");
    std::vector<long long> positions;
    const auto add_position = [&](long long p) { if (0 <= p && p < n) positions.push_back(p); };
    add_position(0); add_position(n - 1); add_position(n - 2);
    for (unsigned long long power = 1; power <= static_cast<unsigned long long>(limit); power <<= 1) {
      for (int delta : {-2, -1, 0, 1, 2}) add_position(static_cast<long long>(power) + delta);
    }
    std::sort(positions.begin(), positions.end());
    positions.erase(std::unique(positions.begin(), positions.end()), positions.end());
    std::map<long long, long long> expected;
    for (long long p : positions) expect(tree.get(static_cast<Coord>(p)).value, 0, "missing cell");
    const auto check_points = [&] {
      for (long long p : positions) {
        const auto found = expected.find(p);
        const long long value = found == expected.end() ? 0 : found->second;
        expect(tree.get(static_cast<Coord>(p)).value, value, "point value");
      }
    };
    for (std::size_t i = 0; i < positions.size(); ++i) {
      const long long p = positions[i];
      const long long delta = static_cast<long long>(i % 23) - 11;
      tree.add(static_cast<Coord>(p), Additive(delta)); expected[p] += delta;
      if (i % 11 == 0) check_points();
    }
    check_points();
    for (long long p : positions) {
      long long prefix = 0;
      for (const auto& [q, value] : expected) if (q < p + 1) prefix += value;
      expect(tree.pref(static_cast<Coord>(p + 1)).value, prefix, "prefix oracle");
      expect(tree.sum(static_cast<Coord>(p), static_cast<Coord>(p + 1)).value,
             expected[p], "unit range oracle");
    }
    // Nonassignable T remains usable in the container's copy constructor.
    const auto copied = tree;
    if (!positions.empty()) {
      const long long p = positions.back(), original = expected[p];
      tree.add(static_cast<Coord>(p), Additive(-original));
      expect(tree.get(static_cast<Coord>(p)).value, 0, "cancel to zero");
      expect(copied.get(static_cast<Coord>(p)).value, original, "independent copy");
    }
  }
}

int main() {
  test_coord<signed char>();
  test_coord<short>();
  test_coord<int>();
  test_coord<long long>();
  std::cout << "dynamic-fenwick checks=" << checks << " checksum=" << checksum << '\n';
}

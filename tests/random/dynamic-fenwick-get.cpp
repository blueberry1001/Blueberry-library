#include "blueberry/data-structure/dynamic-fenwick-tree.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>
#include <atcoder/modint>

using Mint = atcoder::modint998244353;
using Oracle = std::map<long long, long long>;

// The additive API must not acquire assignment, -=, +, or ordering requirements.
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

std::uint64_t seed, checks = 0;
const char* case_label = "";
long long case_size = 0;
int step = -1;
const Oracle* current_values = nullptr;

long long numeric(long long x) { return x; }
long long numeric(const Additive& x) { return x.value; }
long long numeric(Mint x) { return x.val(); }

void expect(long long actual, long long expected, const char* operation,
            long long left = 0, long long right = 0) {
  ++checks;
  if (actual == expected) return;
  std::cerr << "seed=" << seed << " case=" << case_label << " n=" << case_size
            << " step=" << step << " operation=" << operation
            << " left=" << left << " right=" << right
            << " expected=" << expected << " actual=" << actual << '\n';
  if (current_values) {
    for (auto [p, value] : *current_values) std::cerr << p << ' ' << value << '\n';
  }
  std::abort();
}

// Plain enumeration, independent of the Fenwick cell decomposition.
long long oracle_sum(const Oracle& values, long long left, long long right) {
  long long result = 0;
  for (auto [p, value] : values) if (left <= p && p < right) result += value;
  return result;
}

template<class T, class Coord>
void mixed_case(long long n, std::mt19937_64& rng) {
  using Tree = blueberry::DynamicFenwickTree<T, Coord>;
  case_size = n; step = -1;
  Tree tree(static_cast<Coord>(n));
  const auto& view = tree;
  Oracle values;
  current_values = &values;
  expect(view.size(), n, "size");
  expect(numeric(view.pref(Coord(0))), 0, "empty prefix");
  expect(numeric(view.sum(Coord(0), static_cast<Coord>(n))), 0, "initial sum");
  if (n == 0) { current_values = nullptr; return; }

  std::vector<long long> points{0, n - 1};
  for (std::uint64_t power = 1; power <= static_cast<std::uint64_t>(n); power <<= 1) {
    for (int delta : {-2, -1, 0, 1, 2}) {
      const long long p = static_cast<long long>(power) + delta;
      if (0 <= p && p < n) points.push_back(p);
    }
  }
  std::sort(points.begin(), points.end());
  points.erase(std::unique(points.begin(), points.end()), points.end());
  const auto check_point = [&](long long p) {
    expect(numeric(view.get(static_cast<Coord>(p))),
           numeric(T(oracle_sum(values, p, p + 1))), "get", p);
  };
  const auto position = [&] {
    return rng() % 2 ? points[rng() % points.size()]
                     : static_cast<long long>(rng() % static_cast<std::uint64_t>(n));
  };
  const auto endpoint = [&] {
    return static_cast<long long>(rng() % (static_cast<std::uint64_t>(n) + 1));
  };
  for (long long p : points) check_point(p);  // Unstored cells must read as zero.
  tree.add(Coord(0), T(-7)); values[0] = -7; check_point(0);

  for (step = 0; step < 180; ++step) {
    const long long p = position();
    const unsigned operation = rng() % 6;
    if (operation < 3) {
      const long long delta = step % 11 == 0 ? 0 : static_cast<long long>(rng() % 61) - 30;
      tree.add(static_cast<Coord>(p), T(delta)); values[p] += delta;
      check_point(p); check_point(position());
    } else if (operation == 3) {
      check_point(p);
    } else if (operation == 4) {
      const long long right = endpoint();
      expect(numeric(view.pref(static_cast<Coord>(right))),
             numeric(T(oracle_sum(values, 0, right))), "pref", 0, right);
    } else {
      long long left = endpoint(), right = endpoint();
      if (left > right) std::swap(left, right);
      expect(numeric(view.sum(static_cast<Coord>(left), static_cast<Coord>(right))),
             numeric(T(oracle_sum(values, left, right))), "sum", left, right);
    }
    if (step % 17 == 0) for (long long q : points) check_point(q);
    expect(numeric(view.sum(static_cast<Coord>(p), static_cast<Coord>(p))), 0, "empty sum", p, p);
  }
  for (long long p : points) check_point(p);
  expect(numeric(view.pref(static_cast<Coord>(n))),
         numeric(T(oracle_sum(values, 0, n))), "full prefix", 0, n);

  // Also instantiate copy construction with the nonassignable additive type.
  const auto copied = tree;
  const long long old = values[0];
  tree.add(Coord(0), T(-old)); values[0] = 0;
  check_point(0);
  expect(numeric(copied.get(Coord(0))), numeric(T(old)), "independent copy", 0);
  current_values = nullptr;
}

template<class T, class Coord>
void coordinate_cases(std::mt19937_64& rng, const char* label) {
  static_assert(std::is_signed_v<Coord>);
  case_label = label;
  const long long limit = std::numeric_limits<Coord>::max();
  for (long long n : {0LL, 1LL, 9LL, limit - 1, limit}) mixed_case<T, Coord>(n, rng);
}

void ownership_cases() {
  using Tree = blueberry::DynamicFenwickTree<long long>;
  case_label = "ownership"; case_size = 37; step = -1;
  Tree original(37); original.add(0, -4); original.add(7, 9); original.add(36, 2);
  Tree copied(original); copied.add(0, 8);
  expect(original.get(0), -4, "copy leaves source");
  expect(copied.get(0), 4, "copy update");
  Tree assigned(1); assigned = original; assigned.add(7, -9);
  expect(original.get(7), 9, "assignment leaves source");
  expect(assigned.get(7), 0, "assignment update");
  Tree moved(std::move(assigned));
  expect(assigned.size(), 0, "move constructor source size");
  expect(assigned.pref(0), 0, "move constructor source prefix");
  expect(moved.get(36), 2, "move constructor target");
  Tree target(2); target.add(1, 100); target = std::move(moved);
  expect(moved.size(), 0, "move assignment source size");
  expect(moved.sum(0, 0), 0, "move assignment source sum");
  expect(target.size(), 37, "move assignment target size");
  target.add(36, -5);
  expect(target.get(36), -3, "move target update");
  expect(original.get(36), 2, "move leaves original");
  expect(copied.get(36), 2, "move leaves other copy");
}

void signed_extreme_cases() {
  case_label = "signed extremes"; case_size = 8; step = -1;
  blueberry::DynamicFenwickTree<long long> high(8), low(8);
  // Accumulating the removed prefix is safe. Subtracting its cells one by one
  // from the covering cell would overflow at the first subtraction.
  high.add(6, -1); high.add(5, 1); high.add(7, std::numeric_limits<long long>::max());
  low.add(6, 1); low.add(5, -1); low.add(7, std::numeric_limits<long long>::min());
  expect(high.get(7), std::numeric_limits<long long>::max(), "maximum point", 7);
  expect(low.get(7), std::numeric_limits<long long>::min(), "minimum point", 7);
  expect(high.pref(7), 0, "maximum preceding prefix", 0, 7);
  expect(low.pref(7), 0, "minimum preceding prefix", 0, 7);
}

void search_cases(std::mt19937_64& rng) {
  case_label = "nonnegative lower_bound";
  for (int n : {0, 1, 9, 65}) {
    case_size = n; step = -1;
    blueberry::DynamicFenwickTree<long long, int> tree(n);
    std::vector<long long> values(n);
    expect(tree.lower_bound(1), n, "zero lower_bound");
    expect(tree.lower_bound(0), 0, "nonpositive lower_bound");
    if (n == 0) continue;
    for (step = 0; step < 120; ++step) {
      const int p = rng() % n;
      const long long next = rng() % 8;
      tree.add(p, next - values[p]); values[p] = next;
      long long total = 0;
      for (int i = 0; i < n; ++i) {
        total += values[i]; expect(tree.get(i), values[i], "get with search", i);
      }
      for (long long weight : {-1LL, 0LL, 1LL, total, total + 1,
                               static_cast<long long>(rng() % static_cast<std::uint64_t>(total + 2))}) {
        int expected = 0;
        long long sum = 0;
        if (weight > 0) while (expected < n && sum + values[expected] < weight) sum += values[expected++];
        expect(tree.lower_bound(weight), expected, "lower_bound", weight);
      }
    }
  }
}

int main(int argc, char** argv) {
  seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 20261003;
  std::mt19937_64 rng(seed);
  coordinate_cases<long long, signed char>(rng, "integer/signed char");
  coordinate_cases<long long, short>(rng, "integer/short");
  coordinate_cases<long long, int>(rng, "integer/int");
  coordinate_cases<long long, long long>(rng, "integer/long long");
  coordinate_cases<Additive, signed char>(rng, "nonassignable/signed char");
  coordinate_cases<Additive, long long>(rng, "nonassignable/long long");
  coordinate_cases<Mint, int>(rng, "modint/int");
  coordinate_cases<Mint, long long>(rng, "modint/long long");
  ownership_cases(); signed_extreme_cases(); search_cases(rng);
  std::cout << "seed=" << seed << " checks=" << checks << '\n';
}

#include "blueberry/extra-fast/bitset.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <utility>
#include <vector>

using blueberry::extra_fast::Bitset;
using Bits = std::vector<bool>;
unsigned long long seed;
int step;

void check(const Bitset& actual, const Bits& expected, const char* operation) {
  bool correct = actual.size() == static_cast<int>(expected.size());
  int count = 0;
  for (int p = 0; p < static_cast<int>(expected.size()); ++p) {
    count += expected[p];
    correct = correct && actual.test(p) == expected[p];
  }
  if (correct && actual.count() == count) return;
  std::cerr << "seed=" << seed << " step=" << step << " operation=" << operation
            << " n=" << expected.size() << " expected=";
  for (bool bit : expected) std::cerr << bit;
  std::cerr << " actual=";
  for (int p = 0; p < actual.size(); ++p) std::cerr << actual.test(p);
  std::cerr << " count_expected=" << count << " count_actual=" << actual.count() << '\n';
  std::exit(1);
}

void shift(Bits& bits, int amount) {
  const Bits old = bits;
  for (int p = amount; p < static_cast<int>(bits.size()); ++p)
    bits[p] = old[p] || old[p - amount];
}

int main(int argc, char** argv) {
  seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  std::mt19937_64 random(seed);
  Bitset empty;
  check(empty, {}, "default constructor");
  for (bool simd : {false, true}) {
    for (int n : {0, 1, 2, 7, 63, 64, 65, 127, 128, 129, 191, 192, 193,
                  255, 256, 257, 319, 320, 321, 511, 512, 513,
                  1023, 1024, 1025, 4097}) {
      Bitset tree(n, simd);
      Bits expected(n, false);
      check(tree, expected, "size constructor");
      if (tree.uses_simd() != (simd && Bitset::simd_available())) return 2;
      for (int p = 0; p < n; ++p) {
        const bool value = random() & 1;
        tree.set(p, value); expected[p] = value;
      }
      // Shift boundaries, tail bits, and exact 256-bit vector boundaries.
      for (int k : {0, 1, 2, 63, 64, 65, 127, 128, 255, 256, n, n + 1,
                    std::numeric_limits<int>::max()}) {
        Bitset copy(tree);
        Bits oracle(expected);
        copy.shift_or(k); shift(oracle, k);
        check(copy, oracle, "shift boundary");
      }
      for (step = 0; step < 240; ++step) {
        const int operation = random() % 8;
        if (operation == 0 && n) {
          const int p = random() % n;
          tree.set(p); expected[p] = true;
        } else if (operation == 1 && n) {
          const int p = random() % n;
          tree.reset(p); expected[p] = false;
        } else if (operation == 2) {
          const int k = random() % (n + 70);
          tree.shift_or(k); shift(expected, k);
        } else if (operation < 6) {
          Bitset other(n, !simd);
          Bits operand(n);
          for (int p = 0; p < n; ++p) {
            operand[p] = random() & 1;
            other.set(p, operand[p]);
          }
          if (operation == 3) tree.or_with(other);
          else if (operation == 4) tree.xor_with(other);
          else tree.and_with(other);
          for (int p = 0; p < n; ++p) {
            if (operation == 3) expected[p] = expected[p] || operand[p];
            else if (operation == 4) expected[p] = expected[p] != operand[p];
            else expected[p] = expected[p] && operand[p];
          }
        } else if (operation == 6) {
          tree.or_with(tree); tree.and_with(tree);
        } else {
          tree.xor_with(tree); expected.assign(n, false);
        }
        check(tree, expected, "random operation");
      }
      Bitset copied(tree), assigned;
      assigned = tree;
      auto* self = &assigned;
      assigned = *self;
      Bitset moved(std::move(copied)), move_assigned;
      move_assigned = std::move(assigned);
      check(moved, expected, "move construction");
      check(move_assigned, expected, "move assignment");
      moved.clear();
      check(moved, Bits(n, false), "clear");
      check(tree, expected, "copy storage independent");
      // Subset-sum DP exercises repeated overlapping in-place source loads.
      tree.clear(); expected.assign(n, false);
      if (n) { tree.set(0); expected[0] = true; }
      for (int k : {1, 63, 64, 65, 255, 256, 3, 15, 4, 1}) {
        tree.shift_or(k); shift(expected, k);
        check(tree, expected, "subset sum");
      }
    }
  }
}

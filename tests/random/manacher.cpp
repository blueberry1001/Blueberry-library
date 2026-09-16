#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "blueberry/string/manacher.hpp"

template <class Sequence>
void check(const Sequence& input, unsigned long long seed) {
  const int n = static_cast<int>(input.size());
  std::vector<int> expected(n == 0 ? 0 : 2 * n - 1);
  // Enumerate substrings independently of the center-based library algorithm.
  for (int l = 0; l < n; ++l) {
    for (int r = l + 1; r <= n; ++r) {
      bool palindrome = true;
      for (int k = 0; k < (r - l) / 2; ++k) {
        if (!(input[l + k] == input[r - 1 - k])) palindrome = false;
      }
      if (palindrome) expected[l + r - 1] = std::max(expected[l + r - 1], r - l);
    }
  }
  const auto actual = blueberry::manacher(input);
  if (expected != actual) {
    std::cerr << "seed=" << seed << " input=";
    for (const auto& value : input) std::cerr << static_cast<int>(value) << ',';
    std::cerr << "\nexpected=";
    for (int value : expected) std::cerr << value << ',';
    std::cerr << "\nactual=";
    for (int value : actual) std::cerr << value << ',';
    std::cerr << '\n';
    std::exit(1);
  }
}

struct Symbol {
  int value;
  bool operator==(const Symbol&) const = default;
  explicit operator int() const { return value; }
};

int main(int argc, char** argv) {
  const auto seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  std::mt19937_64 random(seed);
  for (const std::string text : {"", "a", "aa", "ab", "aba", "abba", "aaaaaaa",
                                 "abababab", "abacabadabacaba"}) {
    check(text, seed);
  }
  check(std::string("\0\xff\0", 3), seed);
  check(std::vector<Symbol>{{-1}, {2}, {-1}, {-1}, {2}, {-1}}, seed);
  for (int n = 0; n <= 10; ++n) {
    for (int mask = 0; mask < (1 << n); ++mask) {
      std::vector<int> input(n);
      for (int i = 0; i < n; ++i) input[i] = (mask >> i) & 1;
      check(input, seed);
    }
  }
  for (int trial = 0; trial < 300; ++trial) {
    const int n = static_cast<int>(random() % 61);
    const int alphabet = 1 + static_cast<int>(random() % 8);
    std::vector<int> input(n);
    for (int& value : input) value = static_cast<int>(random() % alphabet) - 3;
    check(input, seed);
  }
  const int n = 200000;
  const auto length = blueberry::manacher(std::string(n, 'a'));
  for (int i = 0; i < 2 * n - 1; ++i) {
    assert(length[i] == std::min(i + 1, 2 * n - 1 - i));
  }
}

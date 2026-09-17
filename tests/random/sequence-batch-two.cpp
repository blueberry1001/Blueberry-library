#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>
#include "blueberry/data-structure/disjoint-sparse-table.hpp"
#include "blueberry/math/longest-increasing-subsequence.hpp"
#include "blueberry/string/prefix-function.hpp"
#include "blueberry/graph/cartesian-tree.hpp"

unsigned seed = 20260918;
void check(bool ok, const std::vector<int>& a, const char* operation) {
  if (ok) return;
  std::cerr << "seed=" << seed << " operation=" << operation << " input=";
  for (int x : a) std::cerr << x << ',';
  std::cerr << " expected=oracle actual=mismatch\n";
  std::exit(1);
}
void tree_oracle(const std::vector<int>& a, int l, int r, int parent,
                 std::vector<int>& p) {
  if (l == r) return;
  int m = static_cast<int>(std::min_element(a.begin() + l, a.begin() + r) - a.begin());
  p[m] = parent == -1 ? m : parent;
  tree_oracle(a, l, m, m, p);
  tree_oracle(a, m + 1, r, m, p);
}
int main(int argc, char** argv) {
  if (argc > 1) seed = static_cast<unsigned>(std::stoul(argv[1]));
  std::mt19937 rng(seed);
  for (int trial = 0; trial < 2000; ++trial) {
    int n = trial % 65;
    std::vector<int> a(n);
    for (int& x : a) x = static_cast<int>(rng() % 7) - 3;
    if (n > 1 && trial % 7 == 0) { a[0] = std::numeric_limits<int>::min(); a[1] = std::numeric_limits<int>::max(); }
    auto ids = blueberry::longest_increasing_subsequence(a);
    std::vector<int> dp(n, 1);
    int best = 0;
    for (int i = 0; i < n; ++i) {
      for (int j = 0; j < i; ++j) if (a[j] < a[i]) dp[i] = std::max(dp[i], dp[j] + 1);
      best = std::max(best, dp[i]);
    }
    check(static_cast<int>(ids.size()) == best, a, "lis length");
    for (std::size_t i = 1; i < ids.size(); ++i)
      check(ids[i - 1] < ids[i] && a[ids[i - 1]] < a[ids[i]], a, "lis witness");
    std::vector<int> parent(n);
    tree_oracle(a, 0, n, -1, parent);
    check(parent == blueberry::cartesian_tree(a), a, "cartesian");
    auto pi = blueberry::prefix_function(a);
    for (int i = 0; i < n; ++i) {
      int expected = 0;
      for (int k = 1; k <= i; ++k)
        if (std::equal(a.begin(), a.begin() + k, a.begin() + i + 1 - k)) expected = k;
      check(pi[i] == expected, a, "prefix");
    }
    const int m = trial % 13;
    std::vector<int> pattern(m);
    for (int& x : pattern) x = static_cast<int>(rng() % 7) - 3;
    std::vector<int> matches;
    for (int i = 0; i + m <= n; ++i)
      if (std::equal(pattern.begin(), pattern.end(), a.begin() + i)) matches.push_back(i);
    check(matches == blueberry::kmp_search(a, pattern), a, "kmp");
    std::vector<std::string> words;
    for (int x : a) words.push_back(std::to_string(x) + ',');
    blueberry::DisjointSparseTable table(words, [](const std::string& x, const std::string& y) { return x + y; });
    check(table.size() == n, a, "dst size");
    for (int l = 0; l < n; ++l) {
      std::string expected;
      for (int r = l + 1; r <= n; ++r) {
        expected += words[r - 1];
        check(table.prod(l, r) == expected, a, "dst noncommutative");
      }
    }
  }
  assert((blueberry::kmp_search(std::string("aaaa"), std::string("aa")) == std::vector<int>{0, 1, 2}));
  const std::string nul(4, '\0');
  assert((blueberry::kmp_search(nul, std::string(2, '\0')) == std::vector<int>{0, 1, 2}));
}

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <vector>
#include "blueberry/math/longest-increasing-subsequence.hpp"

struct MoveOnly {
  int value;
  explicit MoveOnly(int v) : value(v) {}
  MoveOnly(MoveOnly&&) = default;
  MoveOnly(const MoveOnly&) = delete;
  bool operator<(const MoveOnly& other) const { return value < other.value; }
};

template<class T> void check(const std::vector<T>& a) {
  const int n = static_cast<int>(a.size());
  std::vector<int> dp(n, 1); int best = 0;
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < i; ++j) if (a[j] < a[i]) dp[i] = std::max(dp[i], dp[j] + 1);
    best = std::max(best, dp[i]);
  }
  auto ids = blueberry::longest_increasing_subsequence(a);
  assert(static_cast<int>(ids.size()) == best);
  for (int i = 0; i < best; ++i) {
    assert(0 <= ids[i] && ids[i] < n);
    if (i) assert(ids[i-1] < ids[i] && a[ids[i-1]] < a[ids[i]]);
  }
}

int main() {
  std::mt19937_64 gen(817263);
  for (int rep = 0; rep < 5000; ++rep) {
    int n = gen() % 45;
    std::vector<int> a(n); std::vector<std::uint64_t> b(n);
    for (int i = 0; i < n; ++i) { a[i] = static_cast<int>(gen() % 21) - 10; b[i] = gen() % 13; }
    check(a); check(b);
  }
  using L = std::numeric_limits<std::int64_t>;
  check(std::vector<std::int64_t>{L::min(), L::max(), 0, L::min(), L::max()});
  check(std::vector<bool>{true, false, true, false});
  check(std::vector<std::string>{"d", "b", "c", "b", "a"});
  std::vector<MoveOnly> move_only;
  for (int x : {3, 1, 2, 5, 4}) move_only.emplace_back(x);
  check(move_only);
}

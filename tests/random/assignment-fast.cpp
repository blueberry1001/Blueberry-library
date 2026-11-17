#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <vector>
#include "blueberry/graph/assignment.hpp"
#include "../../docs/development/measurements/assignment-shortest-20261117/baseline.hpp.txt"
using namespace std;
struct Scalar {
  long long x = 0;
  Scalar() = default;
  Scalar(long long value) : x(value) {}
  Scalar& operator+=(Scalar b) { x += b.x; return *this; }
  Scalar& operator-=(Scalar b) { x -= b.x; return *this; }
  friend Scalar operator-(Scalar a, Scalar b) { return a.x - b.x; }
  friend bool operator<(Scalar a, Scalar b) { return a.x < b.x; }
};
namespace std {
template<> struct numeric_limits<Scalar> : numeric_limits<long long> {
  static Scalar max() noexcept { return numeric_limits<long long>::max(); }
};
}
template<class T> void verify(const vector<vector<T>>& a, T expected) {
  auto [value, match] = blueberry::assignment(a);
  if (value != expected || match.size() != a.size()) abort();
  vector<bool> used(a.empty() ? 0 : a[0].size());
  T sum{};
  for (int i = 0; i < static_cast<int>(a.size()); ++i) {
    int j = match[i];
    if (j < 0 || j >= static_cast<int>(used.size()) || used[j]) abort();
    used[j] = true; sum += a[i][j];
  }
  if (sum != expected) abort();
}
int main(int argc, char** argv) {
  unsigned seed = argc > 1 ? strtoul(argv[1], nullptr, 10) : 1729;
  mt19937 rng(seed);
  for (int n : {64, 65, 100}) {
    for (int trial = 0; trial < 40; ++trial) {
      int m = n + rng() % 17;
      vector<vector<long long>> a(n, vector<long long>(m));
      for (auto& row : a) for (auto& value : row) value = int(rng() % 2001) - 1000;
      verify(a, old_assignment::assignment(a).first);
    }
    vector<vector<long long>> chain(n, vector<long long>(n, 1000));
    for (int i = 0; i + 1 < n; ++i) { chain[i][i] = 0; chain[i][i + 1] = 1; }
    chain[n - 1][0] = 0;
    verify(chain, static_cast<long long>(n - 1));
    verify(chain, old_assignment::assignment(chain).first);
  }
  for (int extra : {0, 1}) {
    int n = 64, m = 8 * n + extra;
    vector<vector<long long>> a(n, vector<long long>(m));
    for (auto& row : a) for (auto& value : row) value = int(rng() % 61) - 30;
    verify(a, old_assignment::assignment(a).first);
    a.back().back() = numeric_limits<long long>::max() / (static_cast<long long>(n) + 1) / 16 + 1;
    verify(a, old_assignment::assignment(a).first);
    a.back().back() = 0;
    a.front().front() = numeric_limits<long long>::max() / (static_cast<long long>(n) + 1) / 16 + 1;
    verify(a, old_assignment::assignment(a).first);
  }
  for (int n : {63, 64, 65, 80}) for (int trial = 0; trial < 100; ++trial) {
    int m = n + 3;
    vector<vector<long long>> a(n, vector<long long>(m, 1000));
    for (int i = 4; i < n; ++i) a[i][i] = 0;
    vector<int> choices{0, 1, 2, 3, n, n + 1, n + 2};
    for (int i = 0; i < 4; ++i) for (int j : choices) a[i][j] = int(rng() % 31) - 15;
    long long best = numeric_limits<long long>::max();
    sort(choices.begin(), choices.end());
    do { long long sum = 0; for (int i = 0; i < 4; ++i) sum += a[i][choices[i]]; best = min(best, sum); }
    while (next_permutation(choices.begin(), choices.end()));
    verify(a, best);
    vector<vector<int>> b(n, vector<int>(m));
    for (int i = 0; i < n; ++i) for (int j = 0; j < m; ++j) b[i][j] = static_cast<int>(a[i][j]);
    verify(b, static_cast<int>(best));
  }
  for (int n : {64, 65}) {
    long long bound = numeric_limits<long long>::max() / (static_cast<long long>(n) + 1) / 16;
    for (long long value : {bound, bound + 1, -bound, -bound - 1})
      verify(vector<vector<long long>>(n, vector<long long>(n, value)), n * value);
    int ibound = numeric_limits<int>::max() / (n + 1) / 16;
    for (int value : {ibound, ibound + 1, -ibound, -ibound - 1})
      verify(vector<vector<int>>(n, vector<int>(n, value)), n * value);
    int sbound = numeric_limits<short>::max() / (n + 1) / 16;
    for (int value : {sbound, sbound + 1, -sbound, -sbound - 1})
      verify(vector<vector<short>>(n, vector<short>(n, static_cast<short>(value))), static_cast<short>(n * value));
    verify(vector<vector<unsigned>>(n, vector<unsigned>(n, 0)), 0u);
    verify(vector<vector<double>>(n, vector<double>(n, 0)), 0.0);
    auto result = blueberry::assignment(vector<vector<Scalar>>(n, vector<Scalar>(n, Scalar{})));
    if (result.first.x != 0 || result.second.size() != static_cast<size_t>(n)) abort();
    for (int trial = 0; trial < 40; ++trial) {
      vector<vector<long long>> a(n, vector<long long>(n + 3, bound));
      for (int i = 4; i < n; ++i) a[i][i] = 0;
      vector<int> choices{0, 1, 2, 3, n, n + 1, n + 2};
      for (int i = 0; i < 4; ++i) for (int j : choices) a[i][j] = (int(rng() % 31) - 15) * (bound / 256);
      long long best = numeric_limits<long long>::max();
      do { long long sum = 0; for (int i = 0; i < 4; ++i) sum += a[i][choices[i]]; best = min(best, sum); }
      while (next_permutation(choices.begin(), choices.end()));
      verify(a, best);
      a[n - 1][0] = bound + 1;
      verify(a, best);
    }
  }
  cout << "assignment seed=" << seed << " passed\n";
}

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <numeric>
#include <random>
#include <vector>
#include "blueberry/graph/assignment.hpp"
using namespace std;

struct Number {
  long long value = 0;
  Number() = default;
  Number(long long x) : value(x) {}
  Number& operator+=(Number x) { value += x.value; return *this; }
  Number& operator-=(Number x) { value -= x.value; return *this; }
  friend Number operator+(Number a, Number b) { return a.value + b.value; }
  friend Number operator-(Number a, Number b) { return a.value - b.value; }
  friend bool operator<(Number a, Number b) { return a.value < b.value; }
};
namespace std {
template <> class numeric_limits<Number> {
 public: static Number max() { return numeric_limits<long long>::max(); }
};
}

long long brute(const vector<vector<long long>>& a) {
  if (a.empty()) return 0;
  long long best = numeric_limits<long long>::max();
  auto dfs = [&](auto&& self, int i, int used, long long sum) -> void {
    if (i == static_cast<int>(a.size())) { best = min(best, sum); return; }
    for (int j = 0; j < static_cast<int>(a[i].size()); ++j)
      if (!(used & (1 << j))) self(self, i + 1, used | (1 << j), sum + a[i][j]);
  };
  dfs(dfs, 0, 0, 0);
  return best;
}

template<class T> void check(const vector<vector<T>>& a, long long expected) {
  auto [value, match] = blueberry::assignment(a);
  if (static_cast<long long>(value) != expected || match.size() != a.size()) abort();
  vector<int> seen(a.empty() ? 0 : a[0].size());
  long long sum = 0;
  for (int i = 0; i < static_cast<int>(a.size()); ++i) {
    int j = match[i];
    if (j < 0 || j >= static_cast<int>(seen.size()) || seen[j]++) abort();
    sum += static_cast<long long>(a[i][j]);
  }
  if (sum != expected) abort();
}

int main(int argc, char** argv) {
  mt19937 rng(argc > 1 ? static_cast<unsigned>(strtoul(argv[1], nullptr, 10)) : 1);
  check(vector<vector<long long>>{}, 0);
  for (int n = 1; n <= 7; ++n) for (int m = n; m <= 8; ++m) for (int t = 0; t < 80; ++t) {
    vector<vector<long long>> a(n, vector<long long>(m));
    for (int i = 0; i < n; ++i) for (int j = 0; j < m; ++j) {
      if (t % 4 == 0) a[i][j] = static_cast<int>(rng() % 3) - 1;
      else if (t % 4 == 1) a[i][j] = i * j;
      else if (t % 4 == 2) a[i][j] = -i * j;
      else a[i][j] = static_cast<int>(rng() % 2001) - 1000;
    }
    auto expected = brute(a);
    check(a, expected);
    vector<vector<int>> b(n, vector<int>(m));
    for (int i = 0; i < n; ++i) for (int j = 0; j < m; ++j) b[i][j] = static_cast<int>(a[i][j]);
    check(b, expected);
  }
  vector<vector<int>> a{{82, 6, 28, 57, 33}, {6, -46, 14, 51, 23}, {61, -44, -64, 1, 49}};
  for (auto& row : a) for (int& x : row) x *= 16000000;
  check(a, -1232000000LL);
  vector<vector<long long>> b{{82, 6, 28, 57, 33}, {6, -46, 14, 51, 23}, {61, -44, -64, 1, 49}};
  for (auto& row : b) for (long long& x : row) x *= 70000000000000000LL;
  check(b, -5390000000000000000LL);
  vector<vector<long long>> distinct{{0, 9, 9, 9}, {9, 0, 9, 9}, {9, 9, 0, 9}, {9, 9, 9, 0}};
  check(distinct, 0);
  {
    const int n = 4097;
    vector<vector<int>> large(n, vector<int>(n, 1));
    for (int i = 0; i < n; ++i) large[i][i] = 0;
    auto [value, match] = blueberry::assignment(large);
    if (value != 0 || match.size() != large.size()) abort();
    for (int i = 0; i < n; ++i) if (match[i] != i) abort();
  }
  const long long hi = numeric_limits<long long>::max(), lo = numeric_limits<long long>::min();
  vector<vector<long long>> tiny{{hi, 0, hi - 1}, {hi, lo + 10, hi - 2}};
  check(tiny, 8);
  vector<vector<Number>> custom{{Number(4), Number(1), Number(9)}, {Number(2), Number(8), Number(3)}};
  auto c = blueberry::assignment(custom);
  if (c.first.value != 3 || c.second[0] != 1 || c.second[1] != 0) abort();
  vector<vector<double>> floating{{2.5, -0.5}, {-1.25, 3.5}};
  auto d = blueberry::assignment(floating);
  if (d.first != -1.75 || d.second[0] != 1 || d.second[1] != 0) abort();
}

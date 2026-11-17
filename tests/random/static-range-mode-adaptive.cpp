#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <random>
#include <string>
#include <vector>
#include "blueberry/data-structure/static-range-mode.hpp"
using namespace std;
struct Key {
  int x;
  friend bool operator<(const Key& a, const Key& b) { return a.x < b.x; }
  friend bool operator==(const Key& a, const Key& b) { return a.x == b.x; }
};
template<class T> void test(const vector<T>& a, const vector<pair<int,int>>& q, unsigned seed) {
  auto answer = blueberry::static_range_mode(a, q);
  if (answer.size() != q.size()) std::abort();
  for (int id = 0; id < (int)q.size(); ++id) {
    auto [l, r] = q[id]; map<T, int> count; int best = 0;
    for (int j = l; j < r; ++j) best = max(best, ++count[a[j]]);
    auto [index, frequency] = answer[id];
    if (frequency != best || (!best && index != -1) || (best && (index < 0 || index >= (int)a.size() || count[a[index]] != best))) {
      cerr << "seed=" << seed << " n=" << a.size() << " l=" << l << " r=" << r << " expected=" << best << " actual=" << index << ',' << frequency << '\n';
      std::abort();
    }
  }
}
int main(int argc, char** argv) {
  unsigned seed = argc > 1 ? static_cast<unsigned>(std::stoul(argv[1])) : 399921;
  mt19937_64 rng(seed);
  for (int n = 0; n <= 70; ++n) for (int rep = 0; rep < 5; ++rep) {
    vector<int> a(n), unique(n), same(n, 42), binary(n);
    vector<long long> b(n); vector<string> c(n); vector<bool> d(n); vector<Key> custom(n);
    for (int i = 0; i < n; ++i) {
      auto x = rng(); a[i] = int(x % 13) - 6; b[i] = (x & 1) ? static_cast<long long>(x) : static_cast<long long>(x % 13) - 6;
      c[i] = to_string(a[i]); d[i] = x & 1; unique[i] = i; binary[i] = i % 2; custom[i] = Key{a[i]};
    }
    vector<pair<int,int>> q;
    for (int i = 0; i < 180; ++i) { int l = rng() % (n + 1), r = rng() % (n + 1); if (l > r) swap(l, r); q.push_back({l, r}); }
    test(a, q, seed); test(b, q, seed); test(c, q, seed); test(d, q, seed); test(custom, q, seed);
    if (!rep) { test(unique, q, seed); test(same, q, seed); test(binary, q, seed); test(a, {}, seed); }
  }
  test(vector<int>{4, 2, 4, 4}, {{0, 1}, {1, 2}, {0, 0}, {3, 4}}, seed);
  vector<int> long_a(2000); for (int& x : long_a) x = rng() % 11;
  test(long_a, {{0, 2000}, {1999, 2000}}, seed);
  vector<pair<int,int>> many(10000); for (auto& [l, r] : many) { l = rng() % 41; r = rng() % 41; if (l > r) swap(l, r); }
  test(vector<int>(long_a.begin(), long_a.begin() + 40), many, seed);
  test(vector<long long>{numeric_limits<long long>::min(), 0, numeric_limits<long long>::max(), numeric_limits<long long>::min()}, {{0, 4}, {0, 1}, {1, 3}, {4, 4}}, seed);
  test(vector<unsigned long long>{0, numeric_limits<unsigned long long>::max(), 1, 0, numeric_limits<unsigned long long>::max()}, {{0, 5}, {0, 1}, {1, 4}, {5, 5}}, seed);
  test(vector<Key>{{1}, {2}, {1}, {3}, {4}, {1}, {5}, {2}}, {{0,0}, {0,1}, {0,8}, {1,8}, {1,1}, {1,2}, {8,8}}, seed);
}

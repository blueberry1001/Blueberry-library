#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <vector>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <psapi.h>
#endif
#include "baseline-final.hpp.txt"
#include "blueberry/data-structure/static-range-mode.hpp"
#include "adaptive-mo.hpp.txt"
namespace mode_candidate { using blueberry::static_range_mode; }
using namespace std;
using Query = pair<int, int>;
struct Key {
  int x;
  bool operator<(const Key& b) const { return x < b.x; }
  bool operator==(const Key& b) const { return x == b.x; }
};
template<class T> void check(const vector<T>& a, const vector<Query>& qs) {
  auto old = mode_baseline::static_range_mode(a, qs);
  auto cur = mode_candidate::static_range_mode(a, qs);
  auto adaptive = mode_adaptive::static_range_mode(a, qs);
  if (old.size() != qs.size() || cur.size() != qs.size() || adaptive.size() != qs.size()) abort();
  for (size_t j = 0; j < qs.size(); ++j) {
    auto [l, r] = qs[j]; map<T,int> f; int best = 0;
    for (int i = l; i < r; ++i) best = max(best, ++f[a[i]]);
    for (auto ans : {old[j], cur[j], adaptive[j]}) {
      if (ans.second != best || (best ? ans.first < 0 || ans.first >= static_cast<int>(a.size()) || f[a[ans.first]] != best : ans.first != -1)) {
        cerr << "mismatch n=" << a.size() << " query=" << l << ',' << r << " answer=" << ans.first << ',' << ans.second << " expected=" << best << '\n'; abort();
      }
    }
  }
}
int main(int argc, char** argv) {
  if (argc == 1) {
    mt19937 gen(1729);
    for (int n = 0; n <= 60; ++n) for (int t = 0; t < 50; ++t) {
      vector<int> a(n); for (int& x : a) x = static_cast<int>(gen()%9)-4;
      vector<Query> qs; for (int l = 0; l <= n; ++l) for (int r = l; r <= n; ++r) qs.emplace_back(l,r);
      shuffle(qs.begin(), qs.end(), gen); check(a,qs);
      qs.clear(); check(a,qs);
    }
    vector<string> s{"aa", "ab", "aa", "cc", "ab", "aa"}; check(s, {{0,6},{1,4},{3,3},{2,6}});
    check(s, vector<Query>(20, {1, 5}));
    check(s, {{2, 2}});
    vector<bool> b{true,false,true,false,true}; check(b,{{0,5},{2,4},{0,0},{1,5}});
    vector<Key> k{{2},{1},{2},{3},{1}}; check(k,{{0,5},{1,4},{3,3},{0,2}});
    vector<int> a(50); for (int& x : a) x = static_cast<int>(gen()%11);
    for (int groups : {1,2,8,9}) {
      vector<Query> p, v;
      for (int i = 0; i < 200; ++i) { int g = i % groups; p.emplace_back(g, 49 - static_cast<int>(gen()%20)); v.emplace_back(static_cast<int>(gen()%20), 50 - g); }
      check(a,p); check(a,v);
    }
    cout << "random and boundary pass\n"; return 0;
  }
  int which = atoi(argv[1]), n = atoi(argv[2]), q = atoi(argv[3]), mode = atoi(argv[4]);
  mt19937 gen(22017); vector<int> a(n);
  for (int& x : a) x = mode == 2 ? 42 : mode == 3 ? static_cast<int>(gen()%8) : static_cast<int>(gen()%10000);
  if (mode == 7 || mode == 8) iota(a.begin(),a.end(),0);
  if (mode == 9) for (int i = 0; i < n; ++i) a[i] = n-i;
  vector<Query> qs; qs.reserve(q);
  for (int i = 0; i < q; ++i) {
    int l = static_cast<int>(gen()%(n+1)), r = static_cast<int>(gen()%(n+1)); if (l > r) swap(l,r);
    if (mode == 1) r = min(n,l+static_cast<int>(gen()%9));
    if (mode == 4) { l = 0; r = n; }
    if (mode == 5) { l = i % 256; r = n - i % 251; }
    if (mode == 6) { l = static_cast<int>(gen()%512); r = n - static_cast<int>(gen()%512); }
    if (mode == 8) { l = i % (n+1); r = n; }
    if (mode == 9) { l = 0; r = i % (n+1); }
    if (mode == 10) { l = i % (n-2); r = n - i % 2; }
    if (mode == 11) { l = i % 2; r = 1 + i % (n-1); }
    if (mode == 12) { l = static_cast<int>(gen()%512); r = n - static_cast<int>(gen()%2); }
    if (mode == 13) { l = static_cast<int>(gen()%(n-8)); r = n - i % 8; }
    if (mode == 14) { l = static_cast<int>(gen()%(n-9)); r = n - i % 9; }
    qs.emplace_back(l,r);
  }
  auto start = chrono::steady_clock::now();
  auto ans = which == 0 ? mode_baseline::static_range_mode(a,qs) : which == 1 ? mode_candidate::static_range_mode(a,qs) : mode_adaptive::static_range_mode(a,qs);
  double ms = chrono::duration<double,milli>(chrono::steady_clock::now()-start).count();
  uint64_t hash = 1469598103934665603ULL;
  for (auto [idx, f] : ans) { (void)idx; hash = (hash ^ static_cast<unsigned>(f)) * 1099511628211ULL; }
  for (int j = 0; j < min(q,mode >= 7 ? 4 : 32); ++j) {
    auto [l,r] = qs[j]; map<int,int> counts; int f = 0;
    for (int i = l; i < r; ++i) f = max(f, ++counts[a[i]]);
    auto [idx,got] = ans[j];
    if (got != f || (got ? idx < 0 || idx >= n || counts[a[idx]] != got : idx != -1)) { cerr << "bad query=" << j << '\n'; return 1; }
  }
  cout << ms << ' ' << hash << ' ';
#ifdef _WIN32
  PROCESS_MEMORY_COUNTERS mem{}; mem.cb = sizeof(mem);
  if (GetProcessMemoryInfo(GetCurrentProcess(), &mem, sizeof(mem))) cout << mem.PeakWorkingSetSize;
  else cout << "NA";
#else
  cout << "NA";
#endif
  cout << '\n';
}

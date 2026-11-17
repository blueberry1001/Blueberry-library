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
struct MutableKey {
  int value;
  bool operator<(const MutableKey& rhs) const { return value < rhs.value; }
  friend bool operator==(MutableKey& x, MutableKey& y) { return x.value == y.value; }
};
struct CopyOnlyMutable {
  int value;
  explicit CopyOnlyMutable(int x):value(x){}
  CopyOnlyMutable(const CopyOnlyMutable&)=default;
  CopyOnlyMutable& operator=(const CopyOnlyMutable&)=default;
  CopyOnlyMutable(CopyOnlyMutable&&)=delete;
  CopyOnlyMutable& operator=(CopyOnlyMutable&&)=delete;
  friend bool operator<(const CopyOnlyMutable& x,const CopyOnlyMutable& y){return x.value<y.value;}
  friend bool operator==(CopyOnlyMutable& x,CopyOnlyMutable& y){return x.value==y.value;}
};
struct NoSwapMutable {
  int value;
  explicit NoSwapMutable(int x):value(x){}
  NoSwapMutable(const NoSwapMutable&)=default;
  NoSwapMutable& operator=(const NoSwapMutable&)=default;
  NoSwapMutable(NoSwapMutable&&)=default;
  NoSwapMutable& operator=(NoSwapMutable&&)=default;
  friend void swap(NoSwapMutable&,NoSwapMutable&)=delete;
  friend bool operator<(const NoSwapMutable& x,const NoSwapMutable& y){return x.value<y.value;}
  friend bool operator==(NoSwapMutable& x,NoSwapMutable& y){return x.value==y.value;}
};
static_assert(is_copy_constructible_v<CopyOnlyMutable> && !is_move_constructible_v<CopyOnlyMutable>);
static_assert(is_move_constructible_v<NoSwapMutable> && !is_swappable_v<NoSwapMutable>);
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
  {
    const vector<CopyOnlyMutable> a{CopyOnlyMutable(4),CopyOnlyMutable(2),CopyOnlyMutable(4),CopyOnlyMutable(3),CopyOnlyMutable(2),CopyOnlyMutable(2),CopyOnlyMutable(1),CopyOnlyMutable(4)};
    const vector<NoSwapMutable> b{NoSwapMutable(4),NoSwapMutable(2),NoSwapMutable(4),NoSwapMutable(3),NoSwapMutable(2),NoSwapMutable(2),NoSwapMutable(1),NoSwapMutable(4)};
    vector<pair<int,int>> queries;for(int l=0;l<=8;++l)for(int r=l;r<=8;++r)queries.emplace_back(l,r);
    test(a,queries,seed);test(b,queries,seed);test(a,vector<pair<int,int>>(100,{1,7}),seed);test(b,vector<pair<int,int>>(100,{1,7}),seed);
    test(a,{{0,8}},seed);test(b,{{1,1}},seed);test(vector<CopyOnlyMutable>{},{{0,0}},seed);test(vector<NoSwapMutable>{},{},seed);
  }
  {
    vector<MutableKey> mutable_values(80);
    for (auto& x : mutable_values) x.value = int(rng()%13)-6;
    for (int groups : {1,2,8,9,16}) for (int side=0;side<2;++side) {
      vector<pair<int,int>> queries;
      for(int i=0;i<300;++i){int t=i%groups;queries.emplace_back(side?int(rng()%60):t,side?80-t:20+int(rng()%61));}
      auto ans=blueberry::static_range_mode(mutable_values,queries);
      for(int i=0;i<(int)queries.size();++i){auto [l,r]=queries[i];map<int,int> c;int best=0;for(int j=l;j<r;++j)best=max(best,++c[mutable_values[j].value]);auto [index,f]=ans[i];if(f!=best||(f&&(index<0||index>=80||c[mutable_values[index].value]!=f))||(!f&&index!=-1))abort();}
    }
    for(auto interval:{pair{0,80},pair{20,20},pair{10,14}}){vector<pair<int,int>>queries(100,interval);auto ans=blueberry::static_range_mode(mutable_values,queries);map<int,int> c;int best=0;for(int i=interval.first;i<interval.second;++i)best=max(best,++c[mutable_values[i].value]);for(auto[index,f]:ans)if(f!=best||(f&&(index<0||index>=80||c[mutable_values[index].value]!=f))||(!f&&index!=-1))abort();}
  }
}

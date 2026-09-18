#include <chrono>
#include <iostream>
#include <random>
#include <vector>
#include "blueberry/data-structure/weighted-wavelet-matrix.hpp"
#include "blueberry/data-structure/offline-fenwick-tree-2d.hpp"
using namespace std;
int main() {
  constexpr int n = 40000, q = 40000;
  mt19937 rng(20260918);
  vector<int> a(n); vector<long long> w(n, 1);
  vector<pair<int,int>> points;
  for (int i = 0; i < n; ++i) { a[i] = rng() % n; points.emplace_back(i,a[i]); }
  struct Query {int l,r,lo,hi;}; vector<Query> queries;
  for (int i = 0; i < q; ++i) {
    int l=rng()%n,r=rng()%n,lo=rng()%n,hi=rng()%n;
    if(l>r)swap(l,r); if(lo>hi)swap(lo,hi); queries.push_back({l,r,lo,hi});
  }
  using Clock=chrono::steady_clock;
  for (int run=0;run<3;++run) for(int kind=0;kind<2;++kind) {
    auto t=Clock::now(); long long checksum=0; double build,ops;
    if(kind==0) {
      blueberry::WeightedWaveletMatrix<int> wm(a,w);
      auto b=Clock::now();
      for(int i=0;i<q;++i) {auto x=queries[i]; if(i%2)wm.add(x.l,1);else checksum+=wm.sum(x.l,x.r,x.lo,x.hi);}
      build=chrono::duration<double,milli>(b-t).count();ops=chrono::duration<double,milli>(Clock::now()-b).count();
    } else {
      blueberry::OfflineFenwickTree2D<long long,int> ft(points);
      for(int i=0;i<n;++i)ft.add(i,a[i],w[i]);
      auto b=Clock::now();
      for(int i=0;i<q;++i) {auto x=queries[i];if(i%2)ft.add(x.l,a[x.l],1);else checksum+=ft.sum(x.l,x.lo,x.r,x.hi);}
      build=chrono::duration<double,milli>(b-t).count();ops=chrono::duration<double,milli>(Clock::now()-b).count();
    }
    cout<<"run="<<run<<" kind="<<(kind?"offline-fenwick-2d":"weighted-wm")<<" build_ms="<<build<<" ops_ms="<<ops<<" checksum="<<checksum<<'\n';
  }
}

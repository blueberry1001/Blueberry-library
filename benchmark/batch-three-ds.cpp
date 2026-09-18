// Independently written alternatives; fixed identical inputs and checked outputs.
#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>
#include <atcoder/fenwicktree>
#include "blueberry/data-structure/persistent-queue.hpp"
#include "blueberry/data-structure/static-range-distinct.hpp"
#include "blueberry/data-structure/rectangle-union.hpp"
using namespace std;
template<class F> long long measure(const char* label, F f) {
  auto start=chrono::steady_clock::now(); long long result=f();
  cout<<label<<','<<chrono::duration<double,milli>(chrono::steady_clock::now()-start).count()<<','<<result<<'\n';
  return result;
}
int main() {
  const int n=100000;
  mt19937 rng(3103);
  vector<int> a(n); for(auto& x:a) x=rng()%10000;
  vector<pair<int,int>> qs(n); for(auto& [l,r]:qs) { l=rng()%(n+1); r=rng()%(n+1); if(l>r)swap(l,r); }
  vector<array<int,4>> rect(2000);
  for(auto& r:rect) { for(int& x:r)x=rng()%100000; if(r[0]>r[2])swap(r[0],r[2]); if(r[1]>r[3])swap(r[1],r[3]); }
  for(int run=0;run<3;++run) {
    cout<<"run,"<<run<<'\n';
    auto q1=measure("queue_skip",[&]{ blueberry::PersistentQueue<int> q; int v=0; long long s=0;
      for(int x:a)v=q.push(v,x);
      for(int i=0;i<n;++i){s+=q.front(v);v=q.pop(v);}return s; });
    auto q2=measure("queue_doubling",[&]{
      vector<array<int,17>> up(n); long long s=0;
      for(int i=0;i<n;++i){up[i].fill(-1);up[i][0]=i-1;for(int j=1;j<17;++j)if(up[i][j-1]>=0)up[i][j]=up[up[i][j-1]][j-1];}
      for(int i=0;i<n;++i){int v=n-1,k=n-i-1;for(int j=0;j<17;++j)if(k>>j&1)v=up[v][j];s+=a[v];}return s; });
    assert(q1==q2);
    auto d1=measure("distinct_wavelet",[&]{blueberry::StaticRangeDistinct<int>d(a);long long s=0;for(auto [l,r]:qs)s+=d.count(l,r);return s;});
    auto d2=measure("distinct_offline_fenwick",[&]{vector<int> order(n),last(10000,-1);iota(order.begin(),order.end(),0);sort(order.begin(),order.end(),[&](int x,int y){return qs[x].second<qs[y].second;});atcoder::fenwick_tree<int> bit(n);int p=0;long long s=0;for(int i:order){auto [l,r]=qs[i];while(p<r){if(last[a[p]]>=0)bit.add(last[a[p]],-1);bit.add(p,1);last[a[p]]=p;++p;}s+=bit.sum(l,r);}return s;});
    assert(d1==d2);
    auto r1=measure("rectangle_tree",[&]{return blueberry::rectangle_union_area(rect);});
    auto r2=measure("rectangle_strip_scan",[&]{vector<int> xs;for(auto r:rect){xs.push_back(r[0]);xs.push_back(r[2]);}sort(xs.begin(),xs.end());xs.erase(unique(xs.begin(),xs.end()),xs.end());long long s=0;for(int i=1;i<(int)xs.size();++i){vector<pair<int,int>> ys;for(auto r:rect)if(r[0]<=xs[i-1]&&xs[i]<=r[2])ys.emplace_back(r[1],r[3]);sort(ys.begin(),ys.end());int end=0;long long len=0;for(auto [l,r]:ys){len+=max(0,r-max(l,end));end=max(end,r);}s+=len*(xs[i]-xs[i-1]);}return s;});
    assert(r1==r2);
  }
}

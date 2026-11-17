#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <numeric>
#include <queue>
#include <random>
#include <tuple>
#include <vector>
#ifdef RELEASE_HEADER
#define NDEBUG
#endif
#include "blueberry/graph/minimum-spanning-forest.hpp"
#ifdef RELEASE_HEADER
#undef NDEBUG
#include <cassert>
#endif
using namespace std;
template<class T> void check(int n,const vector<tuple<int,int,T>>& e) {
  vector<vector<pair<int,T>>> g(n);
  for(auto [u,v,w]:e) {g[u].emplace_back(v,w);g[v].emplace_back(u,w);}
  vector<int> seen(n);T expected{};
  using E=pair<T,int>;
  for(int s=0;s<n;++s) if(!seen[s]) {
    priority_queue<E,vector<E>,greater<E>> pq;
    pq.push({T{},s});
    while(!pq.empty()) {
      auto [w,u]=pq.top();pq.pop();
      if(seen[u]) continue;
      seen[u]=1;expected+=w;
      for(auto [v,c]:g[u]) if(!seen[v]) pq.push({c,v});
    }
  }
  auto [actual,idx]=blueberry::minimum_spanning_forest(n,e);
  assert(actual==expected);
  vector<int> p(n), used(e.size());iota(p.begin(),p.end(),0);
  auto root=[&](int x){while(x!=p[x]) x=p[x];return x;};
  T sum{};bool first=true;T last{};
  for(int i:idx) {
    assert(i>=0 && static_cast<size_t>(i)<e.size() && !used[i]); used[i]=1;
    auto [u,v,w]=e[i];assert(root(u)!=root(v));p[root(u)]=root(v);
    assert(first || !(w<last));first=false;last=w;sum+=w;
  }
  assert(sum==actual);
  for(auto [u,v,w]:e) assert(root(u)==root(v));
}
struct Weight {
 long long value=0;
 Weight& operator+=(Weight rhs){value+=rhs.value;return *this;}
 friend bool operator<(Weight a,Weight b){return a.value<b.value;}
 friend bool operator>(Weight a,Weight b){return b<a;}
 friend bool operator==(Weight a,Weight b){return a.value==b.value;}
};
int main(){
 mt19937_64 rng(0x9321010);
 check<long long>(0,{});
 check<long long>(1,{{0,0,-8}});
 for(int t=0;t<6000;++t) {
   int n=1+rng()%15,m=rng()%80; vector<tuple<int,int,long long>> e;
   for(int i=0;i<m;++i) e.emplace_back(rng()%n,rng()%n,static_cast<long long>(rng()%101)-50);
   check(n,e);
 }
 for(int m:{8191,8192,8193,20000}) for(int t=0;t<12;++t) {
   int n=2+rng()%100;vector<tuple<int,int,long long>> e;
   for(int i=0;i<m;++i) {
     long long w=static_cast<int32_t>(rng());
     if(t==1) w=static_cast<long long>(rng()%3)-1;
     if(t==2 && i==m-1) w=static_cast<long long>(numeric_limits<int32_t>::max())+1;
     if(t==3 && i==m-1) w=static_cast<long long>(numeric_limits<int32_t>::min())-1;
     if(t==4) w=i%4096-2048;
     if(t==5) w=i%4097-2048;
     if(t==6) w=static_cast<long long>(numeric_limits<int32_t>::min())+i%4096;
     if(t==7) w=static_cast<long long>(numeric_limits<int32_t>::max())-i%4096;
     e.emplace_back(rng()%n,rng()%n,w);
   }
   check(n,e);
 }
 for(int mode=0;mode<3;++mode) {
   vector<tuple<int,int,uint64_t>> e;
   for(int i=0;i<8192;++i) e.emplace_back(0,1,mode==0?uint64_t(i):mode==1?uint64_t(0xffffffffu)-i:(uint64_t(1)<<40)+i);
   check(2,e);
 }
 for(int n:{4095,4096,4097}) {
   vector<tuple<int,int,long long>> e;
   for(int i=0;i<8192;++i) e.emplace_back(rng()%n,rng()%n,static_cast<int32_t>(rng()));
   check(n,e);
 }
 vector<tuple<int,int,Weight>> z{{0,1,{-5}},{0,1,{3}},{1,2,{2}},{0,2,{1}}};check(3,z);
 vector<tuple<int,int,double>> f{{0,1,-5},{0,1,3},{1,2,2},{0,2,1}};check(3,f);
 cerr<<"mst correctness seed=0x9321010 passed\n";
}

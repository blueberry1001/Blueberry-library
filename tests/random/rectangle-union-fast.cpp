#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <vector>
#include "blueberry/data-structure/rectangle-union.hpp"
using namespace std;

long long brute(const vector<array<int,4>>& r) {
  long long ans=0;
  for(int x=-5;x<6;++x) for(int y=-5;y<6;++y) {
    bool covered=false;
    for(auto a:r) covered|=a[0]<=x && x<a[2] && a[1]<=y && y<a[3];
    ans+=covered;
  }
  return ans;
}

int main() {
  vector<array<int,4>> all;
  for(int x=-2;x<=2;++x) for(int xx=x;xx<=2;++xx)
    for(int y=-2;y<=2;++y) for(int yy=y;yy<=2;++yy) all.push_back({x,y,xx,yy});
  for(auto a:all) for(auto b:all) {
    vector<array<int,4>> r{a,b};
    if(blueberry::rectangle_union_area(r)!=brute(r)) {cerr<<"exhaustive failure\n";return 1;}
  }
  mt19937 rng(20261117);
  for(int t=0;t<10000;++t) {
    vector<array<int,4>> r;
    for(unsigned i=0,n=rng()%24;i<n;++i) {
      int x=int(rng()%10)-5,y=int(rng()%10)-5;
      r.push_back({x,y,x+int(rng()%(7-x)),y+int(rng()%(7-y))});
    }
    auto expected=brute(r),actual=blueberry::rectangle_union_area(r);
    if(expected!=actual) {
      cerr<<"seed=20261117 case="<<t<<" expected="<<expected<<" actual="<<actual<<'\n';
      for(auto a:r) cerr<<a[0]<<' '<<a[1]<<' '<<a[2]<<' '<<a[3]<<'\n';
      return 1;
    }
  }
  using ll=long long;
  ll lo=numeric_limits<ll>::min(),hi=numeric_limits<ll>::max();
  vector<array<ll,4>> degenerate{{lo,0,hi,0},{0,lo,0,hi}};
  if(blueberry::rectangle_union_area(degenerate)!=0) return 2;
  vector<array<ll,4>> huge{{0,0,3000000000LL,3000000000LL},{0,0,3000000000LL,3000000000LL}};
  if(blueberry::rectangle_union_area(huge)!=9000000000000000000LL) return 3;
#ifdef __SIZEOF_INT128__
  vector<array<ll,4>> wide{{lo,0,hi,1},{lo,0,hi,1}};
  if(blueberry::rectangle_union_area<__int128_t>(wide)!=__int128_t(hi)-__int128_t(lo)) return 4;
  wide.push_back({0,1,1,2});
  if(blueberry::rectangle_union_area<__int128_t>(wide)!=__int128_t(hi)-__int128_t(lo)+1) return 5;
  using ull=unsigned long long;
  vector<array<ull,4>> unsign{{0,0,numeric_limits<ull>::max(),1},{0,0,numeric_limits<ull>::max(),1}};
  if(blueberry::rectangle_union_area<__uint128_t>(unsign)!=__uint128_t(numeric_limits<ull>::max())) return 6;
  unsign.push_back({0,1,1,2});
  if(blueberry::rectangle_union_area<__uint128_t>(unsign)!=__uint128_t(numeric_limits<ull>::max())+1) return 7;
#endif
  cout<<"ok\n";
}

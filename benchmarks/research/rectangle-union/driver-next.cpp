#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <vector>
#include <windows.h>
#include <psapi.h>
#include "baseline.hpp"
#include "reserved.hpp"
#include "branch.hpp"
#include "both.hpp"
#include "counted.hpp"
#include "ranked.hpp"
#include "rankfree.hpp"
#include "rankdirect.hpp"
#include "rankfast.hpp"
#include "rankfreefast.hpp"
using namespace std;
int main(int argc,char** argv) {
  int variant=atoi(argv[1]), kind=atoi(argv[2]), n=atoi(argv[3]), repeat=atoi(argv[4]);
  mt19937 rng(20261117); vector<array<long long,4>> a; a.reserve(n);
  for (int i=0;i<n;++i) {
    long long x=int(rng()%1000000)-500000, y=int(rng()%1000000)-500000;
    long long w=1+rng()%1000,h=1+rng()%1000;
    if(kind==1) {x=0;y=0;w=100;h=100;}
    if(kind==2) {x=i*2;y=i*2;w=1;h=1;}
    if(kind==3) {y=int(rng()%8)*10;h=10;}
    if(kind==4) {w=0;h=0;}
    if(kind==5) {if(i%100) w=0;}
    if(kind==6) {x=0;y=0;w=1000000;h=1000000;}
    a.push_back({x,y,x+w,y+h});
  }
  auto call=[&](int v) {if(v==0)return baseline::rectangle_union_area(a);if(v==1)return reserved::rectangle_union_area(a);if(v==2)return branch::rectangle_union_area(a);if(v==3)return both::rectangle_union_area(a);if(v==4)return counted::rectangle_union_area(a);if(v==5)return ranked::rectangle_union_area(a);if(v==6)return rankfree::rectangle_union_area(a);if(v==7)return rankdirect::rectangle_union_area(a);if(v==8)return rankfast::rectangle_union_area(a);return rankfreefast::rectangle_union_area(a);};
  if(variant==10) {
    for(int k=0;k<100;++k) { vector<array<int,4>> r; for(int i=0;i<k;++i){int x=int(rng()%17)-8,y=int(rng()%17)-8,w=rng()%6,h=rng()%6;r.push_back({x,y,x+w,y+h});}
      bool grid[32][32]{}; long long ans=0;for(auto q:r)for(int x=q[0];x<q[2];++x)for(int y=q[1];y<q[3];++y)if(!grid[x+8][y+8]){grid[x+8][y+8]=true;++ans;}
      if(baseline::rectangle_union_area(r)!=ans||reserved::rectangle_union_area(r)!=ans||branch::rectangle_union_area(r)!=ans||both::rectangle_union_area(r)!=ans||counted::rectangle_union_area(r)!=ans||ranked::rectangle_union_area(r)!=ans||rankfree::rectangle_union_area(r)!=ans||rankdirect::rectangle_union_area(r)!=ans||rankfast::rectangle_union_area(r)!=ans||rankfreefast::rectangle_union_area(r)!=ans) return 3;
    }
    cout<<"ok\n";return 0;
  }
  std::uint64_t hash=0; long long area=0; auto start=chrono::steady_clock::now();
  for(int t=0;t<repeat;++t) {area=call(variant); hash=hash*37+static_cast<std::uint64_t>(area);}
  auto ms=chrono::duration<double,milli>(chrono::steady_clock::now()-start).count();
  PROCESS_MEMORY_COUNTERS pm{}; if(!GetProcessMemoryInfo(GetCurrentProcess(),&pm,sizeof(pm))) return 4;
  cout<<ms<<' '<<hash<<' '<<area<<' '<<pm.PeakWorkingSetSize<<'\n';
}

#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>
#include <queue>
#include <random>
#include <vector>
#include "blueberry/data-structure/linear-rmq.hpp"
#include "blueberry/data-structure/sparse-table.hpp"
#include "blueberry/data-structure/kd-tree.hpp"
#include "blueberry/data-structure/offline-fenwick-tree-2d.hpp"
#include "blueberry/graph/linear-lca.hpp"
#include "blueberry/graph/lowest-common-ancestor.hpp"
#include "blueberry/graph/contour-query.hpp"
long long op(long long a,long long b){return a+b;}
long long e(){return 0;}
using Clock=std::chrono::steady_clock;
template<class F> long long measure(const char*name,int run,F fn){auto s=Clock::now();long long hash=fn();auto t=Clock::now();std::cout<<name<<','<<run<<','<<std::chrono::duration<double,std::milli>(t-s).count()<<','<<hash<<'\n';return hash;}
int main(){
 std::mt19937 rng(20260919);const int n=100000,q=200000;
 std::vector<int>a(n);for(auto&x:a)x=rng()%1000000;
 std::vector<std::pair<int,int>> ranges(q);for(auto&[l,r]:ranges){l=rng()%n;r=rng()%n;if(l>r)std::swap(l,r);++r;}
 std::vector<std::vector<int>>g(50000);for(int i=1;i<int(g.size());++i){int p=rng()%i;g[p].push_back(i);g[i].push_back(p);}
 std::vector<std::pair<int,int>>pairs(100000);for(auto&[u,v]:pairs){u=rng()%g.size();v=rng()%g.size();}
 std::vector<std::pair<int,int>>xy(10000);for(auto&[x,y]:xy){x=rng()%100000;y=rng()%100000;}
 struct Rect{int x,y,r,u,p;};std::vector<Rect>rects(20000);for(auto&z:rects){z.x=rng()%100000;z.r=rng()%100000;z.y=rng()%100000;z.u=rng()%100000;z.p=rng()%xy.size();if(z.x>z.r)std::swap(z.x,z.r);if(z.y>z.u)std::swap(z.y,z.u);}
 std::cout<<"seed=20260919 rmq_N=100000_Q=200000 lca_N=50000_Q=100000 spatial_N=10000_Q=20000_times_add+query\nname,run,total_build_and_query_ms,checksum\n";
 for(int run=0;run<5;++run){
  auto x=measure("linear-rmq",run,[&]{blueberry::LinearRMQ<int>t(a);long long sum=0;for(auto[l,r]:ranges)sum+=t.prod(l,r);return sum;});
  auto y=measure("sparse-table",run,[&]{blueberry::SparseTable t(a,[](int l,int r){return std::min(l,r);});long long sum=0;for(auto[l,r]:ranges)sum+=t.prod(l,r);return sum;});if(x!=y)return 1;
  x=measure("linear-lca",run,[&]{blueberry::LinearLCA t(g);long long sum=0;for(auto[u,v]:pairs)sum+=t.lca(u,v);return sum;});
  y=measure("sparse-lca",run,[&]{blueberry::LowestCommonAncestorRMQ t(g);long long sum=0;for(auto[u,v]:pairs)sum+=t.lca(u,v);return sum;});if(x!=y)return 1;
  x=measure("kd-tree",run,[&]{blueberry::KDTree<int,long long,op,e>t(xy,std::vector<long long>(xy.size()));long long sum=0;for(auto z:rects){t.set(z.p,t.get(z.p)+1);sum+=t.prod(z.x,z.y,z.r,z.u);}return sum;});
  y=measure("offline-fenwick-2d",run,[&]{blueberry::OfflineFenwickTree2D<long long,int>t(xy);long long sum=0;for(auto z:rects){t.add(xy[z.p].first,xy[z.p].second,1);sum+=t.sum(z.x,z.y,z.r,z.u);}return sum;});if(x!=y)return 1;
 }
}

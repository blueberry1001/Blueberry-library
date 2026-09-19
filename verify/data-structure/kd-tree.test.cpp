#define PROBLEM "https://judge.yosupo.jp/problem/point_add_rectangle_sum"
#include <iostream>
#include <utility>
#include <vector>
#include "blueberry/data-structure/kd-tree.hpp"
long long op(long long a,long long b){return a+b;}
long long e(){return 0;}
int main(){
  std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
  int n,q;std::cin>>n>>q;std::vector<std::pair<int,int>>p(n);std::vector<long long>w(n);
  for(int i=0;i<n;++i)std::cin>>p[i].first>>p[i].second>>w[i];
  struct Query{int type,x,y,r,u,id;};std::vector<Query>queries(q);
  for(auto& z:queries){std::cin>>z.type>>z.x>>z.y>>z.r;if(z.type==1)std::cin>>z.u;
    else{z.id=int(p.size());p.emplace_back(z.x,z.y);w.push_back(0);}}
  // Pre-register future insertion positions; duplicate points keep independent IDs.
  blueberry::KDTree<int,long long,op,e>tree(p,w);
  for(auto z:queries)if(z.type==0)tree.set(z.id,z.r);else std::cout<<tree.prod(z.x,z.y,z.r,z.u)<<'\n';
}

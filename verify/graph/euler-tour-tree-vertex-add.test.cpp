#define PROBLEM "https://judge.yosupo.jp/problem/dynamic_tree_vertex_add_subtree_sum"
#include <iostream>
#include <vector>
#include "blueberry/graph/euler-tour-tree.hpp"
long long op(long long a,long long b){return a+b;}
long long e(){return 0;}
long long mapping(long long f,long long x,int n){return x+f*n;}
long long composition(long long f,long long g){return f+g;}
long long id(){return 0;}
int main(){
  std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
  int n,q;std::cin>>n>>q;std::vector<long long>a(n);for(auto&x:a)std::cin>>x;
  blueberry::EulerTourTree<long long,op,e,long long,mapping,composition,id>t(a);
  for(int i=1;i<n;++i){int u,v;std::cin>>u>>v;t.link(u,v);}
  while(q--){int type,u,v;std::cin>>type>>u>>v;
    if(type==0){int w,x;std::cin>>w>>x;t.cut(u,v);t.link(w,x);}
    if(type==1)t.set(u,t.get(u)+v);
    if(type==2)std::cout<<t.prod(u,v)<<'\n';
  }
}

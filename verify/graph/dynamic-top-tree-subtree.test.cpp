#define PROBLEM "https://judge.yosupo.jp/problem/dynamic_tree_vertex_add_subtree_sum"
#include <iostream>
#include <vector>
#include "blueberry/graph/dynamic-top-tree.hpp"
long long vertex(long long value,long long light){return value+light;}
long long edge(long long path){return path;}
long long merge(long long x,long long y){return x+y;}
long long identity(){return 0;}
int main(){
  std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
  int n,q;std::cin>>n>>q;std::vector<long long>a(n);for(auto&x:a)std::cin>>x;
  blueberry::DynamicTopTree<long long,long long,long long,vertex,edge,merge,merge,identity>t(a);
  for(int i=1;i<n;++i){int u,v;std::cin>>u>>v;t.link(u,v);}
  while(q--){int type,u,v;std::cin>>type>>u>>v;
    if(type==0){int w,x;std::cin>>w>>x;t.cut(u,v);t.link(w,x);}
    if(type==1)t.set(u,t.get(u)+v);
    if(type==2)std::cout<<t.subtree_prod(u,v)<<'\n';
  }
}

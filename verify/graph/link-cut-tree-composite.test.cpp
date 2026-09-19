#define PROBLEM "https://judge.yosupo.jp/problem/dynamic_tree_vertex_set_path_composite"
#include <iostream>
#include <vector>
#include <atcoder/modint>
#include "blueberry/graph/link-cut-tree.hpp"
using Mint=atcoder::modint998244353;
struct Affine{Mint a,b;};
Affine op(Affine f,Affine g){return {g.a*f.a,g.a*f.b+g.b};}
Affine e(){return {1,0};}
int main(){
  std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
  int n,q;std::cin>>n>>q;std::vector<Affine>a(n);
  for(auto&f:a){int x,y;std::cin>>x>>y;f={x,y};}
  blueberry::LinkCutTree<Affine,op,e> t(a);
  for(int i=1;i<n;++i){int u,v;std::cin>>u>>v;t.link(u,v);}
  while(q--){int type,u,v;std::cin>>type>>u>>v;
    if(type==0){int w,x;std::cin>>w>>x;t.cut(u,v);t.link(w,x);}
    if(type==1){int d;std::cin>>d;t.set(u,{v,d});}
    if(type==2){int x;std::cin>>x;auto f=t.prod(u,v);std::cout<<(f.a*x+f.b).val()<<'\n';}
  }
}

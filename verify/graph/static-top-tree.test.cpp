#define PROBLEM "https://judge.yosupo.jp/problem/point_set_tree_path_composite_sum_fixed_root"
#include <iostream>
#include <vector>
#include <atcoder/modint>
#include "blueberry/graph/static-top-tree.hpp"
using Mint=atcoder::modint998244353;
struct Vertex{Mint value,a=1,b=0;int count=1;};
struct Path{Mint a,b;};
Path vertex(Vertex v,Mint light){return {v.a,v.a*(v.value+light)+v.b*v.count};}
Mint edge(Path p){return p.b;}
Path compress(Path x,Path y){return {x.a*y.a,x.a*y.b+x.b};}
Mint rake(Mint x,Mint y){return x+y;}
Mint identity(){return 0;}
int main(){
  std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
  int n,q;std::cin>>n>>q;std::vector<Vertex>a(n);
  for(auto&x:a){int value;std::cin>>value;x.value=value;}
  std::vector<std::vector<int>>g(n);std::vector<int>u(n-1),v(n-1),b(n-1),c(n-1);
  for(int i=0;i<n-1;++i){std::cin>>u[i]>>v[i]>>b[i]>>c[i];g[u[i]].push_back(v[i]);g[v[i]].push_back(u[i]);}
  std::vector<int>p(n,-1),order{0};p[0]=0;
  for(std::size_t i=0;i<order.size();++i)for(int w:g[order[i]])if(w!=p[order[i]]){p[w]=order[i];order.push_back(w);}
  for(int i=n-1;i>0;--i)a[p[order[i]]].count+=a[order[i]].count;
  std::vector<int>child(n-1);for(int i=0;i<n-1;++i){child[i]=p[u[i]]==v[i]?u[i]:v[i];a[child[i]].a=b[i];a[child[i]].b=c[i];}
  blueberry::StaticTopTree<Vertex,Path,Mint,vertex,edge,compress,rake,identity>t(g,a);
  while(q--){int type,index,x;std::cin>>type>>index>>x;
    if(type==0){a[index].value=x;t.set(index,a[index]);}
    else{int y;std::cin>>y;int w=child[index];a[w].a=x;a[w].b=y;t.set(w,a[w]);}
    std::cout<<t.all_prod().b.val()<<'\n';
  }
}

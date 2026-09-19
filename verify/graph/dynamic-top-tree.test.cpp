#define PROBLEM "https://judge.yosupo.jp/problem/point_set_tree_path_composite_sum"
#include <iostream>
#include <vector>
#include <atcoder/modint>
#include "blueberry/graph/dynamic-top-tree.hpp"
using Mint=atcoder::modint998244353;
struct Vertex{Mint a=1,b=0,value=0,count=0;};
struct Point{Mint sum,count;};
struct Path{Mint a,b,c,d;};
Path vertex(Vertex v,Point light){return {v.a,v.b,v.a*light.sum+v.b*light.count+v.value,light.count+v.count};}
Point edge(Path p){return {p.c,p.d};}
Path compress(Path x,Path y){return {x.a*y.a,x.a*y.b+x.b,x.a*y.c+x.b*y.d+x.c,x.d+y.d};}
Point rake(Point x,Point y){return {x.sum+y.sum,x.count+y.count};}
Point identity(){return {0,0};}
int main(){
  std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
  int n,q;std::cin>>n>>q;std::vector<Vertex>a(2*n-1);
  for(int i=0;i<n;++i){int x;std::cin>>x;a[i].value=x;a[i].count=1;}
  std::vector<int>u(n-1),v(n-1);
  for(int i=0;i<n-1;++i){int b,c;std::cin>>u[i]>>v[i]>>b>>c;a[n+i].a=b;a[n+i].b=c;}
  blueberry::DynamicTopTree<Vertex,Path,Point,vertex,edge,compress,rake,identity>t(a);
  // Edge nodes carry the bidirectional affine map, original vertices carry weight.
  for(int i=0;i<n-1;++i){t.link(u[i],n+i);t.link(n+i,v[i]);}
  while(q--){int type,index,x,r;std::cin>>type>>index>>x;
    if(type==0){std::cin>>r;a[index].value=x;t.set(index,a[index]);}
    else{int y;std::cin>>y>>r;a[n+index].a=x;a[n+index].b=y;t.set(n+index,a[n+index]);}
    std::cout<<t.all_prod(r).c.val()<<'\n';
  }
}

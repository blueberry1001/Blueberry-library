#include <cassert>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>
#include <atcoder/modint>
#include "blueberry/graph/dynamic-top-tree.hpp"
using Mint=atcoder::modint998244353;
struct Path{Mint a,b;int length;};
Path vertex(Mint value,Mint light){Mint x=value*light;return {x,x,1};}
Mint edge(Path path){return path.b+1;}
Path compress(Path x,Path y){return {x.a*y.a,x.a*y.b+x.b,x.length+y.length};}
Mint rake(Mint a,Mint b){return a*b;}
Mint identity(){return 1;}
int main(int argc,char**argv){
 blueberry::DynamicTopTree<Mint,Path,Mint,vertex,edge,compress,rake,identity>zero;
 assert(zero.size()==0);
 unsigned seed=argc>1?std::strtoul(argv[1],nullptr,10):20260919;std::mt19937 rng(seed);
 for(int n=1;n<=45;++n){
  std::vector<Mint>a(n);for(auto&x:a)x=rng()%5;
  blueberry::DynamicTopTree<Mint,Path,Mint,vertex,edge,compress,rake,identity>t(a);
  std::vector<std::vector<bool>>g(n,std::vector<bool>(n));
  auto order=[&](int root,int blocked){std::vector<int>p(n,-1),out{root};p[root]=root;if(blocked>=0)p[blocked]=blocked;
   for(std::size_t i=0;i<out.size();++i)for(int w=0;w<n;++w)if(g[out[i]][w]&&p[w]==-1){p[w]=out[i];out.push_back(w);}
   return std::pair{out,p};};
  auto dp=[&](int root,int blocked){auto[o,p]=order(root,blocked);std::vector<Mint>d(n);
   for(auto it=o.rbegin();it!=o.rend();++it){int v=*it;d[v]=a[v];for(int w=0;w<n;++w)if(w!=v&&p[w]==v)d[v]*=d[w]+1;}return d[root];};
  for(int step=0;step<3000;++step){int u=rng()%n,v=rng()%n,type=rng()%7;auto[o,p]=order(u,-1);bool same=p[v]!=-1;
   auto require=[&](bool ok){if(!ok){std::cerr<<"seed="<<seed<<" n="<<n<<" step="<<step<<" type="<<type<<" u="<<u<<" v="<<v<<'\n';std::abort();}};
   if(type==0){require(t.link(u,v)==!same);if(!same)g[u][v]=g[v][u]=true;}
   if(type==1){require(t.cut(u,v)==g[u][v]);g[u][v]=g[v][u]=false;}
   if(type==2){a[u]=rng()%7;t.set(u,a[u]);require(t.get(u)==a[u]);}
   if(type==3)require(t.same(u,v)==same);
   if(type==4){auto all=t.all_prod(u);require(all.b==dp(u,-1));require(all.length==1);if(same){auto result=t.prod(u,v);require(result.b==dp(u,-1));int length=1;for(int x=v;x!=u;x=p[x])++length;require(result.length==length);require(t.all_prod(u).length==1);}}
   if(type==5){int parent=-1;for(int w=0;w<n;++w)if(g[u][w]){parent=w;break;}require(t.subtree_prod(u,parent).b==dp(u,parent));}
   if(type==6){t.evert(u);require(t.leader(u)==u);if(same)require(t.leader(v)==u);}
   if(step%397==0){auto copy=t;copy.set(u,999);require(t.get(u)==a[u]);for(int root=0;root<n;++root)require(t.all_prod(root).b==dp(root,-1));}
  }
 }
}

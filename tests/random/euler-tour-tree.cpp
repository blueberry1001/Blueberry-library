#include <cassert>
#include <cstdlib>
#include <iostream>
#include <queue>
#include <random>
#include <vector>
#include "blueberry/graph/euler-tour-tree.hpp"
long long op(long long a,long long b){return a+b;}
long long e(){return 0;}
struct F{long long a,b;};
long long mapping(F f,long long x,int n){return f.a*x+f.b*n;}
F composition(F f,F g){return {f.a*g.a,f.a*g.b+f.b};}
F id(){return {1,0};}
int main(int argc,char**argv){
 blueberry::EulerTourTree<long long,op,e,F,mapping,composition,id>zero;
 assert(zero.size()==0);
 unsigned seed=argc>1?std::strtoul(argv[1],nullptr,10):20260919;std::mt19937 rng(seed);
 for(int n=1;n<=40;++n){
  std::vector<long long>a(n);blueberry::EulerTourTree<long long,op,e,F,mapping,composition,id>t(a);
  std::vector<std::vector<bool>>edge(n,std::vector<bool>(n));
  auto component=[&](int v,int p){std::vector<int>out{v};std::vector<bool>seen(n);seen[v]=true;if(p>=0)seen[p]=true;
   for(std::size_t i=0;i<out.size();++i)for(int w=0;w<n;++w)if(edge[out[i]][w]&&!seen[w]){seen[w]=true;out.push_back(w);}
   return out;};
  for(int step=0;step<3000;++step){int u=rng()%n,v=rng()%n,k=rng()%7;
   auto require=[&](bool ok){if(!ok){std::cerr<<"seed="<<seed<<" n="<<n<<" step="<<step<<" type="<<k<<" u="<<u<<" v="<<v<<'\n';std::abort();}};
   auto c=component(u,-1);bool same=false;for(int w:c)if(w==v)same=true;
   if(k==0){require(t.link(u,v)==!same);if(!same)edge[u][v]=edge[v][u]=true;}
   if(k==1){require(t.cut(u,v)==edge[u][v]);edge[u][v]=edge[v][u]=false;}
   if(k==2){a[u]=int(rng()%51)-25;t.set(u,a[u]);}
   if(k==3)require(t.same(u,v)==same);
   if(k>=4){int p=-1;if(rng()%2)for(int w=0;w<n;++w)if(edge[u][w]){p=w;break;}c=component(u,p);
    if(k==4){long long sum=0;for(int w:c)sum+=a[w];require(t.prod(u,p)==sum);}
    else{F f{int(rng()%3)-1,int(rng()%11)-5};t.apply(u,p,f);for(int w:c)a[w]=f.a*a[w]+f.b;}
   }
   for(int w=0;w<n;++w)require(t.get(w)==a[w]);
   if(step%997==0){auto copy=t;copy.set(u,999);require(t.get(u)==a[u]);}
  }
 }
}

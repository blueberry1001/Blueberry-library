#include <cassert>
#include <bit>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>
#include <atcoder/modint>
#include "blueberry/graph/static-top-tree.hpp"
using Mint=atcoder::modint998244353;
struct Affine{Mint a,b;};
int calls=0;
Affine vertex(Affine v,Mint light){++calls;return {v.a,v.a*light+v.b};}
Mint edge(Affine p){++calls;return p.b;}
Affine compress(Affine x,Affine y){++calls;return {x.a*y.a,x.a*y.b+x.b};}
Mint rake(Mint x,Mint y){++calls;return x+y;}
Mint identity(){return 0;}
int main(int argc,char**argv){
 unsigned seed=argc>1?std::strtoul(argv[1],nullptr,10):20260919;std::mt19937 rng(seed);
 for(int trial=0;trial<150;++trial){int n=1+rng()%150;std::vector<std::vector<int>>g(n);std::vector<int>p(n,-1);
  for(int v=1;v<n;++v){p[v]=trial%3==0?0:trial%3==1?v-1:rng()%v;g[v].push_back(p[v]);g[p[v]].push_back(v);}
  std::vector<Affine>a(n);for(auto&x:a)x={rng()%17,rng()%23};
  blueberry::StaticTopTree<Affine,Affine,Mint,vertex,edge,compress,rake,identity>t(g,a);
  for(int step=0;step<300;++step){int v=rng()%n;a[v]={rng()%17,rng()%23};calls=0;t.set(v,a[v]);
   if(calls>20*int(std::bit_width(unsigned(n)))+10)std::abort();
   std::vector<Mint>dp(n);for(int x=n-1;x>=0;--x){Mint s=0;for(int y:g[x])if(p[y]==x)s+=dp[y];dp[x]=a[x].a*s+a[x].b;}
   if(t.all_prod().b!=dp[0]){std::cerr<<"seed="<<seed<<" trial="<<trial<<" step="<<step<<'\n';std::abort();}
  }
 }
 for(int shape=0;shape<3;++shape){int n=100000;std::vector<std::vector<int>>g(n);for(int v=1;v<n;++v){int p=shape==0?0:shape==1?v-1:(v-1)/2;g[v].push_back(p);g[p].push_back(v);}
  std::vector<Affine>a(n,Affine{1,1});blueberry::StaticTopTree<Affine,Affine,Mint,vertex,edge,compress,rake,identity>t(g,a);
  assert(t.all_prod().b==n);for(int v:{0,n/2,n-1}){calls=0;t.set(v,{1,2});assert(calls<=20*int(std::bit_width(unsigned(n)))+10);}
  assert(t.all_prod().b==n+3);
 }
}

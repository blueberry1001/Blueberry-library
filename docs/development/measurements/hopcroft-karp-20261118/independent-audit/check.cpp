#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <random>
#include <utility>
#include <vector>
#include "candidate.hpp"

int brute(int l,int r,const std::vector<std::pair<int,int>>& edges){
  std::vector<unsigned> adj(l);for(auto [u,v]:edges)adj[u]|=1u<<v;
  std::vector<int> dp(1u<<r,-1);dp[0]=0;
  for(int u=0;u<l;++u){auto next=dp;for(unsigned mask=0;mask<dp.size();++mask)if(dp[mask]>=0){unsigned avail=adj[u]&~mask;while(avail){unsigned bit=avail&-avail;avail-=bit;auto x=mask|bit;next[x]=std::max(next[x],dp[mask]+1);}}dp.swap(next);}
  return *std::max_element(dp.begin(),dp.end());
}
bool check(int l,int r,const std::vector<std::pair<int,int>>& edges){
  blueberry::HopcroftKarp h(l,r,edges);int optimum=brute(l,r,edges);
  if(h.size()!=optimum)return false;
  std::vector<unsigned char> u(l),v(r),cu(l),cv(r);int cnt=0;
  for(auto [x,y]:h.pairs()){
    if(x<0||x>=l||y<0||y>=r||u[x]||v[y]||std::find(edges.begin(),edges.end(),std::pair<int,int>(x,y))==edges.end())return false;
    if(h.left_match(x)!=y||h.right_match(y)!=x)return false;
    ++cnt;u[x]=v[y]=1;
  }
  if(cnt!=optimum)return false;
  for(int i=0;i<l;++i)if(!u[i]&&h.left_match(i)!=-1)return false;
  for(int i=0;i<r;++i)if(!v[i]&&h.right_match(i)!=-1)return false;
  auto [left,right]=h.min_vertex_cover();
  if(left.size()+right.size()!=static_cast<std::size_t>(optimum))return false;
  for(int x:left){if(x<0||x>=l||cu[x])return false;cu[x]=1;}
  for(int y:right){if(y<0||y>=r||cv[y])return false;cv[y]=1;}
  for(auto [x,y]:edges)if(!cu[x]&&!cv[y])return false;
  auto copied=h;auto p=copied.pairs();auto cover=copied.min_vertex_cover();
  if(copied.size()!=optimum||p!=h.pairs()||cover!=std::pair(left,right))return false;
  auto moved=std::move(copied);if(moved.pairs()!=p||moved.min_vertex_cover()!=cover)return false;
  return true;
}
int main(){
  for(auto shape:{std::pair{5,4},std::pair{4,5}}){int l=shape.first,r=shape.second;unsigned m=1u<<(l*r);
    for(unsigned mask=0;mask<m;++mask){std::vector<std::pair<int,int>>e;for(int u=0;u<l;++u)for(int v=0;v<r;++v)if(mask>>(u*r+v)&1u)e.emplace_back(u,v);
      if(!check(l,r,e)){std::printf("FAIL exhaustive l=%d r=%d mask=%u\n",l,r,mask);return 1;}}
  }
  std::mt19937 gen(0x91ec742a);for(int z=0;z<30000;++z){int l=gen()%9,r=gen()%9;std::vector<std::pair<int,int>>e;if(l&&r)for(int i=0,n=gen()%100;i<n;++i)e.emplace_back(gen()%l,gen()%r);
    std::shuffle(e.begin(),e.end(),gen);if(!check(l,r,e)){std::printf("FAIL randomized %d\n",z);return 2;}}
  std::puts("PASS: all 5x4 and 4x5 graphs; 30000 randomized multigraphs incl. empty; copies/moves and cover");
}

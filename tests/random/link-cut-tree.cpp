#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <queue>
#include <random>
#include <string>
#include <vector>
#include "blueberry/graph/link-cut-tree.hpp"
std::string join(std::string a, std::string b) { return a + b; }
std::string empty() { return {}; }
int main(int argc,char**argv) {
  blueberry::LinkCutTree<std::string,join,empty> zero;
  if(zero.size()!=0)std::abort();
  unsigned seed=argc>1?static_cast<unsigned>(std::strtoul(argv[1],nullptr,10)):20260919;
  std::mt19937 rng(seed);
  for(int n=1;n<=45;++n){
    std::vector<std::string>a(n);for(auto&x:a)x=char('a'+rng()%26);
    blueberry::LinkCutTree<std::string,join,empty> t(a);
    std::vector<std::vector<bool>> edge(n,std::vector<bool>(n));
    auto path=[&](int u,int v){
      std::vector<int>p(n,-1);std::queue<int>q;q.push(u);p[u]=u;
      while(!q.empty()){int x=q.front();q.pop();for(int y=0;y<n;++y)if(edge[x][y]&&p[y]<0){p[y]=x;q.push(y);}}
      std::vector<int>r;if(p[v]<0)return r;
      for(int x=v;;x=p[x]){r.push_back(x);if(x==u)break;}
      std::reverse(r.begin(),r.end());return r;
    };
    for(int step=0;step<4000;++step){
      int u=rng()%n,v=rng()%n,kind=rng()%5;auto p=path(u,v);
      auto require=[&](bool ok){if(!ok){std::cerr<<"seed="<<seed<<" n="<<n<<" step="<<step<<" type="<<kind<<" u="<<u<<" v="<<v<<'\n';std::abort();}};
      if(kind==0){bool expected=p.empty();require(t.link(u,v)==expected);if(expected)edge[u][v]=edge[v][u]=true;}
      if(kind==1){bool expected=edge[u][v];require(t.cut(u,v)==expected);edge[u][v]=edge[v][u]=false;}
      if(kind==2){a[u]=char('a'+rng()%26);t.set(u,a[u]);require(t.get(u)==a[u]);}
      if(kind>=3){require(t.same(u,v)==!p.empty());if(!p.empty()){std::string expected;for(int x:p)expected+=a[x];require(t.prod(u,v)==expected);std::reverse(expected.begin(),expected.end());require(t.prod(v,u)==expected);}}
      if(step%701==0){auto copy=t;require(copy.get(u)==a[u]);copy.set(u,"!");require(t.get(u)==a[u]);}
      if(step%17==0){t.evert(u);for(int w:path(u,v))require(t.leader(w)==u);require(t.leader(u)==u);}
    }
  }
}

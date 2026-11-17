#include <algorithm>
#include <chrono>
#include <cstdio>
#include <random>
#include <vector>
#include "blueberry/graph/tree-diameter.hpp"
using namespace std;
using Graph = vector<vector<pair<int,int>>>;
pair<int,vector<int>> candidate(const Graph& g) {
 int n = int(g.size()); if (!n) return {0,{}};
 vector<int> p(n), order; vector<int> d(n); order.reserve(n);
 auto farthest = [&](int root) {
  order.clear(); order.push_back(root); p[root]=root; d[root]=0; int best=root;
  for (size_t i=0; i<order.size(); ++i) { int v=order[i];
   for (auto [u,w]:g[v]) { if (u==p[v]) continue; p[u]=v; d[u]=d[v]+w; order.push_back(u); if (d[best]<d[u]) best=u; }
  } return best;
 };
 int a=farthest(0), b=farthest(a); vector<int> path;
 for(int v=b;;v=p[v]) {path.push_back(v);if(v==a)break;}
 reverse(path.begin(),path.end()); return {d[b],move(path)};
}
int main(){mt19937 rng(123456); for (int n:{10000,100000,1000000}) for(int shape=0;shape<3;++shape){Graph g(n);for(int i=1;i<n;++i){int p=shape==0?i-1:shape==1?0:rng()%i; int w=rng()%10;g[i].emplace_back(p,w);g[p].emplace_back(i,w);} auto a=blueberry::tree_diameter(g),b=candidate(g);if(a.first!=b.first||a.second!=b.second)return 2; for(int rep=0;rep<9;++rep)for(int k=0;k<2;++k){int v=(k+rep)%2;auto start=chrono::steady_clock::now();long long sum=0;for(int j=0;j<5;++j)sum+=(v?candidate(g):blueberry::tree_diameter(g)).first;auto end=chrono::steady_clock::now();printf("%d %d %d %d %.6f %lld\n",n,shape,rep,v,chrono::duration<double,milli>(end-start).count()/5,sum);}}}

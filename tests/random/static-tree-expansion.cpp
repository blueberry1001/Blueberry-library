#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <numeric>
#include <queue>
#include <random>
#include <vector>
#include "blueberry/data-structure/kd-tree.hpp"
#include "blueberry/data-structure/linear-rmq.hpp"
#include "blueberry/graph/linear-lca.hpp"
#include "blueberry/graph/contour-query.hpp"

long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
unsigned long long seed;
int trial, step;
void check(bool ok, const char* what) {
  if (ok) return;
  std::cerr << "seed=" << seed << " trial=" << trial << " step=" << step << " operation=" << what << '\n';
  std::abort();
}
int main(int argc, char** argv) {
  seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  std::mt19937_64 rng(seed);
  for (trial = 0; trial < 160; ++trial) {
    int n = trial % 65;
    std::vector<int> a(n); for (int& x : a) x = int(rng() % 11) - 5;
    blueberry::LinearRMQ<int> rmq(a);
    blueberry::LinearRMQ<int, std::greater<int>> maxq(a);
    for (int l = 0; l < n; ++l) for (int r = l + 1; r <= n; ++r) {
      int p = int(std::min_element(a.begin() + l, a.begin() + r) - a.begin());
      int q = int(std::max_element(a.begin() + l, a.begin() + r) - a.begin());
      check(rmq.argmin(l,r) == p && rmq.prod(l,r) == a[p], "rmq leftmost");
      check(maxq.argmin(l,r) == q, "rmq comparator");
    }
    std::vector<std::pair<int,int>> xy(n);
    std::vector<long long> weights(n);
    for (auto& [x,y] : xy) { x = int(rng()%9)-4; y = int(rng()%9)-4; }
    blueberry::KDTree<int,long long,op,e> kd(xy,weights);
    for (step=0;step<120;++step) {
      if(n){int p=rng()%n;weights[p]=int(rng()%101)-50;kd.set(p,weights[p]);check(kd.get(p)==weights[p],"kd get");}
      int xl=int(rng()%13)-6,xr=int(rng()%13)-6,yl=int(rng()%13)-6,yr=int(rng()%13)-6;
      if(xl>xr)std::swap(xl,xr);
      if(yl>yr)std::swap(yl,yr);
      long long sum=0;for(int i=0;i<n;++i)if(xl<=xy[i].first&&xy[i].first<xr&&yl<=xy[i].second&&xy[i].second<yr)sum+=weights[i];
      check(kd.prod(xl,yl,xr,yr)==sum,"kd rectangle");
      check(kd.all_prod()==std::accumulate(weights.begin(),weights.end(),0LL),"kd all");
    }
    if(!n)continue;
    std::vector<std::vector<int>>g(n);std::vector<int>parent(n,-1),depth(n);
    for(int v=1;v<n;++v){int p=trial%3==0?0:trial%3==1?v-1:int(rng()%v);parent[v]=p;depth[v]=depth[p]+1;g[p].push_back(v);g[v].push_back(p);}
    std::vector<std::vector<int>> dist(n,std::vector<int>(n,-1));
    for(int v=0;v<n;++v){std::queue<int>q;q.push(v);dist[v][v]=0;while(!q.empty()){int u=q.front();q.pop();for(int w:g[u])if(dist[v][w]<0){dist[v][w]=dist[v][u]+1;q.push(w);}}}
    blueberry::LinearLCA lca(g);blueberry::CentroidDecomposition cd(g);
    blueberry::ContourQuery<long long> sums(g);blueberry::ContourAdd<long long> adds(g);
    std::vector<long long> points(n),dual(n);
    for(int u=0;u<n;++u)for(int v=0;v<n;++v){int x=u,y=v;while(depth[x]>depth[y])x=parent[x];while(depth[y]>depth[x])y=parent[y];while(x!=y){x=parent[x];y=parent[y];}
      check(lca.lca(u,v)==x&&lca.distance(u,v)==dist[u][v],"lca");}
    for(int v=0;v<n;++v)for(auto p:cd.ancestors(v))check(p.distance==dist[v][p.centroid],"centroid ancestor distance");
    std::vector<std::vector<int>> pairs(n,std::vector<int>(n));std::vector<int> occurrences(n);
    blueberry::centroid_decomposition_thirds(g,[&](const auto& ids,const auto& par,const auto& color){
      std::vector<int>d(ids.size());check(par[0]==-1,"thirds root");
      for(int i=0;i<int(ids.size());++i){if(i){check(0<=par[i]&&par[i]<i,"thirds parent order");d[i]=d[par[i]]+1;}
        if(color[i]>=0)++occurrences[ids[i]];
        for(int j=0;j<i;++j)if(color[i]>=0&&color[j]>=0&&color[i]!=color[j]){
          ++pairs[ids[i]][ids[j]];++pairs[ids[j]][ids[i]];
          check(dist[ids[i]][ids[j]]==d[i]+d[j],"thirds separator path");}}
    });
    for(int u=0;u<n;++u){check(occurrences[u]<=4*int(std::bit_width(unsigned(n)))+2,"thirds real occurrence bound");for(int v=0;v<n;++v)check(pairs[u][v]==(u!=v),"thirds exact pair partition");}
    for(step=0;step<150;++step){int v=rng()%n,l=rng()%(n+3),r=rng()%(n+3);if(l>r)std::swap(l,r);long long x=int(rng()%101)-50;
      points[v]+=x;sums.add(v,x);adds.add(v,l,r,x);
      for(int u=0;u<n;++u)if(l<=dist[v][u]&&dist[v][u]<r)dual[u]+=x;
      long long expected=0;for(int u=0;u<n;++u)if(l<=dist[v][u]&&dist[v][u]<r)expected+=points[u];
      check(sums.sum(v,l,r)==expected,"contour sum");for(int u=0;u<n;++u)check(adds.get(u)==dual[u],"contour add");
    }
  }
  // Exercise large mask-block boundaries and adversarial equal coordinates.
  for(int n : {127,128,129,1023,1024,1025,65535,65536,65537}) {
    std::vector<int>a(n);for(auto&x:a)x=rng()%101;
    blueberry::LinearRMQ<int> rmq(a);
    for(int k=0;k<100;++k){int l=rng()%n,r=l+1+rng()%(n-l);check(rmq.argmin(l,r)==std::min_element(a.begin()+l,a.begin()+r)-a.begin(),"large rmq");}
  }
  using KD = blueberry::KDTree<int,long long,op,e>;
  const int lo = std::numeric_limits<int>::min(), hi = std::numeric_limits<int>::max();
  KD extremes({{lo,lo},{hi,hi},{0,0}}, {2,3,5});
  check(extremes.prod(lo,lo,hi,hi)==7 && extremes.all_prod()==10,"kd extreme coordinates");
  auto copied = extremes;
  copied.set(0,9);
  check(extremes.get(0)==2 && copied.get(0)==9,"kd independent copy");
  std::vector<std::vector<int>> single(1);
  blueberry::ContourQuery<long long> point(single);
  point.add(0,7);
  check(point.sum(0,0,hi)==7 && point.sum(0,hi,hi)==0,"contour INT_MAX radius");
  blueberry::ContourAdd<long long> range(single);
  range.add(0,0,hi,11);
  check(range.get(0)==11,"contour add INT_MAX radius");
}

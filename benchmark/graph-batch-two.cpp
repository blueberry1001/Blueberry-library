#include <algorithm>
#include <chrono>
#include <functional>
#include <cstdlib>
#include <iostream>
#include <queue>
#include <random>
#include "blueberry/graph/cycle-detection.hpp"
#include "blueberry/graph/topological-sort.hpp"
#include "blueberry/graph/tree-diameter.hpp"
#include "blueberry/graph/triangle-enumeration.hpp"
using namespace std;
using Clock = chrono::steady_clock;
long long sink = 0;
long long previous = -1;
int candidate = 0;
template<class F> void measure(const char* label, F f) {
  vector<double> times;
  long long result = -1;
  for(int r=0;r<5;++r) {
    auto t=Clock::now(); auto value=static_cast<long long>(f());
    times.push_back(chrono::duration<double,milli>(Clock::now()-t).count());
    if(result != -1 && result != value) std::abort();
    result=value; sink += value;
  }
  if(candidate++ % 2 == 0) previous=result;
  else if(previous != result) std::abort();
  cout << label << " checksum=" << result << " samples_ms=";
  for(double t:times) cout << t << ',';
  cout << '\n';
  sort(times.begin(),times.end());
  cout << label << " min_ms=" << times.front() << " median_ms=" << times[2] << '\n';
}
int main() {
  mt19937 rng(20260918); int n=20000;
  vector<vector<int>> dag(n); vector<pair<int,int>> edges;
  for(int i=0;i<100000;++i) { int a=rng()%n,b=rng()%n; if(a==b) continue; if(a>b) swap(a,b); dag[a].push_back(b); edges.emplace_back(a,b); }
  measure("cycle_iterative",[&] { return blueberry::find_cycle<true>(n,edges).size(); });
  measure("cycle_recursive",[&] { vector<vector<int>> g(n); for(auto[a,b]:edges) g[a].push_back(b); vector<int> state(n); auto dfs=[&](auto&& self,int v)->bool {state[v]=1; for(int u:g[v]) {if(state[u]==1) return true; if(!state[u] && self(self,u)) return true;} state[v]=2; return false;}; for(int v=0;v<n;++v) if(!state[v]&&dfs(dfs,v)) return 1; return 0; });
  measure("topological_vector",[&] { return blueberry::topological_sort(dag)->size(); });
  measure("topological_queue",[&] {vector<int>d(n),order; for(auto&row:dag)for(int v:row)++d[v]; queue<int> q;for(int v=0;v<n;++v)if(!d[v])q.push(v);while(!q.empty()){int v=q.front();q.pop();order.push_back(v);for(int u:dag[v])if(!--d[u])q.push(u);}return order.size();});
  vector<vector<pair<int,long long>>> tree(n);
  for(int i=1;i<n;++i){int p=rng()%i;long long w=rng()%100;tree[i].emplace_back(p,w);tree[p].emplace_back(i,w);}
  measure("diameter_iterative",[&]{return blueberry::tree_diameter(tree).first;});
  measure("diameter_recursive",[&]{vector<int> parent(n);vector<long long>d(n);auto run=[&](int root){int best=root;d[root]=0;auto dfs=[&](auto&&self,int v,int p)->void{parent[v]=p;for(auto[u,w]:tree[v])if(u!=p){d[u]=d[v]+w;if(d[u]>d[best])best=u;self(self,u,v);}};dfs(dfs,root,-1);return best;};int a=run(0),b=run(a);vector<int>path;for(int v=b;v!=-1;v=parent[v])path.push_back(v);return d[b];});
  int tn=1800;vector<pair<int,int>> simple;vector<vector<unsigned char>> matrix(tn,vector<unsigned char>(tn));
  for(int a=0;a<tn;++a)for(int b=a+1;b<tn;++b)if(rng()%100<4){simple.emplace_back(a,b);matrix[a][b]=matrix[b][a]=1;}
  cout << "dag_n="<<n<<" dag_m="<<edges.size()<<" triangle_n="<<tn<<" triangle_m="<<simple.size()<<" runs=5 seed=20260918\n";
  measure("triangles_degree_mark",[&]{long long c=0;blueberry::enumerate_triangles(tn,simple,[&](int,int,int){++c;});return c;});
  measure("triangles_neighbor_pairs",[&]{long long c=0;vector<vector<unsigned char>> local(tn,vector<unsigned char>(tn));vector<vector<int>>out(tn);for(auto[a,b]:simple){out[a].push_back(b);local[a][b]=local[b][a]=1;}for(int a=0;a<tn;++a)for(size_t i=0;i<out[a].size();++i)for(size_t j=i+1;j<out[a].size();++j)c+=local[out[a][i]][out[a][j]];return c;});
  cerr<<"checksum="<<sink<<'\n';
}

// Independently written alternatives for comparable, reproducible measurements.
#include <bit>
#include <chrono>
#include <iostream>
#include <limits>
#include <queue>
#include <random>
#include <atcoder/modint>
#include "blueberry/graph/assignment.hpp"
#include "blueberry/graph/complement-components.hpp"
#include "blueberry/graph/count-spanning-trees.hpp"
#include "blueberry/graph/minimum-spanning-forest.hpp"
using namespace std;
using mint = atcoder::modint998244353;
long long subset_assignment(const vector<vector<long long>>& a) {
  int n = a.size(); vector<long long> dp(1 << n, numeric_limits<long long>::max()/4); dp[0] = 0;
  for (unsigned s = 0; s < dp.size(); ++s) {
    int row = popcount(s); if (row == n) continue;
    for (int j = 0; j < n; ++j) if (!(s >> j & 1)) dp[s | (1u << j)] = min(dp[s | (1u << j)], dp[s] + a[row][j]);
  }
  return dp.back();
}
int dense_complement(const vector<vector<int>>& g) {
  int n = g.size(), count = 0; vector<vector<bool>> a(n, vector<bool>(n));
  for (int v = 0; v < n; ++v) for (int u : g[v]) a[v][u] = true;
  vector<bool> seen(n); vector<int> q;
  for (int root = 0; root < n; ++root) if (!seen[root]) {
    ++count; q = {root}; seen[root] = true;
    for (size_t h = 0; h < q.size(); ++h) for (int u = 0; u < n; ++u) if (!seen[u] && !a[q[h]][u]) { seen[u] = true; q.push_back(u); }
  }
  return count;
}
mint subset_det(int n, const vector<pair<int,int>>& e) {
  --n; vector<vector<mint>> a(n, vector<mint>(n));
  for (auto [u,v] : e) { if (u < n) a[u][u] += 1; if (v < n) a[v][v] += 1; if (u < n && v < n) { a[u][v] -= 1; a[v][u] -= 1; } }
  vector<mint> dp(1 << n); dp[0] = 1;
  for (unsigned s = 0; s < dp.size(); ++s) { int row = popcount(s); if (row == n) continue;
    for (int col = 0; col < n; ++col) if (!(s >> col & 1)) { mint x = dp[s]*a[row][col]; if (popcount(s >> col)&1) x = -x; dp[s | (1u << col)] += x; }
  }
  return dp.back();
}
long long prim(int n, const vector<tuple<int,int,long long>>& e) {
  vector<vector<pair<long long,int>>> g(n); for (auto [u,v,w] : e) { g[u].emplace_back(w,v); g[v].emplace_back(w,u); }
  priority_queue<pair<long long,int>,vector<pair<long long,int>>,greater<pair<long long,int>>> q;
  vector<bool> used(n); long long sum=0;
  for(int root=0;root<n;++root) if(!used[root]) { q.emplace(0,root); while(!q.empty()) { auto [w,v]=q.top();q.pop();if(used[v])continue;used[v]=true;sum+=w;for(auto [c,u]:g[v])if(!used[u])q.emplace(c,u); } }
  return sum;
}
template<class F> void measure(const char* name, F f) {
  for (int run=0;run<3;++run) { auto begin=chrono::steady_clock::now(); auto checksum=f(); auto end=chrono::steady_clock::now();
    cout << name << ',' << run << ',' << chrono::duration<double,milli>(end-begin).count() << ',' << checksum << '\n'; }
}
int main() {
  mt19937 rng(20260918);
  vector<vector<long long>> a(18,vector<long long>(18)); for(auto& row:a)for(auto& x:row)x=int(rng()%2001)-1000;
  vector<vector<int>> g(3000); for(int i=0;i<18000;++i){int u=rng()%g.size(),v=rng()%g.size();g[u].push_back(v);g[v].push_back(u);}
  vector<pair<int,int>> e;for(int i=0;i<100;++i)e.emplace_back(rng()%18,rng()%18);
  vector<tuple<int,int,long long>> w;for(int i=0;i<120000;++i)w.emplace_back(rng()%30000,rng()%30000,int(rng()%200001)-100000);
  cout << "candidate,run,milliseconds,checksum\n";
  measure("assignment_hungarian_n18",[&]{return blueberry::assignment(a).first;});
  measure("assignment_subset_dp_n18",[&]{return subset_assignment(a);});
  measure("complement_linked_n3000_m18000",[&]{return blueberry::complement_components(g).size();});
  measure("complement_dense_n3000_m18000",[&]{return dense_complement(g);});
  measure("tree_count_elimination_n18_m100",[&]{return blueberry::count_spanning_trees<mint>(18,e).val();});
  measure("tree_count_subset_det_n18_m100",[&]{return subset_det(18,e).val();});
  measure("forest_kruskal_n30000_m120000",[&]{return blueberry::minimum_spanning_forest(30000,w).first;});
  measure("forest_prim_n30000_m120000",[&]{return prim(30000,w);});
}

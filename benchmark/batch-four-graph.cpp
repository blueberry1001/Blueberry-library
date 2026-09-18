// Comparable independent reference candidates, not copied external code.
#include <algorithm>
#include <bit>
#include <chrono>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include "blueberry/graph/clique-enumeration.hpp"
#include "blueberry/graph/maximum-independent-set.hpp"
#include "blueberry/graph/rooted-tree-isomorphism.hpp"
using namespace std;
int main() {
  mt19937 rng(20260918);
  auto measure = [&](string name, auto run) {
    for (int repeat = 0; repeat < 3; ++repeat) {
      auto start = chrono::steady_clock::now();
      auto checksum = run();
      double ms = chrono::duration<double, milli>(chrono::steady_clock::now() - start).count();
      cout << name << ',' << repeat << ',' << ms << ',' << checksum << '\n';
    }
  };
  vector<int> parent(4000); iota(parent.begin(), parent.end(), -1);
  measure("tree-vector-keys-path4000", [&] { return blueberry::rooted_tree_isomorphism(parent)[0]; });
  measure("tree-parentheses-path4000", [&] {
    map<string,int> ids; string key; int id = 0;
    for (int v = 3999; v >= 0; --v) { key = "(" + key + ")"; id = ids.try_emplace(key, ids.size()).first->second; }
    return id;
  });
  int n = 24; vector<pair<int,int>> edges; vector<unsigned> adj(n);
  for (int u = 0; u < n; ++u) for (int v = u+1; v < n; ++v) if (rng()%4 == 0) {
    edges.emplace_back(u,v); adj[u] |= 1U << v; adj[v] |= 1U << u;
  }
  measure("mis-meet-middle-n24", [&] { return blueberry::maximum_independent_set(n, edges).size(); });
  measure("mis-subset-dp-n24", [&] {
    vector<unsigned char> dp(1U << n);
    for (unsigned s=1;s<(1U<<n);++s) {
      int v=countr_zero(s); unsigned r=s&(s-1);
      dp[s]=max<int>(dp[r],1+dp[r&~adj[v]]);
    }
    return int(dp.back());
  });
  n=20; edges.clear(); adj.assign(n,0);
  for(int u=0;u<n;++u) for(int v=u+1;v<n;++v) if(rng()%2) {
    edges.emplace_back(u,v);adj[u]|=1U<<v;adj[v]|=1U<<u;
  }
  measure("cliques-oriented-n20", [&] {
    int count=0;blueberry::enumerate_cliques(n,edges,[&](const auto&){++count;});return count;
  });
  measure("cliques-subsets-n20", [&] {
    vector<bool> good(1U<<n);good[0]=true;int count=0;
    for(unsigned s=1;s<(1U<<n);++s) {
      int v=countr_zero(s);unsigned r=s&(s-1);
      good[s]=good[r]&&!(r&~adj[v]);count+=good[s];
    }
    return count;
  });
}

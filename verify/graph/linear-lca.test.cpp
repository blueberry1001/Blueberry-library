#define PROBLEM "https://judge.yosupo.jp/problem/lca"
#include <iostream>
#include <vector>
#include "blueberry/graph/linear-lca.hpp"
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n,q; std::cin>>n>>q; std::vector<std::vector<int>> g(n);
  for(int v=1;v<n;++v){int p;std::cin>>p;g[p].push_back(v);g[v].push_back(p);}
  blueberry::LinearLCA lca(g);
  while(q--){int u,v;std::cin>>u>>v;std::cout<<lca.lca(u,v)<<'\n';}
}

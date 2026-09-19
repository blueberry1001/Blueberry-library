#define PROBLEM "https://judge.yosupo.jp/problem/vertex_get_range_contour_add_on_tree"
#include <iostream>
#include <vector>
#include "blueberry/graph/contour-query.hpp"
int main(){
  std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
  int n,q;std::cin>>n>>q;std::vector<long long>a(n);for(auto&x:a)std::cin>>x;
  std::vector<std::vector<int>>g(n);
  for(int i=1;i<n;++i){int u,v;std::cin>>u>>v;g[u].push_back(v);g[v].push_back(u);}
  blueberry::ContourAdd<long long> tree(g);
  while(q--){int t,v;std::cin>>t>>v;if(t==0){int l,r;long long x;std::cin>>l>>r>>x;tree.add(v,l,r,x);}
    else std::cout<<a[v]+tree.get(v)<<'\n';}
}

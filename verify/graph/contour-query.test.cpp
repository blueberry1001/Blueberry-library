#define PROBLEM "https://judge.yosupo.jp/problem/vertex_add_range_contour_sum_on_tree"
#include <iostream>
#include <vector>
#include "blueberry/graph/contour-query.hpp"
int main(){
  std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
  int n,q;std::cin>>n>>q;std::vector<long long>a(n);for(auto&x:a)std::cin>>x;
  std::vector<std::vector<int>>g(n);
  for(int i=1;i<n;++i){int u,v;std::cin>>u>>v;g[u].push_back(v);g[v].push_back(u);}
  blueberry::ContourQuery<long long> tree(g);
  for(int i=0;i<n;++i)tree.add(i,a[i]);
  while(q--){int t,v;std::cin>>t>>v;if(t==0){long long x;std::cin>>x;tree.add(v,x);}
    else{int l,r;std::cin>>l>>r;std::cout<<tree.sum(v,l,r)<<'\n';}}
}

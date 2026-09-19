#define PROBLEM "https://judge.yosupo.jp/problem/frequency_table_of_tree_distance"
#include <algorithm>
#include <iostream>
#include <vector>
#include <atcoder/convolution>
#include "blueberry/graph/centroid-decomposition.hpp"
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n; std::cin>>n; std::vector<std::vector<int>> g(n);
  for(int i=1;i<n;++i){int u,v;std::cin>>u>>v;g[u].push_back(v);g[v].push_back(u);}
  std::vector<long long> answer(n);
  blueberry::centroid_decomposition_thirds(g,[&](const auto& ids,const auto& parent,const auto& color){
    std::vector<int> d(ids.size());
    std::vector<long long> a,b;
    for(int i=0;i<int(ids.size());++i){
      if(i)d[i]=d[parent[i]]+1;
      if(color[i]<0)continue;
      auto& f=color[i]==0?a:b;
      if(int(f.size())<=d[i])f.resize(d[i]+1);
      ++f[d[i]];
    }
    // Every pair occurs once across the two real colors; virtuals carry paths only.
    auto c=atcoder::convolution_ll(a,b);
    for(int i=1;i<std::min(n,int(c.size()));++i)answer[i]+=c[i];
  });
  for(int i=1;i<n;++i)std::cout<<answer[i]<<(i+1==n?'\n':' ');
}

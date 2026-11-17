#include <algorithm>
#include <numeric>
#include <cstdint>
#include <cstdio>
#include <random>
#include <utility>
#include <vector>
#include "instrumented.hpp"
int main() {
  std::mt19937_64 rng(20261117);
  for (int trial = 0; trial < 2000; ++trial) {
    std::vector<std::pair<int,int>> e;
    for (int g=0;g<8;++g) for(int i=0;i<5;++i) for(int j=i+1;j<5;++j) e.emplace_back(5*g+i,5*g+j);
    for (int g=0;g<7;++g) e.emplace_back(5*g+rng()%5,5*(g+1)+rng()%5);
    std::vector<int> perm(40); std::iota(perm.begin(),perm.end(),0); std::shuffle(perm.begin(),perm.end(),rng);
    for (auto& [u,v] : e) { u=perm[u]; v=perm[v]; }
    auto result=blueberry::maximum_independent_set(40,e);
    if(result.size()!=8) return 2;
    if(blueberry::budget_exhausted) {
      printf("trial %d\n",trial);
      for (auto [u,v] : e) printf("%d %d\n",u,v);
      return 0;
    }
  }
  return 1;
}

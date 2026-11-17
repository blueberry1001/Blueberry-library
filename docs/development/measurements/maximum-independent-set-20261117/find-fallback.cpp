#include <cstdint>
#include <cstdio>
#include <random>
#include <utility>
#include <vector>
#include "instrumented.hpp"
int main() {
  std::mt19937_64 rng(20261117);
  for (int trial=0; trial<10000; ++trial) {
    std::vector<std::pair<int,int>> e;
    unsigned density=2+trial%58;
    for(int i=0;i<40;++i) for(int j=i+1;j<40;++j) if(rng()%100<density) e.emplace_back(i,j);
    auto result=blueberry::maximum_independent_set(40,e);
    if(blueberry::budget_exhausted) {
      printf("trial %d density %u answer %zu\n",trial,density,result.size());
      for(auto [u,v]:e) printf("%d %d\n",u,v);
      return 0;
    }
  }
  return 1;
}

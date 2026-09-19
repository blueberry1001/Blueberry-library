#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>
#include "blueberry/data-structure/persistent-array.hpp"
#include "blueberry/data-structure/persistent-union-find.hpp"
#include "blueberry/data-structure/range-parallel-union-find.hpp"
using namespace std;
int main(int argc, char** argv) {
  const auto seed = argc > 1 ? strtoull(argv[1], nullptr, 10) : 1;
  mt19937_64 rng(seed);
  auto require = [&](bool ok, const char* operation, int n, int step) {
    if (!ok) { cerr << "seed=" << seed << " operation=" << operation << " n=" << n << " step=" << step << " expected=brute_version actual=mismatch\n"; exit(1); }
  };
  for (int n : {0,1,2,3,31,32,33,65}) {
    blueberry::PersistentArray<int> array(n, -7);
    blueberry::PersistentUnionFind uf(n);
    vector<vector<int>> arrays(1, vector<int>(n, -7)), partitions(1, vector<int>(n));
    iota(partitions[0].begin(), partitions[0].end(), 0);
    assert(array.size() == n && uf.size() == n && uf.components(0) == n);
    for (int step=0; step<500; ++step) {
      int version = rng()%arrays.size();
      if (n && rng()%3) {
        int p=rng()%n, value=static_cast<int>(rng()%101)-50;
        auto next=arrays[version]; next[p]=value;
        require(array.set(version,p,value)==static_cast<int>(arrays.size()),"array version",n,step);
        arrays.push_back(next);
      } else { require(array.fork(version)==static_cast<int>(arrays.size()),"fork",n,step); arrays.push_back(arrays[version]); }
      for (int k=0;k<5&&n;++k) { int v=rng()%arrays.size(),p=rng()%n; require(array.get(v,p)==arrays[v][p],"array get",n,step); }
      if (!n) continue;
      version=rng()%partitions.size(); int a=rng()%n,b=rng()%n;
      auto next=partitions[version]; int x=next[a], y=next[b];
      for (int& p:next) if(p==y)p=x;
      require(uf.merge(version,a,b)==static_cast<int>(partitions.size()),"UF version",n,step);
      partitions.push_back(next);
      for(int check=0;check<5;++check) {
        int v=rng()%partitions.size(),p=rng()%n,q=rng()%n;
        require(uf.same(v,p,q)==(partitions[v][p]==partitions[v][q]),"UF same",n,step);
        require(uf.comp_size(v,p)==count(partitions[v].begin(),partitions[v].end(),partitions[v][p]),"UF size",n,step);
        auto unique=partitions[v];sort(unique.begin(),unique.end());unique.erase(std::unique(unique.begin(),unique.end()),unique.end());
        require(uf.components(v)==static_cast<int>(unique.size()),"UF components",n,step);
      }
    }
    blueberry::PersistentArray<int> built(vector<int>(n,19));
    for(int p=0;p<n;++p)assert(built.get(0,p)==19);
    auto copied=array; if(n){copied.set(0,0,123);assert(array.get(0,0)==-7);}
    blueberry::RangeParallelUnionFind ranged(n);
    vector<int> partition(n), sums(n,1);iota(partition.begin(),partition.end(),0);
    int callbacks=0;
    for(int step=0;step<1000;++step) {
      int length=rng()%(n+1),a=rng()%(n-length+1),b=rng()%(n-length+1);
      ranged.range_merge(a,b,length,[&](int kept,int removed){++callbacks;sums[kept]+=sums[removed];});
      for(int i=0;i<length;++i) {int x=partition[a+i],y=partition[b+i];for(int& p:partition)if(p==y)p=x;}
      auto unique=partition;sort(unique.begin(),unique.end());unique.erase(std::unique(unique.begin(),unique.end()),unique.end());
      require(ranged.components()==static_cast<int>(unique.size())&&callbacks==n-ranged.components(),"range callback",n,step);
      for(int i=0;i<n;++i) { require(ranged.comp_size(i)==count(partition.begin(),partition.end(),partition[i]),"range size",n,step);require(sums[ranged.leader(i)]==ranged.comp_size(i),"range sum callback",n,step);for(int j=0;j<n;++j)require(ranged.same(i,j)==(partition[i]==partition[j]),"range same",n,step); }
    }
  }
}

// Standalone reproducible comparison: g++ -std=gnu++20 -O2 -I. benchmarks/sqrt-tree.cpp
#include "blueberry/data-structure/sqrt-tree.hpp"
#include "blueberry/data-structure/disjoint-sparse-table.hpp"
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <utility>
#include <vector>
using Clock = std::chrono::steady_clock;
using Value = std::uint64_t;
Value sum(Value a, Value b) { return a + b; }
Value zero() { return 0; }
template <class Build>
void measure(const char* name, int n, int run, Build build, const std::vector<std::pair<int,int>>& queries) {
  auto begin = Clock::now();
  auto table = build();
  auto built = Clock::now();
  Value checksum = 0;
  for (auto [l,r] : queries) checksum += table.prod(l,r);
  auto end = Clock::now();
  std::cout << name << ',' << n << ',' << queries.size() << ',' << run << ','
            << std::chrono::duration<double,std::milli>(built-begin).count() << ','
            << std::chrono::duration<double,std::milli>(end-built).count() << ',' << checksum << '\n';
}
int main() {
  std::cout << "structure,n,q,run,build_ms,query_ms,checksum\n";
  for (int n : {65536, 65537, 262144}) {
    std::mt19937_64 rng(20260918);
    std::vector<Value> a(n); for (auto& x : a) x = rng() % 100000;
    std::vector<std::pair<int,int>> queries;
    for (int i = 0; i < 500000; ++i) {
      int l = rng() % n, r = rng() % n;
      if (l > r) std::swap(l,r);
      queries.emplace_back(l,r+1);
    }
    for (int run = 0; run < 5; ++run) {
      auto sqrt = [&] { measure("sqrt",n,run,[&] { return blueberry::SqrtTree<Value,sum,zero>(a); },queries); };
      auto dst = [&] { measure("dst",n,run,[&] { return blueberry::DisjointSparseTable(a,sum); },queries); };
      if (run % 2) { dst(); sqrt(); } else { sqrt(); dst(); }
    }
  }
}

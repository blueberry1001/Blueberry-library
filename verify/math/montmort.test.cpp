#define PROBLEM "https://judge.yosupo.jp/problem/montmort_number_mod"
#include <iostream>
#include "blueberry/math/montmort.hpp"
// Verify the complete coefficient/sequence output against the official checker.
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n;std::uint32_t mod;std::cin>>n>>mod;auto a=blueberry::montmort(n,mod);a.erase(a.begin());
  for(std::size_t i=0;i<a.size();++i) std::cout<<(i ? " " : "")<<a[i];
  std::cout<<'\n';
}

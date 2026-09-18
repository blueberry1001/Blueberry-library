#define PROBLEM "https://judge.yosupo.jp/problem/stirling_number_of_the_second_kind"
#include <iostream>
#include "blueberry/math/stirling-second.hpp"
#include <atcoder/modint>
using Mint = atcoder::modint998244353;
// Verify the complete coefficient/sequence output against the official checker.
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n;std::cin>>n;auto a=blueberry::stirling_second<Mint>(n);
  for(std::size_t i=0;i<a.size();++i) std::cout<<(i ? " " : "")<<a[i].val();
  std::cout<<'\n';
}

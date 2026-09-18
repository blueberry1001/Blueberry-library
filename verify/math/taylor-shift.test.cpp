#define PROBLEM "https://judge.yosupo.jp/problem/polynomial_taylor_shift"
#include <iostream>
#include "blueberry/math/taylor-shift.hpp"
#include <atcoder/modint>
using Mint = atcoder::modint998244353;
// Verify the complete coefficient/sequence output against the official checker.
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n,c; std::cin>>n>>c; std::vector<Mint> f(n); for(auto& x:f){int v;std::cin>>v;x=v;} auto a=blueberry::taylor_shift(f,Mint(c));
  for(std::size_t i=0;i<a.size();++i) std::cout<<(i ? " " : "")<<a[i].val();
  std::cout<<'\n';
}

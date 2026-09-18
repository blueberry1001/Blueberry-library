#define PROBLEM "https://judge.yosupo.jp/problem/product_of_polynomial_sequence"
#include <iostream>
#include "blueberry/math/polynomial-product.hpp"
#include <atcoder/modint>
using Mint = atcoder::modint998244353;
// Verify the complete coefficient/sequence output against the official checker.
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n; std::cin>>n; std::vector<std::vector<Mint>> f(n); for(auto& p:f){int d;std::cin>>d;p.resize(d+1);for(auto& x:p){int v;std::cin>>v;x=v;}} auto a=blueberry::polynomial_product(std::move(f));
  for(std::size_t i=0;i<a.size();++i) std::cout<<(i ? " " : "")<<a[i].val();
  std::cout<<'\n';
}

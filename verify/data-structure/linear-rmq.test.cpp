#define PROBLEM "https://judge.yosupo.jp/problem/staticrmq"
#include <iostream>
#include <vector>
#include "blueberry/data-structure/linear-rmq.hpp"
int main() {
  std::ios::sync_with_stdio(false); std::cin.tie(nullptr);
  int n,q; std::cin>>n>>q;
  std::vector<int> a(n); for(auto& x:a) std::cin>>x;
  blueberry::LinearRMQ<int> rmq(a);
  while(q--){int l,r;std::cin>>l>>r;std::cout<<rmq.prod(l,r)<<'\n';}
}

#define PROBLEM "https://judge.yosupo.jp/problem/unionfind_with_potential"
#include <iostream>
#include <atcoder/modint>
#include "blueberry/data-structure/potential-union-find.hpp"
using namespace std;
using Mint = atcoder::modint998244353;
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int n,q; cin>>n>>q;
  blueberry::PotentialUnionFind<Mint> uf(n);
  while(q--) {
    int t,u,v; cin>>t>>u>>v;
    if(t==0) { int x; cin>>x; cout<<uf.merge(u,v,Mint(x))<<'\n'; }
    else { auto x=uf.diff(u,v); cout<<(x?static_cast<int>(x->val()):-1)<<'\n'; }
  }
}

#define PROBLEM "https://judge.yosupo.jp/problem/point_set_range_composite"
#include <array>
#include <iostream>
#include <vector>
#include <atcoder/modint>
#include "blueberry/data-structure/persistent-segment-tree.hpp"
using namespace std;
using Mint = atcoder::modint998244353;
using Affine = array<Mint,2>;
struct Compose {
  Affine operator()(const Affine& f,const Affine& g) const { return {g[0]*f[0],g[0]*f[1]+g[1]}; }
};
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int n,q; cin>>n>>q;
  vector<Affine> a(n);
  for(auto& f:a) { int x,y; cin>>x>>y; f={x,y}; }
  blueberry::PersistentSegmentTree tree(a,Compose{},Affine{1,0});
  int version=0;
  while(q--) {
    int t; cin>>t;
    if(t==0) { int p,x,y; cin>>p>>x>>y; version=tree.set(version,p,Affine{x,y}); }
    else { int l,r,x; cin>>l>>r>>x; auto f=tree.prod(version,l,r); cout<<(f[0]*x+f[1]).val()<<'\n'; }
  }
}

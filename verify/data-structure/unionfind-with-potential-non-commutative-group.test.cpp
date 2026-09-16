#define PROBLEM "https://judge.yosupo.jp/problem/unionfind_with_potential_non_commutative_group"
#include <array>
#include <iostream>
#include <atcoder/modint>
#include "blueberry/data-structure/potential-union-find.hpp"
using namespace std;
using Mint = atcoder::modint998244353;
using Matrix = array<Mint,4>;
struct Multiply {
  Matrix operator()(const Matrix& a,const Matrix& b) const {
    return {a[0]*b[0]+a[1]*b[2],a[0]*b[1]+a[1]*b[3],
            a[2]*b[0]+a[3]*b[2],a[2]*b[1]+a[3]*b[3]};
  }
};
struct Inverse {
  // The official input guarantees determinant one.
  Matrix operator()(const Matrix& a) const { return {a[3],-a[1],-a[2],a[0]}; }
};
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int n,q; cin>>n>>q;
  blueberry::PotentialUnionFind<Matrix,Multiply,Inverse> uf(n,Multiply{},Inverse{},Matrix{1,0,0,1});
  while(q--) {
    int t,u,v; cin>>t>>u>>v;
    if(t==0) {
      Matrix x; for(auto& a:x) { int b; cin>>b; a=b; }
      cout<<uf.merge(u,v,x)<<'\n';
    } else {
      auto x=uf.diff(u,v);
      if(!x) cout<<-1<<'\n';
      else cout<<(*x)[0].val()<<' '<<(*x)[1].val()<<' '<<(*x)[2].val()<<' '<<(*x)[3].val()<<'\n';
    }
  }
}

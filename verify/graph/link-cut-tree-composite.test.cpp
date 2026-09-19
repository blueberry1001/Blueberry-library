#define PROBLEM "https://judge.yosupo.jp/problem/dynamic_tree_vertex_set_path_composite"
#include "blueberry/utility/fast-io.hpp"
#include <vector>
#include <atcoder/modint>
#include "blueberry/graph/link-cut-tree.hpp"
using Mint=atcoder::modint998244353;
struct Affine{Mint a,b;};
Affine op(Affine f,Affine g){return {g.a*f.a,g.a*f.b+g.b};}
Affine e(){return {1,0};}
int main(){
  blueberry::FastInput in; blueberry::FastOutput out;
  int n,q;in.read(n,q);std::vector<Affine>a(n);
  for(auto&f:a){int x,y;in.read(x,y);f={x,y};}
  blueberry::LinkCutTree<Affine,op,e> t(a);
  for(int i=1;i<n;++i){int u,v;in.read(u,v);t.link(u,v);}
  while(q--){int type,u,v;in.read(type,u,v);
    if(type==0){int w,x;in.read(w,x);t.cut(u,v);t.link(w,x);}
    if(type==1){int d;in.read(d);t.set(u,{v,d});}
    if(type==2){int x;in.read(x);auto f=t.prod(u,v);out.writeln((f.a*x+f.b).val());}
  }
}

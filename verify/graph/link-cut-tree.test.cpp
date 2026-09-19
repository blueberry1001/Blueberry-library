#define PROBLEM "https://judge.yosupo.jp/problem/dynamic_tree_vertex_add_path_sum"
#include "blueberry/utility/fast-io.hpp"
#include <vector>
#include "blueberry/graph/link-cut-tree.hpp"
long long op(long long a,long long b){return a+b;}
long long e(){return 0;}
int main(){
  blueberry::FastInput in; blueberry::FastOutput out;
  int n,q;in.read(n,q);std::vector<long long>a(n);for(auto&x:a)in.read(x);
  blueberry::LinkCutTree<long long,op,e> t(a);
  for(int i=1;i<n;++i){int u,v;in.read(u,v);t.link(u,v);}
  while(q--){int type,u,v;in.read(type,u,v);
    if(type==0){int w,x;in.read(w,x);t.cut(u,v);t.link(w,x);}
    if(type==1)t.set(u,t.get(u)+v);
    if(type==2)out.writeln(t.prod(u,v));
  }
}

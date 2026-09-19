#pragma once
#include <algorithm>
#include <cassert>
#include <limits>
#include <map>
#include <utility>
#include <vector>

namespace blueberry {
// A cyclic Euler tour is stored in a splay sequence. Vertex entries carry values;
// two directed entries per edge carry the identity and are recycled after cut.
template<class S, auto op, auto e, class F, auto mapping, auto composition, auto id>
class EulerTourTree {
  struct Node {
    int child[2]{-1,-1}, parent=-1, count=0;
    bool vertex=false, pending=false;
    S value=e(), product=e();
    F lazy=id();
  };
  int n_;
  std::vector<Node> nodes_;
  std::vector<int> free_, stack_;
  std::map<std::pair<int,int>,std::pair<int,int>> edges_;
  void check(int v) const { assert(0<=v && v<n_); (void)v; }
  int count(int v) const { return v==-1?0:nodes_[v].count; }
  S product(int v) const { return v==-1?e():nodes_[v].product; }
  void pull(int v) {
    auto& a=nodes_[v];
    a.count=count(a.child[0])+int(a.vertex)+count(a.child[1]);
    a.product=op(op(product(a.child[0]),a.value),product(a.child[1]));
  }
  void apply_node(int v,const F& f) {
    if(v==-1)return;
    auto& a=nodes_[v];
    if(a.vertex)a.value=mapping(f,a.value,1);
    a.product=mapping(f,a.product,a.count);
    a.lazy=a.pending?composition(f,a.lazy):f; a.pending=true;
  }
  void push(int v) {
    auto& a=nodes_[v];if(!a.pending)return;
    apply_node(a.child[0],a.lazy);apply_node(a.child[1],a.lazy);
    a.lazy=id();a.pending=false;
  }
  void rotate(int v) {
    const int p=nodes_[v].parent,g=nodes_[p].parent,d=nodes_[p].child[1]==v;
    const int middle=nodes_[v].child[d^1];
    if(g!=-1)nodes_[g].child[nodes_[g].child[1]==p]=v;
    nodes_[v].parent=g;nodes_[v].child[d^1]=p;nodes_[p].parent=v;
    nodes_[p].child[d]=middle;if(middle!=-1)nodes_[middle].parent=p;
    pull(p);pull(v);
  }
  void splay(int v) {
    stack_.clear();for(int x=v;x!=-1;x=nodes_[x].parent)stack_.push_back(x);
    for(auto it=stack_.rbegin();it!=stack_.rend();++it)push(*it);
    while(nodes_[v].parent!=-1){
      int p=nodes_[v].parent,g=nodes_[p].parent;
      if(g!=-1){
        if((nodes_[p].child[1]==v)==(nodes_[g].child[1]==p))rotate(p);
        else rotate(v);
      }
      rotate(v);
    }
  }
  int join(int a,int b) {
    if(a==-1)return b;
    if(b==-1)return a;
    int x=a;while(nodes_[x].child[1]!=-1)x=nodes_[x].child[1];
    splay(x);nodes_[x].child[1]=b;nodes_[b].parent=x;pull(x);return x;
  }
  int reroot(int v) {
    splay(v);int a=nodes_[v].child[0];nodes_[v].child[0]=-1;
    if(a!=-1)nodes_[a].parent=-1;
    pull(v);return join(v,a);
  }
  int edge_node() {
    if(free_.empty()){nodes_.emplace_back();return static_cast<int>(nodes_.size())-1;}
    int x=free_.back();free_.pop_back();nodes_[x]=Node{};return x;
  }
 public:
  explicit EulerTourTree(const std::vector<S>& values={}) : n_(static_cast<int>(values.size())) {
    assert(values.size()<=static_cast<std::size_t>(std::numeric_limits<int>::max()/3));
    nodes_.reserve(values.size()*3);nodes_.resize(values.size());
    for(int v=0;v<n_;++v){nodes_[v].vertex=true;nodes_[v].count=1;nodes_[v].value=nodes_[v].product=values[v];}
  }
  int size() const { return n_; }
  bool same(int u,int v) {
    check(u);check(v);if(u==v)return true;splay(u);splay(v);return nodes_[u].parent!=-1;
  }
  bool link(int u,int v) {
    check(u);check(v);if(same(u,v))return false;
    int a=edge_node(),b=edge_node();edges_[std::minmax(u,v)]={a,b};
    int left=reroot(u),right=reroot(v);join(join(join(left,a),right),b);return true;
  }
  bool cut(int u,int v) {
    check(u);check(v);auto it=edges_.find(std::minmax(u,v));if(it==edges_.end())return false;
    auto [a,b]=it->second;reroot(a);splay(a);
    int rest=nodes_[a].child[1];nodes_[rest].parent=-1;nodes_[a].child[1]=-1;
    splay(b);for(int x:nodes_[b].child)if(x!=-1)nodes_[x].parent=-1;
    nodes_[b].child[0]=nodes_[b].child[1]=-1;
    free_.push_back(a);free_.push_back(b);edges_.erase(it);return true;
  }
  S get(int v) { check(v);splay(v);return nodes_[v].value; }
  void set(int v,const S& x) { check(v);splay(v);nodes_[v].value=x;pull(v); }
  S prod(int v,int parent=-1) {
    check(v);if(parent!=-1){bool removed=cut(v,parent);assert(removed);(void)removed;}
    splay(v);S result=nodes_[v].product;
    if(parent!=-1)link(v,parent);
    return result;
  }
  void apply(int v,int parent,const F& f) {
    check(v);if(parent!=-1){bool removed=cut(v,parent);assert(removed);(void)removed;}
    splay(v);apply_node(v,f);if(parent!=-1)link(v,parent);
  }
};
} // namespace blueberry

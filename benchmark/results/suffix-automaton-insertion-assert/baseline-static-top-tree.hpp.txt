#pragma once
#include <algorithm>
#include <cassert>
#include <functional>
#include <limits>
#include <vector>

namespace blueberry {
// A weighted-balanced rake/compress expression for a fixed rooted tree.
template<class V,class Path,class Point,auto add_vertex,auto add_edge,
         auto compress,auto rake,auto point_identity>
class StaticTopTree {
  struct Node {
    int kind,left=-1,right=-1,parent=-1,vertex=-1;
    Path path{};
    Point point{};
  };
  std::vector<V> values_;
  std::vector<Node> nodes_;
  std::vector<int> vertex_node_;
  int root_=-1;
  void pull(int i) {
    auto& n=nodes_[i];
    if(n.kind==0)n.path=add_vertex(values_[n.vertex],n.left==-1?point_identity():nodes_[n.left].point);
    if(n.kind==1)n.point=add_edge(nodes_[n.left].path);
    if(n.kind==2)n.path=compress(nodes_[n.left].path,nodes_[n.right].path);
    if(n.kind==3)n.point=rake(nodes_[n.left].point,nodes_[n.right].point);
  }
  int node(int kind,int left=-1,int right=-1,int vertex=-1) {
    int i=static_cast<int>(nodes_.size());nodes_.push_back(Node{kind,left,right,-1,vertex,{},{}});
    if(left!=-1)nodes_[left].parent=i;
    if(right!=-1)nodes_[right].parent=i;
    pull(i);return i;
  }
  int balance(const std::vector<int>& items,const std::vector<int>& weights,int kind) {
    if(items.empty())return -1;
    std::vector<int> prefix(weights.size()+1);
    for(std::size_t i=0;i<weights.size();++i)prefix[i+1]=prefix[i]+weights[i];
    // Isolate the weighted median: each recursive side has <= half the weight.
    auto build=[&](auto&& self,int l,int r)->int {
      if(l==r)return -1;
      int target=prefix[l]+(prefix[r]-prefix[l])/2;
      int m=static_cast<int>(std::upper_bound(prefix.begin()+l,prefix.begin()+r,target)-prefix.begin())-1;
      int a=self(self,l,m),b=items[m],c=self(self,m+1,r);
      if(a!=-1)b=node(kind,a,b);
      if(c!=-1)b=node(kind,b,c);
      return b;
    };
    return build(build,0,static_cast<int>(items.size()));
  }
 public:
  StaticTopTree(const std::vector<std::vector<int>>& tree,const std::vector<V>& values,int root=0)
      : values_(values),vertex_node_(values.size(),-1) {
    const int n=size();assert(n>0 && tree.size()==values.size());
    assert(values.size()<=static_cast<std::size_t>(std::numeric_limits<int>::max()/4));
    assert(0<=root && root<n);
    std::vector<int> parent(n,-1),order{root},sub(n,1),heavy(n,-1);
    parent[root]=root;
    for(std::size_t i=0;i<order.size();++i){
      int v=order[i];for(int w:tree[v]){
        assert(0<=w && w<n);if(w==parent[v])continue;
        assert(parent[w]==-1);parent[w]=v;order.push_back(w);
      }
    }
    assert(static_cast<int>(order.size())==n);
    for(auto it=order.rbegin();it!=order.rend();++it){
      int v=*it;for(int w:tree[v])if(parent[w]==v && w!=v){
        sub[v]+=sub[w];if(heavy[v]==-1 || sub[w]>sub[heavy[v]])heavy[v]=w;
      }
    }
    nodes_.reserve(values.size()*4);
    auto path=[&](auto&& self,int head)->int {
      std::vector<int> items,weights;
      for(int v=head;v!=-1;v=heavy[v]){
        std::vector<int> children,child_weights;
        for(int w:tree[v])if(parent[w]==v && w!=v && w!=heavy[v]){
          children.push_back(node(1,self(self,w)));child_weights.push_back(sub[w]);
        }
        int r=balance(children,child_weights,3);
        int leaf=node(0,r,-1,v);vertex_node_[v]=leaf;items.push_back(leaf);
        weights.push_back(sub[v]-(heavy[v]==-1?0:sub[heavy[v]]));
      }
      return balance(items,weights,2);
    };
    root_=path(path,root);
  }
  int size() const { return static_cast<int>(values_.size()); }
  V get(int v) const { assert(0<=v && v<size());return values_[v]; }
  void set(int v,const V& x) {
    assert(0<=v && v<size());values_[v]=x;
    for(int i=vertex_node_[v];i!=-1;i=nodes_[i].parent)pull(i);
  }
  Path all_prod() const { return nodes_[root_].path; }
};
} // namespace blueberry

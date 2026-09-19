#pragma once
#include <algorithm>
#include <cassert>
#include <limits>
#include <vector>

namespace blueberry {
// Dynamic rake/compress clusters with independent light-cluster splay trees.
// Conservative bound: amortized O(log^2 N), O(N) owned storage.
template<class V,class Path,class Point,auto add_vertex,auto add_edge,
         auto compress,auto rake,auto point_identity>
class DynamicTopTree {
  struct Node {
    int child[2]{-1, -1}, parent = -1;
    bool reversed = false;
    int light=-1, handle=-1;
    V value;
    Path forward, backward;
    explicit Node(const V& x) : value(x), forward(add_vertex(x,point_identity())), backward(forward) {}
  };
  std::vector<Node> nodes_;
  std::vector<int> stack_;
  void check(int v) const { assert(0 <= v && v < size()); (void)v; }
  bool auxiliary_root(int v) const {
    const int p = nodes_[v].parent;
    return p == -1 || (nodes_[p].child[0] != v && nodes_[p].child[1] != v);
  }
  struct Light {
    int child[2]{-1,-1},parent=-1;
    Point value=point_identity(),product=point_identity();
  };
  std::vector<Light> lights_;
  std::vector<int> free_;
  Point light_product(int v) const { return v==-1?point_identity():lights_[v].product; }
  void light_pull(int v) {
    auto& a=lights_[v];a.product=rake(rake(light_product(a.child[0]),a.value),light_product(a.child[1]));
  }
  void light_rotate(int v) {
    int p=lights_[v].parent,g=lights_[p].parent,d=lights_[p].child[1]==v;
    int middle=lights_[v].child[d^1];
    if(g!=-1)lights_[g].child[lights_[g].child[1]==p]=v;
    lights_[v].parent=g;lights_[v].child[d^1]=p;lights_[p].parent=v;
    lights_[p].child[d]=middle;if(middle!=-1)lights_[middle].parent=p;
    light_pull(p);light_pull(v);
  }
  void light_splay(int v) {
    while(lights_[v].parent!=-1){
      int p=lights_[v].parent,g=lights_[p].parent;
      if(g!=-1){
        if((lights_[p].child[1]==v)==(lights_[g].child[1]==p))light_rotate(p);
        else light_rotate(v);
      }
      light_rotate(v);
    }
  }
  int light_insert(int root,const Point& value) {
    int v;
    if(free_.empty()){v=static_cast<int>(lights_.size());lights_.emplace_back();}
    else{v=free_.back();free_.pop_back();lights_[v]=Light{};}
    lights_[v].value=lights_[v].product=value;
    if(root==-1)return v;
    int x=root;while(lights_[x].child[1]!=-1)x=lights_[x].child[1];
    light_splay(x);lights_[x].child[1]=v;lights_[v].parent=x;light_pull(x);light_splay(v);return v;
  }
  int light_erase(int v) {
    assert(v!=-1);light_splay(v);
    int a=lights_[v].child[0],b=lights_[v].child[1];
    if(a!=-1)lights_[a].parent=-1;
    if(b!=-1)lights_[b].parent=-1;
    free_.push_back(v);
    if(a==-1)return b;
    if(b==-1)return a;
    int x=a;while(lights_[x].child[1]!=-1)x=lights_[x].child[1];
    light_splay(x);lights_[x].child[1]=b;lights_[b].parent=x;light_pull(x);return x;
  }
  Path local(int v) const { return add_vertex(nodes_[v].value,light_product(nodes_[v].light)); }
  void pull(int v) {
    auto& a=nodes_[v];a.forward=a.backward=local(v);
    if(a.child[0]!=-1){a.forward=compress(nodes_[a.child[0]].forward,a.forward);a.backward=compress(a.backward,nodes_[a.child[0]].backward);}
    if(a.child[1]!=-1){a.forward=compress(a.forward,nodes_[a.child[1]].forward);a.backward=compress(nodes_[a.child[1]].backward,a.backward);}
  }
  void reverse(int v) {
    if (v == -1) return;
    auto& a = nodes_[v];
    std::swap(a.child[0], a.child[1]); std::swap(a.forward, a.backward);
    a.reversed = !a.reversed;
  }
  void push(int v) {
    if (!nodes_[v].reversed) return;
    reverse(nodes_[v].child[0]); reverse(nodes_[v].child[1]); nodes_[v].reversed = false;
  }
  void rotate(int v) {
    const int p = nodes_[v].parent, g = nodes_[p].parent;
    const int d = nodes_[p].child[1] == v, middle = nodes_[v].child[d ^ 1];
    if (!auxiliary_root(p)) nodes_[g].child[nodes_[g].child[1] == p] = v;
    nodes_[v].parent = g; nodes_[v].child[d ^ 1] = p; nodes_[p].parent = v;
    nodes_[p].child[d] = middle;
    if (middle != -1) nodes_[middle].parent = p;
    pull(p); pull(v);
  }
  void splay(int v) {
    stack_.clear();
    for (int x = v;; x = nodes_[x].parent) {
      stack_.push_back(x); if (auxiliary_root(x)) break;
    }
    const int previous_root=stack_.back();
    if(previous_root!=v){nodes_[v].handle=nodes_[previous_root].handle;nodes_[previous_root].handle=-1;}
    for (auto it = stack_.rbegin(); it != stack_.rend(); ++it) push(*it);
    while (!auxiliary_root(v)) {
      const int p = nodes_[v].parent, g = nodes_[p].parent;
      if (!auxiliary_root(p)) {
        if ((nodes_[p].child[1] == v) == (nodes_[g].child[1] == p)) rotate(p);
        else rotate(v);
      }
      rotate(v);
    }
  }
  void access(int v) {
    int last = -1;
    for (int x = v; x != -1; x = nodes_[x].parent) {
      splay(x);
      int old=nodes_[x].child[1];
      if(old!=-1){nodes_[x].light=light_insert(nodes_[x].light,add_edge(nodes_[old].forward));nodes_[old].handle=nodes_[x].light;}
      if(last!=-1){nodes_[x].light=light_erase(nodes_[last].handle);nodes_[last].handle=-1;}
      nodes_[x].child[1]=last;pull(x);last=x;
    }
    splay(v);
  }
  void make_root(int v) { access(v); reverse(v); }
  int root(int v) {
    access(v); push(v);
    while (nodes_[v].child[0] != -1) { v = nodes_[v].child[0]; push(v); }
    splay(v); return v;
  }
 public:
  explicit DynamicTopTree(const std::vector<V>& values = {}) {
    assert(values.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max()));
    nodes_.reserve(values.size());
    lights_.reserve(values.size());
    for (const V& x : values) nodes_.emplace_back(x);
  }
  int size() const { return static_cast<int>(nodes_.size()); }
  void evert(int v) { check(v); make_root(v); }
  int leader(int v) { check(v); return root(v); }
  bool same(int u, int v) { check(u); check(v); return u == v || root(u) == root(v); }
  bool link(int u, int v) {
    check(u); check(v); make_root(u);
    if (root(v) == u) return false;
    access(v);nodes_[v].child[1]=u;nodes_[u].parent=v;pull(v);return true;
  }
  bool cut(int u, int v) {
    check(u); check(v); make_root(u); access(v);
    if (nodes_[v].child[0] != u || nodes_[u].child[1] != -1) return false;
    nodes_[v].child[0] = -1; nodes_[u].parent = -1; pull(v); return true;
  }
  V get(int v) const { check(v); return nodes_[v].value; }
  void set(int v, const V& x) { check(v); access(v); nodes_[v].value = x; pull(v); }
  Path prod(int u, int v) {
    check(u); check(v); assert(same(u, v)); make_root(u); access(v);
    return nodes_[v].forward;
  }
  Path all_prod(int v) { check(v);make_root(v);access(v);return nodes_[v].forward; }
  Path subtree_prod(int v,int parent=-1) {
    check(v);
    if(parent==-1)return all_prod(v);
    bool removed=cut(v,parent);assert(removed);(void)removed;
    Path result=all_prod(v);link(v,parent);return result;
  }
};
}  // namespace blueberry

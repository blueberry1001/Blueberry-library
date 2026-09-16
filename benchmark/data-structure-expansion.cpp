// Independently written alternatives; no third-party implementation is embedded.
// Compare identical operation streams, group operations and treap priorities.
#include "blueberry/data-structure/ordered-set.hpp"
#include "blueberry/data-structure/persistent-segment-tree.hpp"
#include "blueberry/data-structure/potential-union-find.hpp"
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <optional>
#include <random>
#include <string>
#include <vector>

template<class T,class Op,class Inv>
class PotentialAoS {
  struct Entry { int parent; T potential; };
  std::vector<Entry> a;
  Op op;
  Inv inv;
  int root(int v) {
    if(a[v].parent<0) return v;
    int p=a[v].parent;
    int r=root(p);
    a[v].potential=op(a[p].potential,a[v].potential);
    return a[v].parent=r;
  }
 public:
  PotentialAoS(int n,Op f,Inv g,T one):a(n,Entry{-1,one}),op(f),inv(g) {}
  bool merge(int u,int v,T w) {
    int x=root(u),y=root(v);
    if(x==y) return a[u].potential==op(a[v].potential,w);
    T edge=op(op(a[v].potential,w),inv(a[u].potential));
    if(a[x].parent<a[y].parent) { std::swap(x,y); edge=inv(edge); }
    a[y].parent+=a[x].parent; a[x].parent=y; a[x].potential=edge;
    return true;
  }
  std::optional<T> diff(int u,int v) {
    if(root(u)!=root(v)) return std::nullopt;
    return op(inv(a[v].potential),a[u].potential);
  }
};

class PointerSet {
  struct Node { int key; std::uint64_t priority; Node* l=nullptr; Node* r=nullptr; int count=1; };
  Node* top=nullptr;
  std::uint64_t state=0x243f6a8885a308d3ULL;
  static int count(Node* p) { return p?p->count:0; }
  static void pull(Node* p) { p->count=1+count(p->l)+count(p->r); }
  std::uint64_t random() {
    auto z=(state+=0x9e3779b97f4a7c15ULL);
    z=(z^(z>>30))*0xbf58476d1ce4e5b9ULL;
    z=(z^(z>>27))*0x94d049bb133111ebULL;
    return z^(z>>31);
  }
  static Node* rotate(Node* p,bool right) {
    Node* q=right?p->l:p->r;
    if(right) { p->l=q->r; q->r=p; } else { p->r=q->l; q->l=p; }
    pull(p); pull(q); return q;
  }
  Node* add(Node* p,int x) {
    if(!p) return new Node{x,random()};
    if(x<p->key) { p->l=add(p->l,x); if(p->l->priority<p->priority) p=rotate(p,true); }
    else if(x>p->key) { p->r=add(p->r,x); if(p->r->priority<p->priority) p=rotate(p,false); }
    pull(p); return p;
  }
  static Node* join(Node* p,Node* q) {
    if(!p) return q;
    if(!q) return p;
    if(p->priority<q->priority) { p->r=join(p->r,q); pull(p); return p; }
    q->l=join(p,q->l); pull(q); return q;
  }
  static Node* remove(Node* p,int x) {
    if(!p) return p;
    if(x==p->key) { Node* q=join(p->l,p->r); delete p; return q; }
    if(x<p->key) p->l=remove(p->l,x); else p->r=remove(p->r,x);
    pull(p); return p;
  }
  static void destroy(Node* p) { if(p) { destroy(p->l); destroy(p->r); delete p; } }
 public:
  ~PointerSet() { destroy(top); }
  void insert(int x) { top=add(top,x); }
  void erase(int x) { top=remove(top,x); }
  int rank(int x) const {
    int result=0;
    for(Node* p=top;p;) {
      if(p->key<x) { result+=count(p->l)+1; p=p->r; } else p=p->l;
    }
    return result;
  }
};

class PointerPersistent {
  struct Node { long long value; Node* left; Node* right; };
  int n;
  std::vector<Node*> owned;
  std::vector<Node*> roots{nullptr};
  static long long value(Node* p) { return p?p->value:0; }
  Node* change(Node* p,int l,int r,int pos,long long x) {
    Node* result;
    if(r-l==1) result=new Node{x,nullptr,nullptr};
    else {
      int m=l+(r-l)/2;
      Node* left=p?p->left:nullptr; Node* right=p?p->right:nullptr;
      if(pos<m) left=change(left,l,m,pos,x); else right=change(right,m,r,pos,x);
      result=new Node{value(left)+value(right),left,right};
    }
    owned.push_back(result); return result;
  }
  static long long query(Node* p,int l,int r,int a,int b) {
    if(!p||b<=l||r<=a) return 0;
    if(a<=l&&r<=b) return p->value;
    int m=l+(r-l)/2;
    return query(p->left,l,m,a,b)+query(p->right,m,r,a,b);
  }
 public:
  explicit PointerPersistent(int length):n(length) {}
  ~PointerPersistent() { for(auto p:owned) delete p; }
  int set(int v,int p,long long x) { roots.push_back(change(roots[v],0,n,p,x)); return static_cast<int>(roots.size())-1; }
  long long prod(int v,int l,int r) const { return query(roots[v],0,n,l,r); }
};

struct Matrix {
  std::array<long long,4> a;
  bool operator==(const Matrix&) const=default;
};
struct Multiply {
  Matrix operator()(const Matrix& x,const Matrix& y) const {
    const auto& a=x.a; const auto& b=y.a;
    return {{{(a[0]*b[0]+a[1]*b[2])%998244353,(a[0]*b[1]+a[1]*b[3])%998244353,
              (a[2]*b[0]+a[3]*b[2])%998244353,(a[2]*b[1]+a[3]*b[3])%998244353}}};
  }
};
struct Inverse {
  Matrix operator()(const Matrix& x) const { return {{{x.a[3],(998244353-x.a[1])%998244353,(998244353-x.a[2])%998244353,x.a[0]}}}; }
};
std::uint64_t hash_value(long long x) { return static_cast<std::uint64_t>(x); }
std::uint64_t hash_value(const Matrix& x) { return static_cast<std::uint64_t>(x.a[0]); }

template<class UF,class T>
std::uint64_t uf_work(UF& uf,const std::vector<T>& delta,const std::vector<int>& vertices) {
  std::uint64_t checksum=0;
  int n=static_cast<int>(delta.size());
  for(int i=1;i<n;++i) checksum+=uf.merge(i,vertices[i]%i,delta[i]);
  for(int i=0;i<n;++i) checksum+=hash_value(*uf.diff(vertices[i],i));
  return checksum;
}
template<class Set>
std::uint64_t set_work(const std::vector<int>& keys) {
  Set s;
  for(int x:keys) s.insert(x);
  std::uint64_t checksum=0;
  for(int x:keys) { checksum+=s.rank(x); s.erase(x); s.insert(x+1); }
  return checksum;
}
template<class Tree>
std::uint64_t persistent_work(Tree& tree,const std::vector<int>& positions,bool branch) {
  std::uint64_t checksum=0;
  int n=static_cast<int>(positions.size());
  for(int i=0;i<n;++i) {
    int version=branch?positions[i]%(i+1):i;
    int next=tree.set(version,positions[i],i+1);
    checksum+=tree.prod(next,0,positions[i]+1);
  }
  return checksum;
}

int main(int argc,char** argv) {
  if(argc!=4) return 2;
  std::string workload=argv[1],candidate=argv[2];
  int n=std::atoi(argv[3]);
  if(n<=0) return 2;
  std::mt19937_64 rng(20260917);
  std::vector<int> positions(n),keys(n);
  std::vector<long long> delta(n);
  std::vector<Matrix> matrices(n);
  for(int i=0;i<n;++i) {
    positions[i]=static_cast<int>(rng()%n);
    keys[i]=workload=="set-monotone"?i*2:static_cast<int>(rng()%(4*n));
    delta[i]=static_cast<long long>(rng()%10000);
    long long x=static_cast<long long>(rng()%100),y=static_cast<long long>(rng()%100);
    matrices[i]={{{1+x*y,x,y,1}}};
  }
  auto start=std::chrono::steady_clock::now();
  std::uint64_t checksum=0;
  if(workload=="uf-add") {
    if(candidate=="pool") {
      blueberry::PotentialUnionFind<long long> uf(n); checksum=uf_work(uf,delta,positions);
    } else {
      PotentialAoS<long long,std::plus<long long>,std::negate<long long>> uf(n,{}, {},0);
      checksum=uf_work(uf,delta,positions);
    }
  } else if(workload=="uf-matrix") {
    Matrix one{{{1,0,0,1}}};
    if(candidate=="pool") {
      blueberry::PotentialUnionFind<Matrix,Multiply,Inverse> uf(n,{}, {},one);
      checksum=uf_work(uf,matrices,positions);
    } else {
      PotentialAoS<Matrix,Multiply,Inverse> uf(n,{}, {},one);
      checksum=uf_work(uf,matrices,positions);
    }
  } else if(workload.starts_with("set-")) {
    checksum=candidate=="pool"?set_work<blueberry::OrderedSet<int>>(keys):set_work<PointerSet>(keys);
  } else if(workload.starts_with("persistent-")) {
    bool branch=workload=="persistent-branch";
    if(candidate=="pool") {
      blueberry::PersistentSegmentTree tree(n,std::plus<long long>{},0LL);
      checksum=persistent_work(tree,positions,branch);
    } else {
      PointerPersistent tree(n); checksum=persistent_work(tree,positions,branch);
    }
  } else return 2;
  std::cout<<workload<<','<<candidate<<','<<n<<','
           <<std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count()
           <<','<<checksum<<'\n';
}

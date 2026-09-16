#include "blueberry/data-structure/ordered-set.hpp"
#include "blueberry/data-structure/persistent-segment-tree.hpp"
#include "blueberry/data-structure/potential-union-find.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <optional>
#include <queue>
#include <random>
#include <set>
#include <string>
#include <utility>
#include <vector>

// Small noncommutative group: determinant-one matrices over F_101.
using Mat = std::array<int, 4>;
constexpr Mat kIdentity{1, 0, 0, 1};
struct Mul {
  Mat operator()(const Mat& a, const Mat& b) const {
    return {(a[0]*b[0]+a[1]*b[2])%101, (a[0]*b[1]+a[1]*b[3])%101,
            (a[2]*b[0]+a[3]*b[2])%101, (a[2]*b[1]+a[3]*b[3])%101};
  }
};
struct Inv {
  Mat operator()(const Mat& a) const { return {a[3], (101-a[1])%101, (101-a[2])%101, a[0]}; }
};

void potential_test(std::mt19937_64& rng) {
  blueberry::PotentialUnionFind<long long> empty(0);
  assert(empty.size() == 0);
  blueberry::PotentialUnionFind<long long> add(4);
  assert(add.merge(0,1,7) && add.merge(2,1,-3));
  assert(add.diff(0,2) == 10 && add.diff(2,0) == -10);
  assert(!add.merge(0,2,11) && add.diff(0,2) == 10);
  assert(!add.diff(0,3) && add.comp_size(1) == 3);
  for (int trial=0; trial<50; ++trial) {
    const int n=1+static_cast<int>(rng()%25);
    blueberry::PotentialUnionFind<Mat,Mul,Inv> uf(n,Mul{},Inv{},kIdentity);
    std::vector<std::vector<std::pair<int,Mat>>> g(n);
    // BFS assigns potentials from scratch, independently of union/find logic.
    auto oracle = [&](int u,int v)->std::optional<Mat> {
      std::vector<std::optional<Mat>> p(n);
      std::queue<int> q;
      p[v]=kIdentity; q.push(v);
      while (!q.empty()) {
        int x=q.front(); q.pop();
        for (const auto& [y,w]:g[x]) if (!p[y]) {
          p[y]=Mul{}(*p[x],w); q.push(y);
        }
      }
      return p[u];
    };
    for (int step=0; step<250; ++step) {
      int u=static_cast<int>(rng()%n), v=static_cast<int>(rng()%n);
      Mat w=Mul{}(Mat{1,static_cast<int>(rng()%101),0,1},
                  Mat{1,0,static_cast<int>(rng()%101),1});
      auto expected=oracle(u,v);
      bool ok=!expected || *expected==w;
      assert(uf.merge(u,v,w)==ok);
      if (ok) { g[v].push_back({u,w}); g[u].push_back({v,Inv{}(w)}); }
      for (int k=0;k<3;++k) {
        int a=static_cast<int>(rng()%n), b=static_cast<int>(rng()%n);
        assert(uf.diff(a,b)==oracle(a,b));
        assert(uf.same(a,b)==oracle(a,b).has_value());
      }
      int count=0;
      for (int k=0;k<n;++k) count+=oracle(k,u).has_value();
      assert(uf.comp_size(u)==count && uf.leader(uf.leader(u))==uf.leader(u));
    }
  }
}

template<class Compare>
void ordered_case(std::mt19937_64& rng, Compare comp) {
  blueberry::OrderedSet<long long,Compare> tree(comp);
  std::set<long long,Compare> ref(comp);
  for (int step=0;step<12000;++step) {
    long long x=static_cast<long long>(rng()%700)-350;
    if (step==0) x=std::numeric_limits<long long>::min();
    if (step==1) x=std::numeric_limits<long long>::max();
    if (rng()%3) assert(tree.insert(x)==ref.insert(x).second);
    else assert(tree.erase(x)==(ref.erase(x)!=0));
    assert(tree.contains(x)==ref.contains(x));
    assert(tree.size()==static_cast<int>(ref.size()) && tree.empty()==ref.empty());
    assert(tree.rank(x)==std::distance(ref.begin(),ref.lower_bound(x)));
    auto value=[&](auto it)->std::optional<long long> {
      return it==ref.end()?std::nullopt:std::optional<long long>(*it);
    };
    assert(tree.lower_bound(x)==value(ref.lower_bound(x)));
    assert(tree.upper_bound(x)==value(ref.upper_bound(x)));
    auto hi=ref.upper_bound(x);
    auto floor=hi==ref.begin()?std::nullopt:std::optional<long long>(*std::prev(hi));
    assert(tree.floor(x)==floor);
    int k=static_cast<int>(rng()%(ref.size()+2))-1;
    assert(tree.kth(k)==(k<0||k>=tree.size()?std::nullopt:value(std::next(ref.begin(),k))));
  }
  auto copy=tree;
  for (auto x:ref) assert(tree.erase(x));
  assert(tree.empty() && copy.size()==static_cast<int>(ref.size()));
  tree=copy;
  auto moved=std::move(copy);
  assert(copy.empty() && copy.insert(19));
  assert(moved.size()==tree.size());
  copy=std::move(moved);
  assert(moved.empty() && moved.insert(23));
}

void persistent_test(std::mt19937_64& rng) {
  auto join=[](const std::string& a,const std::string& b){ return a+b; };
  for (int n:{0,1,2,3,7,8,9,31,32,33}) {
    blueberry::PersistentSegmentTree tree(n,join,std::string{});
    std::vector<std::vector<std::string>> versions(1,std::vector<std::string>(n));
    assert(tree.size()==n && tree.versions()==1 && tree.all_prod(0).empty());
    for(int step=0;step<400;++step) {
      int v=static_cast<int>(rng()%versions.size());
      if(n) {
        int p=static_cast<int>(rng()%n);
        std::string x(1,static_cast<char>('a'+rng()%26));
        auto next=versions[v];
        int id;
        if(rng()%2) { next[p]=x; id=tree.set(v,p,x); }
        else { next[p]+=x; id=tree.apply(v,p,x); }
        assert(id==static_cast<int>(versions.size()));
        versions.push_back(next);
      }
      int a=static_cast<int>(rng()%(n+1)),b=static_cast<int>(rng()%(n+1));
      if(a>b) std::swap(a,b);
      std::string expected;
      for(int i=a;i<b;++i) expected+=versions[v][i];
      assert(tree.prod(v,a,b)==expected);
      if(n) assert(tree.get(v,a%n)==versions[v][a%n]);
    }
    auto copy=tree;
    assert(copy.all_prod(0)==tree.all_prod(0));
    if(n) { copy.set(0,0,"independent"); assert(tree.get(0,0).empty()); }
  }
  blueberry::PersistentSegmentTree sums(std::vector<long long>{2,3,5},std::plus<long long>{},0LL);
  assert(sums.all_prod(0)==10 && sums.all_prod(sums.apply(0,1,7))==17);
  blueberry::PersistentSegmentTree sparse(std::numeric_limits<int>::max(),std::plus<int>{},0);
  int v=sparse.set(0,std::numeric_limits<int>::max()-1,8);
  assert(sparse.all_prod(v)==8 && sparse.all_prod(0)==0);
  assert(sparse.get(v,std::numeric_limits<int>::max()-1)==8);
}

int main(int argc,char** argv) {
  const unsigned long long seed=argc>1?std::strtoull(argv[1],nullptr,10):20260917;
  std::cerr<<"seed="<<seed<<'\n';
  std::mt19937_64 rng(seed);
  potential_test(rng);
  ordered_case(rng,std::less<long long>{});
  ordered_case(rng,std::greater<long long>{});
  blueberry::OrderedSet<std::string> words;
  assert(words.insert("pear") && words.insert("apple") && words.kth(0)=="apple");
  // No default constructor or equality/arithmetic operators on Key.
  struct Key { explicit Key(int x):value(x) {} int value; };
  auto comp=[](const Key& a,const Key& b){ return a.value/10<b.value/10; };
  blueberry::OrderedSet<Key,decltype(comp)> keys(comp);
  assert(keys.insert(Key(21)) && !keys.insert(Key(29)));
  assert(keys.kth(0)->value==21 && keys.contains(Key(25)));
  blueberry::OrderedSet<int> increasing;
  for(int i=0;i<20000;++i) assert(increasing.insert(i));
  for(int i=0;i<20000;++i) assert(increasing.kth(i)==i);
  for(int i=0;i<20000;++i) assert(increasing.erase(i));
  assert(increasing.empty());
  persistent_test(rng);
}

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>
#include <atcoder/lazysegtree>
#include <atcoder/segtree>
#include <atcoder/modint>
#include "blueberry/algebra/monoids.hpp"
using namespace std;
using namespace blueberry::monoid;
using Mint = atcoder::modint998244353;
template<class M> using Lazy = atcoder::lazy_segtree<typename M::S, M::op, M::e, typename M::F, M::mapping, M::composition, M::id>;
template<class M> using Seg = atcoder::segtree<typename M::S, M::op, M::e>;
int main(int argc, char** argv) {
  unsigned seed = argc > 1 ? strtoul(argv[1], nullptr, 10) : 1;
  cerr << "monoids seed=" << seed << '\n';
  mt19937 rng(seed);
  {
    using I=IndexAffineSum<long long>;
    Lazy<I> tree(vector<I::S>{I::leaf(2,0),I::leaf(-3,1),I::leaf(5,2),I::leaf(1,3)});
    tree.apply(0,4,{1,-2,7}); // add 7,5,3,1
    tree.apply(1,3,{1,3,-4}); // add -1,2 to the overlap
    assert(tree.prod(0,4).sum==22);
    tree.set(2,I::leaf(-9,2)); assert(tree.all_prod().sum==3);
  }
  for (int trial = 0; trial < 100; ++trial) {
    int n = rng() % 30;
    vector<Mint> a(n);
    using A = AffineSum<Mint>; using I = IndexAffineSum<Mint>; using Q = AffineSumSquares<Mint>;
    for(int k=0;k<30;++k) {
      auto u=A::leaf(rng()%100),v=A::leaf(rng()%100); A::F f{rng()%7,rng()%11},g{rng()%7,rng()%11};
      auto a1=A::mapping(A::composition(f,g),u),a2=A::mapping(f,A::mapping(g,u));assert(a1.sum==a2.sum&&a1.len==a2.len);
      assert(A::mapping(A::id(),u).sum==u.sum);
      assert(A::mapping(f,A::op(u,v)).sum==A::op(A::mapping(f,u),A::mapping(f,v)).sum);
      auto iu=I::leaf(u.sum,k),iv=I::leaf(v.sum,k+1); I::F fi{f.a,f.b,2},gi{g.a,g.b,3};
      assert(I::mapping(I::composition(fi,gi),iu).sum==I::mapping(fi,I::mapping(gi,iu)).sum);
      assert(I::mapping(I::id(),iu).sum==iu.sum);
      assert(I::mapping(fi,I::op(iu,iv)).sum==I::op(I::mapping(fi,iu),I::mapping(fi,iv)).sum);
      auto qu=Q::leaf(u.sum),qv=Q::leaf(v.sum);Q::F fq{f.a,f.b},gq{g.a,g.b};
      assert(Q::mapping(Q::composition(fq,gq),qu).square_sum==Q::mapping(fq,Q::mapping(gq,qu)).square_sum);
      assert(Q::mapping(Q::id(),qu).square_sum==qu.square_sum);
      assert(Q::mapping(fq,Q::op(qu,qv)).square_sum==Q::op(Q::mapping(fq,qu),Q::mapping(fq,qv)).square_sum);
      using B=BinaryFlipInversions;auto bu=B::op(B::leaf(true),B::leaf(false));auto bv=B::leaf(k%2);bool bf=k%2,bg=k%3;
      assert(B::mapping(B::composition(bf,bg),bu).inversions==B::mapping(bf,B::mapping(bg,bu)).inversions);
      assert(B::mapping(B::id(),bu).inversions==bu.inversions);
      assert(B::mapping(bf,B::op(bu,bv)).inversions==B::op(B::mapping(bf,bu),B::mapping(bf,bv)).inversions);
    }
    vector<A::S> av; vector<I::S> iv; vector<Q::S> qv;
    for (int i = 0; i < n; ++i) { a[i] = rng() % 100; av.push_back(A::leaf(a[i])); iv.push_back(I::leaf(a[i], i)); qv.push_back(Q::leaf(a[i])); }
    Lazy<A> sa(av); Lazy<I> si(iv); Lazy<Q> sq(qv);
    vector<Mint> indexed = a;
    for (int step = 0; step < 200; ++step) {
      int l = rng() % (n + 1), r = rng() % (n + 1); if (l > r) swap(l,r);
      Mint x = rng() % 4, y = rng() % 11, z = rng() % 7;
      if (step % 7 == 0 && n) {
        int p=rng()%n; a[p]=y; indexed[p]=z; sa.set(p,A::leaf(y)); sq.set(p,Q::leaf(y)); si.set(p,I::leaf(z,p));
      } else if (step % 2) {
        sa.apply(l,r,{x,y}); sq.apply(l,r,{x,y}); si.apply(l,r,{x,z,y});
        for(int i=l;i<r;++i) { a[i]=x*a[i]+y; indexed[i]=x*indexed[i]+z*i+y; }
      } else {
        Mint sum=0, square=0, isum=0;
        for(int i=l;i<r;++i) { sum+=a[i]; square+=a[i]*a[i]; isum+=indexed[i]; }
        assert(sa.prod(l,r).sum==sum); assert(sq.prod(l,r).sum==sum); assert(sq.prod(l,r).square_sum==square); assert(si.prod(l,r).sum==isum);
        assert(sa.prod(l,r).len==r-l); assert(sq.prod(l,r).len==r-l); assert(si.prod(l,r).len==r-l);
        assert(si.prod(l,r).index_sum==Mint(1LL*(l+r-1)*(r-l)/2));
      }
    }
    using B=BinaryFlipInversions; vector<bool> bits(n); vector<B::S> bv;
    for(int i=0;i<n;++i) { bits[i]=rng()%2; bv.push_back(B::leaf(bits[i])); }
    Lazy<B> sb(bv);
    for(int step=0;step<100;++step) {
      int l=rng()%(n+1),r=rng()%(n+1); if(l>r)swap(l,r);
      if(step%2) { sb.apply(l,r,true); for(int i=l;i<r;++i)bits[i]=!bits[i]; }
      long long inv=0,ones=0; for(int i=l;i<r;++i) { if(bits[i])++ones; else inv+=ones; }
      auto s=sb.prod(l,r); assert(s.inversions==inv&&s.one==ones&&s.zero==r-l-ones);
    }
    using X=MaxSubarray<long long>; using C=MaxCount<long long>; using F=AffineComposition<Mint>;
    vector<long long> values(n); vector<X::S>xv; vector<C::S>cv; vector<Bracket::S>pv; vector<F::S>fv;
    vector<char> chars(n); vector<F::S> fs(n);
    for(int i=0;i<n;++i) { values[i]=int(rng()%21)-10; xv.push_back(X::leaf(values[i]));cv.push_back(C::leaf(values[i]));chars[i]=rng()%2?'(':')';pv.push_back(Bracket::leaf(chars[i]));fs[i]=F::leaf(rng()%5,rng()%7);fv.push_back(fs[i]); }
    Seg<X> sx(xv);Seg<C> sc(cv);Seg<Bracket> sp(pv);Seg<F> sf(fv);
    for(int i=0;i<n;i+=3) {values[i]=-values[i]; sx.set(i,X::leaf(values[i]));sc.set(i,C::leaf(values[i]));chars[i]=chars[i]=='('?')':'(';sp.set(i,Bracket::leaf(chars[i]));fs[i]=F::leaf(0,17);sf.set(i,fs[i]);}
    for(int l=0;l<=n;++l)for(int r=l;r<=n;++r) {
      long long best=0,sum=0,prefix=0,suffix=0,balance=0,minp=0,count=0,mx=0;
      for(int i=l;i<r;++i) {sum+=values[i];prefix=max(prefix,sum);balance+=chars[i]=='('?1:-1;minp=min(minp,balance);if(!count||values[i]>mx){mx=values[i];count=1;}else if(values[i]==mx)++count;
        long long cur=0;for(int j=i;j<r;++j){cur+=values[j];best=max(best,cur);} }
      long long cur=0;for(int i=r;i-->l;){cur+=values[i];suffix=max(suffix,cur);}
      auto s=sx.prod(l,r);assert(s.sum==sum&&s.prefix==prefix&&s.suffix==suffix&&s.best==best);
      auto c=sc.prod(l,r);assert(c.count==count&&(!count||c.value==mx));auto p=sp.prod(l,r);assert(p.sum==balance&&p.min_prefix==minp);
      Mint expected=13;for(int i=l;i<r;++i)expected=fs[i].a*expected+fs[i].b;auto f=sf.prod(l,r);assert(f.a*13+f.b==expected);
      // Associativity, including empty sides and noncommutative order.
      int m=(l+r)/2;auto joined=X::op(sx.prod(l,m),sx.prod(m,r));assert(joined.best==s.best&&joined.sum==s.sum);
    }
  }
}

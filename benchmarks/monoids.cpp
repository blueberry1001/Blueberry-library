// Same-contract recipe versus locally written ACL formula benchmark.
#include <chrono>
#include <iostream>
#include <random>
#include <sstream>
#include <utility>
#include <vector>
#include <atcoder/modint>
#include <atcoder/lazysegtree>
#include "blueberry/algebra/monoids.hpp"
using Mint=atcoder::modint998244353;
using Recipe=blueberry::monoid::AffineSum<Mint>;
struct Direct {
  struct S { Mint sum; long long len; }; struct F {Mint a,b;};
  static S op(S x,S y){return {x.sum+y.sum,x.len+y.len};}
  static S e(){return {0,0};}
  static S mapping(F f,S x){return {f.a*x.sum+f.b*x.len,x.len};}
  static F composition(F f,F g){return {f.a*g.a,f.a*g.b+f.b};}
  static F id(){return {1,0};}
};
using Clock=std::chrono::steady_clock;
double ms(Clock::time_point a,Clock::time_point b){return std::chrono::duration<double,std::milli>(b-a).count();}
struct Query {int l,r; Mint a,b;};
template<class M> void run(const char* name,int n,const std::vector<Query>& qs){
  auto start=Clock::now(); std::vector<typename M::S> v(n);for(int i=0;i<n;++i)v[i]={i,1};
  atcoder::lazy_segtree<typename M::S,M::op,M::e,typename M::F,M::mapping,M::composition,M::id> seg(v);
  auto built=Clock::now(); Mint checksum=0;
  for(unsigned i=0;i<qs.size();++i){auto q=qs[i];if(i%2)checksum+=seg.prod(q.l,q.r).sum;else seg.apply(q.l,q.r,{q.a,q.b});}
  auto done=Clock::now();int size=1;while(size<n)size*=2;
  std::cout<<name<<','<<n<<','<<qs.size()<<','<<ms(start,built)<<','<<ms(built,done)<<','<<(2*size*sizeof(typename M::S)+size*sizeof(typename M::F))<<','<<checksum.val()<<'\n';
}
int main(){
  std::mt19937 rng(712367);std::cout<<"variant,n,q,build_ms,operations_ms,tree_payload_bytes,checksum\n";
  for(int n:{1024,100000})for(int ratio:{1,4}){
    std::vector<Query> qs(n*ratio);for(auto& q:qs){q.l=rng()%(n+1);q.r=rng()%(n+1);if(q.l>q.r)std::swap(q.l,q.r);q.a=rng()%7;q.b=rng()%100;}
    for(int rep=0;rep<5;++rep){if(rep%2){run<Direct>("direct",n,qs);run<Recipe>("recipe",n,qs);}else{run<Recipe>("recipe",n,qs);run<Direct>("direct",n,qs);}}
  }
  // Isolated portable stream parse/format: deliberately excludes tree work.
  std::ostringstream input;for(int i=0;i<100000;++i)input<<i<<' ';
  auto text=input.str();auto a=Clock::now();std::istringstream in(text);int x;long long sum=0;while(in>>x)sum+=x;auto b=Clock::now();
  std::ostringstream out;for(int i=0;i<100000;++i)out<<i<<'\n';auto c=Clock::now();
  std::cerr<<"io,parse_ms="<<ms(a,b)<<",format_ms="<<ms(b,c)<<",bytes="<<out.str().size()<<",checksum="<<sum<<'\n';
}

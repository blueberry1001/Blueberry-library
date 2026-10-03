#include "blueberry/geometry/furthest-pair.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <string>
#include <boost/multiprecision/cpp_int.hpp>

using P = std::pair<long long, long long>;
using V = std::vector<P>;
using W = boost::multiprecision::int256_t;
static std::uint64_t cases=0, supports=0, ties=0, full_wraps=0;
static std::size_t largest_advances=0, largest_hull=0;
[[noreturn]] void fail(const char* what, const V& p) {
  std::cerr << "failure=" << what << " case=" << cases << '\n';
  for (auto [x,y]:p) std::cerr << x << ' ' << y << '\n';
  std::exit(1);
}
W dist(P a,P b) { W x=W(a.first)-b.first,y=W(a.second)-b.second;return x*x+y*y; }
W area(P a,P b,P c) { return (W(b.first)-a.first)*(W(c.second)-a.second)-(W(b.second)-a.second)*(W(c.first)-a.first); }
void check(const V& p,bool invariants=true) {
  ++cases;
  const auto before=p;
  auto [a,b]=blueberry::furthest_pair(p);
  if(p!=before) fail("mutation",p);
  if(p.size()<2) {if(a!=-1||b!=-1)fail("sentinel",p);return;}
  if(a<0||a>=b||std::size_t(b)>=p.size())fail("indices",p);
  W best=-1;
  for(std::size_t i=0;i<p.size();++i)for(std::size_t k=i+1;k<p.size();++k)best=std::max(best,dist(p[i],p[k]));
  if(dist(p[a],p[b])!=best)fail("distance",p);
  if(!invariants)return;
  // The returned hull is checked directly against every original point. The
  // all-original-pairs distance oracle above never depends on this hull.
  const auto h=blueberry::convex_hull(p);const auto n=h.size();
  if(n<3)return;
  for(std::size_t i=0;i<n;++i) {
    if(area(h[i],h[(i+1)%n],h[(i+2)%n])<=0)fail("non-strict hull",p);
    for(P q:p)if(area(h[i],h[(i+1)%n],q)<0)fail("non-supporting hull edge",p);
  }
  // Independent exact-integer height model of the production advancement.
  // Compare its selected support with a full scan on every edge; count the
  // unwrapped movement, and verify its considered pairs contain a diameter.
  std::size_t j=1,advances=0;W selected=-1;
  for(std::size_t i=0;i<n;++i) {
    auto height=[&](std::size_t k){return area(h[i],h[(i+1)%n],h[k%n]);};
    while(height(j+1)>height(j)) {
      ++j;++advances;
      if(advances>=2*n)fail("nonlinear sweep",p);
    }
    ++supports;
    W maximum=0;
    for(std::size_t k=0;k<n;++k)maximum=std::max(maximum,height(k));
    if(height(j)!=maximum||maximum<=0)fail("local but not global support",p);
    auto consider=[&](std::size_t k){selected=std::max(selected,dist(h[i],h[k%n]));selected=std::max(selected,dist(h[(i+1)%n],h[k%n]));};
    consider(j);
    if(height(j+1)==height(j)) {
      ++ties;consider(j+1);
      if(height(j+2)==height(j))fail("plateau longer than one edge",p);
    }
  }
  if(selected!=best)fail("antipodal coverage",p);
  if(j>=n)++full_wraps;
  largest_advances=std::max(largest_advances,advances);largest_hull=std::max(largest_hull,n);
}
int main() {
  constexpr long long B=(1LL<<62)-1;
  const std::array<long long,4> small{-2,-1,1,2};
  const std::array<long long,4> extreme{-B,-B/3,B/3,B};
  for(unsigned mask=0;mask<(1U<<16);++mask) {
    V p,q;
    for(unsigned i=0;i<16;++i)if(mask&(1U<<i)) {
      p.emplace_back(small[i/4],small[i%4]);q.emplace_back(extreme[i/4],extreme[i%4]);
    }
    check(p);check(q);
    std::reverse(q.begin(),q.end());
    if(!q.empty()) {q.push_back(q.front());q.push_back(q[q.size()/2]);}
    check(q,false);
  }
  // All subsets of the official non-unimodal-distance counterexample,
  // transformed by all eight signed coordinate permutations at large scale.
  const V trap{{0,0},{-18767,-43052},{-21874,-41502},{-100000,0},{-280,6810},{-189,4855}};
  for(unsigned mask=0;mask<64;++mask)for(int transform=0;transform<8;++transform) {
    V p;
    for(unsigned i=0;i<trap.size();++i)if(mask&(1U<<i)) {
      auto [x,y]=trap[i];if(transform&1)std::swap(x,y);if(transform&2)x=-x;if(transform&4)y=-y;
      p.emplace_back(x*(B/100000),y*(B/100000));
    }
    check(p);
  }
  std::cout << "{\"cases\":" << cases << ",\"support_checks\":" << supports
            << ",\"parallel_ties\":" << ties << ",\"wrapped_sweeps\":" << full_wraps
            << ",\"largest_hull\":" << largest_hull << ",\"largest_advances\":" << largest_advances
            << ",\"status\":\"pass\"}\n";
}

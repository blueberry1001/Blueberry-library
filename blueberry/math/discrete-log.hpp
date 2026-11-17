#pragma once
#include <atcoder/math>
#include <bit>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <optional>
#include <utility>
#include <vector>

namespace blueberry {
namespace discrete_log_detail {
using U = unsigned int;
inline std::vector<std::pair<U,int>> factors(U n) {
  std::vector<std::pair<U,int>> out;
  for (U p=2; p*p <= n; p += p == 2 ? 1 : 2) if (n % p == 0) {
    int e=0; do { n/=p; ++e; } while (n%p==0);
    out.emplace_back(p,e);
  }
  if (n>1) out.emplace_back(n,1);
  return out;
}
inline U power(U a, U e, const atcoder::internal::barrett& m) {
  U r=1; for (;e;e>>=1,a=m.mul(a,a)) if (e&1) r=m.mul(r,a); return r;
}
inline std::optional<U> search(U a,U b,U order,const atcoder::internal::barrett& m) {
  U v=1, small=order<32?order:32;
  for (U i=0;i<small;++i) { if (v==b) return i; v=m.mul(v,a); }
  if (order<=32) return std::nullopt;
  const U width=static_cast<U>(std::sqrt(order))+1;
  const U capacity=std::bit_ceil(width*2),mask=capacity-1;
  const int shift=64-std::countr_zero(capacity);
  std::vector<std::uint64_t> table(capacity);
  auto hash=[shift](U x){return U((x*0x9e3779b97f4a7c15ULL)>>shift);};
  v=1;
  for (U j=0;j<width;++j) {
    if (v==b) return j;
    U slot=hash(v); while(table[slot]) slot=(slot+1)&mask;
    table[slot]=(std::uint64_t(v)<<32)|j;
    v=m.mul(v,a);
  }
  U stride=U(atcoder::inv_mod(v,m.umod())), target=b;
  for (U i=1;i<=width;++i) {
    target=m.mul(target,stride);
    U slot=hash(target); while(table[slot] && U(table[slot]>>32)!=target) slot=(slot+1)&mask;
    if (table[slot]) { auto result=std::uint64_t(i)*width+U(table[slot]); if (result<order) return U(result); return std::nullopt; }
  }
  return std::nullopt;
}
}
inline std::optional<std::int64_t> discrete_log(std::int64_t a,std::int64_t b,std::int64_t m) {
  a%=m; b%=m; if(a<0) a+=m; if(b<0) b+=m;
  std::int64_t factor=1%m,offset=0;
  while(true) {
    if(b==factor) return offset;
    auto g=std::gcd(a,m); if(g==1) break;
    if(b%g) return std::nullopt;
    b/=g; m/=g; factor=factor*(a/g)%m; ++offset;
  }
  if(std::gcd(b,m)!=1) return std::nullopt;
  using namespace discrete_log_detail;
  atcoder::internal::barrett mod{U(m)};
  U base=U(a%m), target=U(b*atcoder::inv_mod(factor,m)%m), v=1;
  for(int j=0;j<512;++j) {
    if(v==target) return offset+j;
    if(j && v==1) return std::nullopt;
    v=mod.mul(v,base);
  }
  U phi=U(m); for(auto [p,e]:factors(U(m))) { (void)e; phi-=phi/p; }
  U order=phi; auto fs=factors(phi);
  for(auto& [p,e]:fs) while(e && power(base,order/p,mod)==1) order/=p,--e;
  if(power(target,order,mod)!=1) return std::nullopt;
  const U invbase=U(atcoder::inv_mod(base,m));
  std::uint64_t result=0,period=1;
  for(auto [p,e]:fs) if(e) {
    U generator=power(base,order/p,mod), remaining=target, residue=0, place=1, exponent=order;
    for(int i=0;i<e;++i) {
      exponent/=p;
      auto digit=search(generator,power(remaining,exponent,mod),p,mod);
      if(!digit) return std::nullopt;
      U term=*digit*place; residue+=term; place*=p;
      remaining=mod.mul(remaining,power(invbase,term,mod));
    }
    U delta=U((std::uint64_t(residue)+place-result%place)%place);
    delta=U(std::uint64_t(delta)*atcoder::inv_mod(period%place,place)%place);
    result+=period*delta; period*=place;
  }
  if(power(base,U(result),mod)!=target) return std::nullopt;
  return offset+std::int64_t(result);
}
}

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/taylor-shift.hpp"
#include "blueberry/math/polynomial-product.hpp"
#include "blueberry/math/stirling-second.hpp"
#include "blueberry/math/montmort.hpp"
using Mint = atcoder::modint998244353;
using Poly = std::vector<Mint>;
Poly multiply(const Poly& a, const Poly& b) {
  if(a.empty() || b.empty()) return {};
  Poly c(a.size()+b.size()-1);
  for(std::size_t i=0;i<a.size();++i) for(std::size_t j=0;j<b.size();++j) c[i+j]+=a[i]*b[j];
  return c;
}
void check(const Poly& actual, const Poly& expected, std::uint64_t seed, int trial, const char* name) {
  if(actual == expected) return;
  std::cerr<<"seed="<<seed<<" trial="<<trial<<" operation="<<name<<" expected:";
  for(auto x:expected)std::cerr<<' '<<x.val();
  std::cerr<<" actual:";for(auto x:actual)std::cerr<<' '<<x.val();std::cerr<<'\n'; std::abort();
}
int main(int argc,char** argv) {
  const std::uint64_t seed=argc>1?std::strtoull(argv[1],nullptr,10):1;
  std::mt19937_64 rng(seed);
  for(int trial=0;trial<150;++trial) {
    const int n=trial<5?trial:int(rng()%100); Mint shift=rng()%100;
    Poly f(n);for(auto& x:f)x=rng()%100;
    Poly expected, power{1};
    expected.resize(n);
    for(int i=0;i<n;++i){for(int j=0;j<=i;++j)expected[j]+=f[i]*power[j];power=multiply(power,{shift,1});}
    check(blueberry::taylor_shift(f,shift),expected,seed,trial,"shift");
    int count=rng()%20;std::vector<Poly> factors(count);expected={1};
    for(auto& a:factors){a.resize(rng()%10);for(auto& x:a)x=rng()%100;expected=multiply(expected,a);}
    check(blueberry::polynomial_product(factors),expected,seed,trial,"product");
    Poly dp(n+1);dp[0]=1;
    for(int i=1;i<=n;++i){for(int k=i;k;--k)dp[k]=dp[k-1]+Mint(k)*dp[k];dp[0]=0;}
    check(blueberry::stirling_second<Mint>(n),dp,seed,trial,"stirling");
    std::uint32_t mod=trial==0?1:trial==1?UINT32_MAX:static_cast<std::uint32_t>(rng()|1);
    auto d=blueberry::montmort(n,mod);
    std::vector<std::uint32_t> brute(n+1);brute[0]=1%mod;
    for(int i=2;i<=n;++i)brute[i]=(std::uint64_t(i-1)*(std::uint64_t(brute[i-1])+brute[i-2]))%mod;
    if(d!=brute){std::cerr<<"seed="<<seed<<" trial="<<trial<<" n="<<n<<" mod="<<mod<<'\n';std::abort();}
  }
  for(int n=0;n<=9;++n){std::vector<int> p(n);std::iota(p.begin(),p.end(),0);unsigned count=0;do{bool ok=true;for(int i=0;i<n;++i)ok&=p[i]!=i;count+=ok;}while(std::next_permutation(p.begin(),p.end()));assert(blueberry::montmort(n,UINT32_MAX)[n]==count);}
  // Explicit zero/trailing-zero and NTT-sized skewed products.
  assert(blueberry::polynomial_product<Mint>({}).front()==1);
  assert((blueberry::polynomial_product<Mint>({{0},{1,2}})==Poly{0,0}));
  std::vector<Poly> skew(100,Poly{1});skew[0]=Poly(256,1);skew.back()={1,1};
  check(blueberry::polynomial_product(skew),multiply(skew[0],skew.back()),seed,150,"skew");
  check(blueberry::taylor_shift(Poly(256,1),Mint(0)),Poly(256,1),seed,151,"zero shift");
  assert((blueberry::stirling_second<atcoder::static_modint<17>>(0)==std::vector<atcoder::static_modint<17>>{1}));
}

// Independently written baseline recurrences, not copied reference code.
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <atcoder/modint>
#include "blueberry/math/taylor-shift.hpp"
#include "blueberry/math/polynomial-product.hpp"
#include "blueberry/math/stirling-second.hpp"
#include "blueberry/math/montmort.hpp"
using M=atcoder::modint998244353;
using V=std::vector<M>;
V shift_slow(const V& a,M c){V b(a.size());for(int i=int(a.size())-1;i>=0;--i){for(int j=int(a.size())-1;j>0;--j)b[j]=b[j]*c+b[j-1];b[0]=b[0]*c+a[i];}return b;}
V stirling_slow(int n){V a(n+1);a[0]=1;for(int i=1;i<=n;++i){for(int j=i;j;--j)a[j]=a[j]*j+a[j-1];a[0]=0;}return a;}
V product_slow(const std::vector<V>& f){V a{1};for(const auto& b:f)a=atcoder::convolution(a,b);return a;}
std::vector<std::uint32_t> derangement_alternative(int n,std::uint32_t mod){std::vector<std::uint32_t>a(n+1);a[0]=1%mod;for(int i=2;i<=n;++i)a[i]=std::uint64_t(i-1)*(std::uint64_t(a[i-1])+a[i-2])%mod;return a;}
template<class F>void run(const char* name,F fn){for(int r=0;r<5;++r){auto start=std::chrono::steady_clock::now();auto a=fn();auto end=std::chrono::steady_clock::now();std::uint64_t sum=0;for(auto x:a){if constexpr(requires{x.val();})sum=sum*1000003+x.val();else sum=sum*1000003+x;}std::cout<<name<<","<<r<<","<<std::chrono::duration<double,std::micro>(end-start).count()<<","<<sum<<'\n';}}
int main(){std::mt19937 rng(123);V f(2048);for(auto& x:f)x=rng();std::vector<V> factors(512,V(2));for(auto& a:factors)for(auto& x:a)x=rng();std::cout<<"candidate,run,microseconds,checksum\n";run("shift-convolution-n2048",[&]{return blueberry::taylor_shift(f,M(47));});run("shift-horner-n2048",[&]{return shift_slow(f,M(47));});run("product-balanced-k512",[&]{return blueberry::polynomial_product(factors);});run("product-sequential-k512",[&]{return product_slow(factors);});run("stirling-convolution-n2048",[]{return blueberry::stirling_second<M>(2048);});run("stirling-dp-n2048",[]{return stirling_slow(2048);});run("montmort-one-step-n1000000",[]{return blueberry::montmort(1000000,1000000000);});run("montmort-two-step-n1000000",[]{return derangement_alternative(1000000,1000000000);});}

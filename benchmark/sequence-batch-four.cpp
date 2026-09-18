#include <chrono>
#include <iostream>
#include <random>
#include "blueberry/data-structure/static-range-inversions.hpp"
#include "blueberry/data-structure/static-range-mode.hpp"
#include "blueberry/string/longest-common-substring.hpp"
using Clock = std::chrono::steady_clock;
template<class F> void measure(const char* label, F f) {
  for(int run=0;run<3;++run) {
    auto begin=Clock::now(); auto checksum=f();
    double ms=std::chrono::duration<double,std::milli>(Clock::now()-begin).count();
    std::cout<<label<<" run="<<run<<" ms="<<ms<<" checksum="<<checksum<<'\n';
  }
}
int main() {
  std::mt19937 rng(20260918);
  const int n=1000,q=1000;
  std::vector<int> a(n); for(int& x:a)x=rng()%100;
  std::vector<std::pair<int,int>> queries(q);
  for(auto& [l,r]:queries){ l=rng()%(n+1);r=rng()%(n+1);if(l>r)std::swap(l,r); }
  measure("inversions-mo",[&]{long long sum=0;for(auto x:blueberry::static_range_inversions(a,queries))sum+=x;return sum;});
  measure("inversions-direct",[&]{long long sum=0;for(auto [l,r]:queries)for(int i=l;i<r;++i)for(int j=i+1;j<r;++j)sum+=a[i]>a[j];return sum;});
  measure("mode-mo",[&]{long long sum=0;for(auto [i,f]:blueberry::static_range_mode(a,queries))sum+=f;return sum;});
  measure("mode-direct",[&]{long long sum=0;for(auto [l,r]:queries){int count[100]={},best=0;for(int i=l;i<r;++i)best=std::max(best,++count[a[i]]);sum+=best;}return sum;});
  std::string s(4000,'a'),t(4000,'a');for(char& c:s)c+=rng()%4;for(char& c:t)c+=rng()%4;
  measure("substring-acl",[&]{auto z=blueberry::longest_common_substring(s,t);return z[1]-z[0];});
  measure("substring-dp",[&]{std::vector<int> dp(t.size()+1);int best=0;for(char c:s)for(int j=static_cast<int>(t.size());j>0;--j){dp[j]=c==t[j-1]?dp[j-1]+1:0;best=std::max(best,dp[j]);}return best;});
}

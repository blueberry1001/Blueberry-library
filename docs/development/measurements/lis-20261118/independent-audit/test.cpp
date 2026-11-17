#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <random>
#include <vector>
#include "candidate.hpp"

template<class T> bool test(const std::vector<T>& a) {
  const int n = static_cast<int>(a.size());
  int best=0;std::vector<int> dp(n,1);
  for(int i=0;i<n;++i){for(int j=0;j<i;++j)if(a[j]<a[i])dp[i]=std::max(dp[i],dp[j]+1);best=std::max(best,dp[i]);}
  auto v=blueberry::longest_increasing_subsequence(a);
  if(static_cast<int>(v.size())!=best)return false;
  for(int i=0;i<best;++i)if(v[i]<0||v[i]>=n||(i&&(!(v[i-1]<v[i])||!(a[v[i-1]]<a[v[i]]))))return false;
  return true;
}
template<class T> bool enumerate(){
 for(int n=0;n<=8;++n){int count=1;for(int i=0;i<n;++i)count*=4;
  for(int mask=0;mask<count;++mask){auto x=mask;std::vector<T>a(n);for(auto& v:a){v=static_cast<T>(x%4);x/=4;}if(!test(a))return false;}}
 return true;
}
struct Pred {bool value; explicit operator bool() const{return value;}};
struct MoveOnly {
 int v;explicit MoveOnly(int x):v(x){}MoveOnly(const MoveOnly&)=delete;MoveOnly(MoveOnly&&)=default;
 Pred operator<(const MoveOnly& o)const{return {v<o.v};}
};
int main(){
 if(!enumerate<std::int8_t>()||!enumerate<std::uint8_t>()||!enumerate<std::int32_t>()||!enumerate<std::uint64_t>())return 1;
 std::vector<MoveOnly>a;for(int i:{4,1,2,2,0,5})a.emplace_back(i);if(!test(a))return 2;
 std::vector<bool>b{1,0,0,1,1,0,1};if(!test(b))return 3;
 using L=std::numeric_limits<std::int64_t>;using U=std::numeric_limits<std::uint64_t>;
 if(!test(std::vector<std::int64_t>{L::max(),L::min(),-1,0,L::max(),L::min()}))return 4;
 if(!test(std::vector<std::uint64_t>{U::max(),0,U::max()-1,U::max(),0}))return 5;
 std::mt19937_64 gen(194153);for(int round=0;round<10000;++round){int n=gen()%80;std::vector<std::int64_t>x(n);for(auto&v:x)v=static_cast<std::int64_t>(gen());if(!test(x))return 6;}
 std::puts("OK 349524 exhaustive four-alphabet inputs, 10000 wide random, extremes, bool and move-only explicit-bool comparator");
}

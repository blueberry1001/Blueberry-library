#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
namespace blueberry{
template<class T,class U>constexpr bool chmin(T&value,const U&candidate){
 if(candidate<value){value=candidate;return true;}
 return false;
}
template<class T,class U>constexpr bool chmax(T&value,const U&candidate){
 if(value<candidate){value=candidate;return true;}
 return false;
}
template<class T>requires(std::is_integral_v<T>&&std::is_signed_v<T>)
constexpr T floor_div(T a,T b){
 const T q=a/b,r=a%b;
 return q-T(r!=0&&((r<0)!=(b<0)));
}
template<class F>long long bin_search(long long ok,long long ng,const F&pred){
 while((ok<ng?static_cast<unsigned long long>(ng)-static_cast<unsigned long long>(ok)
         :static_cast<unsigned long long>(ok)-static_cast<unsigned long long>(ng))>1){
  const long long mid=std::midpoint(ok,ng);
  (pred(mid)?ok:ng)=mid;
 }
 return ok;
}
template<class T>T sum(const std::vector<T>&values){
 T result{};
 for(const auto&value:values)result+=value;
 return result;
}
template<class T>T min_of(const std::vector<T>&values){return*std::min_element(values.begin(),values.end());}
template<class T>T max_of(const std::vector<T>&values){return*std::max_element(values.begin(),values.end());}
template<class Container>int len(const Container&values){return static_cast<int>(values.size());}
template<class T,std::size_t N,class U>void Fill(T(&values)[N],const U&value){
 for(auto&element:values){
  if constexpr(std::is_array_v<T>)Fill(element,value);
  else element=value;
 }
}
inline bool is_palindrome(std::string_view text){return std::equal(text.begin(),text.begin()+text.size()/2,text.rbegin());}
inline long double my_distance(long double x,long double y,long double xx,long double yy){return std::hypot(x-xx,y-yy);}
inline __int128 range_sum(long long a,long long b){
 if(a>b)return 0;
 return(__int128(a)+b)*(__int128(b)-a+1)/2;
}
inline int compress(std::vector<long long>&values){
 auto unique=values;
 std::sort(unique.begin(),unique.end());
 unique.erase(std::unique(unique.begin(),unique.end()),unique.end());
 for(auto&value:values)value=std::lower_bound(unique.begin(),unique.end(),value)-unique.begin();
 return static_cast<int>(unique.size());
}
template<class Index>requires std::is_integral_v<Index>
bool is_bipartite(const std::vector<std::vector<Index>>&graph){
 const int n=static_cast<int>(graph.size());
 std::vector<int>color(n,-1),queue;
 queue.reserve(n);
 for(int root=0;root<n;++root){
  if(color[root]!=-1)continue;
  color[root]=0;
  queue.clear();
  queue.push_back(root);
  for(std::size_t i=0;i<queue.size();++i){
   const int u=queue[i];
   for(auto neighbor:graph[u]){
    const int v=static_cast<int>(neighbor);
    if(color[v]==color[u])return false;
    if(color[v]==-1){color[v]=color[u]^1;queue.push_back(v);}
   }
  }
 }
 return true;
}
inline std::string to_string128(__int128 value){
 const bool negative=value<0;
 unsigned __int128 magnitude=negative?0-static_cast<unsigned __int128>(value)
                    :static_cast<unsigned __int128>(value);
 char buffer[40];
 char*end=buffer+sizeof buffer;
 char*begin=end;
 do{*--begin=static_cast<char>('0'+magnitude%10);magnitude/=10;}while(magnitude);
 if(negative)*--begin='-';
 return std::string(begin,end);
}
inline std::optional<__int128>parse128(std::string_view text){
 if(text.empty())return std::nullopt;
 const bool negative=text.front()=='-';
 if(negative||text.front()=='+')text.remove_prefix(1);
 if(text.empty())return std::nullopt;
 const unsigned __int128 limit=(static_cast<unsigned __int128>(1)<<127)-!negative;
 unsigned __int128 value=0;
 for(char ch:text){
  if(ch<'0'||ch>'9')return std::nullopt;
  const unsigned digit=static_cast<unsigned>(ch-'0');
  if(value>limit/10||(value==limit/10&&digit>limit%10))return std::nullopt;
  value=value*10+digit;
 }
 if(!negative)return static_cast<__int128>(value);
 return-static_cast<__int128>(value-(value!=0))-(value!=0);
}
}

#pragma once
#ifdef BLUEBERRY_CONTEST_STREAM_IO_INCLUDED
#error Use only one contest I/O template in a translation unit.
#endif
#define BLUEBERRY_CONTEST_FAST_IO_INCLUDED
#include "blueberry/utility/internal/contest-template-base.hpp"
#include "blueberry/utility/fast-io.hpp"
namespace blueberry::internal::contest_io{
inline FastInput<>&input(){static FastInput<>value;return value;}
inline FastOutput&output(){static FastOutput value;return value;}
inline int precision=30;
template<class T>struct sequence:std::false_type{};
template<class T,class A>struct sequence<std::vector<T,A>>:std::true_type{};
template<class T,class A>struct sequence<std::deque<T,A>>:std::true_type{};
template<class T,std::size_t N>struct sequence<std::array<T,N>>:std::true_type{};
template<class T>struct pair:std::false_type{};
template<class T,class U>struct pair<std::pair<T,U>>:std::true_type{};
template<class T>bool write(FastOutput&out,const T&value){
 if constexpr(std::is_floating_point_v<T>)return out.write(value,precision);
 else if constexpr(pair<T>::value){return write(out,value.first)&&out.write(' ')&&write(out,value.second);}else if constexpr(sequence<T>::value){
  bool first=true;
  for(const auto&element:value){
   if(!first&&!out.write(' '))return false;
   first=false;
   if(!write(out,element))return false;
  }
  return true;
 }else return out.write(value);
}
}
inline void init_io(int precision=30){blueberry::internal::contest_io::precision=precision;}
template<class...Args>bool scan(Args&...values){
 if constexpr(sizeof...(Args)==0)return true;
 else{
  auto&in=blueberry::internal::contest_io::input();
  return(in.read(values)&&...);
 }
}
template<class...Args>bool print(const Args&...values){
 auto&out=blueberry::internal::contest_io::output();
 if constexpr(sizeof...(Args)>0){
  bool first=true;
  auto one=[&](const auto&value){
   if(!first&&!out.write(' '))return false;
   first=false;
   return blueberry::internal::contest_io::write(out,value);
  };
  if(!(one(values)&&...))return false;
 }
 return out.write('\n');
}
inline bool flush(){return blueberry::internal::contest_io::output().flush();}
#define YesNo(x) print((x) ? "Yes" : "No")
#define YESNO(x) print((x) ? "YES" : "NO")

#pragma once
#ifdef BLUEBERRY_CONTEST_STREAM_IO_INCLUDED
#error Use only one contest I/O template in a translation unit.
#endif
#define BLUEBERRY_CONTEST_FAST_IO_INCLUDED
#include "blueberry/utility/internal/contest-template-base.hpp"
#include "blueberry/utility/fast-io.hpp"
namespace blueberry::internal::contest_io{
class input_buffer:public std::streambuf{
 FastInput<>&in;
 int_type underflow()override{
  commit();int c=in.next();if(c==EOF)return traits_type::eof();--in.pos_;
  char*p=reinterpret_cast<char*>(in.buffer_.data());setg(p+in.pos_,p+in.pos_,p+in.end_);return traits_type::to_int_type(*gptr());
 }
public:
 explicit input_buffer(FastInput<>&value):in(value){}
 void commit(){if(eback()){in.pos_=static_cast<std::size_t>(gptr()-reinterpret_cast<char*>(in.buffer_.data()));setg(nullptr,nullptr,nullptr);}}
};
class output_buffer:public std::streambuf{
 FastOutput&out;
 int_type overflow(int_type c)override{return traits_type::eq_int_type(c,traits_type::eof())?traits_type::not_eof(c):out.write(traits_type::to_char_type(c))?c:traits_type::eof();}
 std::streamsize xsputn(const char*p,std::streamsize n)override{return out.write(std::string_view(p,static_cast<std::size_t>(n)))?n:0;}
 int sync()override{return out.flush()?0:-1;}
public:
 explicit output_buffer(FastOutput&value):out(value){}
};
struct streams{
 FastInput<>input;
 FastOutput output;
 input_buffer in{input};
 output_buffer out{output};
 std::streambuf*old_in,*old_out;
 streams(){std::ios::sync_with_stdio(false);old_in=std::cin.rdbuf(&in);old_out=std::cout.rdbuf(&out);std::cin.tie(nullptr);}
 ~streams(){output.flush();std::cin.rdbuf(old_in);std::cout.rdbuf(old_out);}
};
inline streams&state(){static streams value;return value;}
inline FastOutput&output(){return state().output;}
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
inline void init_io(int precision=30){blueberry::internal::contest_io::state();blueberry::internal::contest_io::precision=precision;std::cout<<std::fixed<<std::setprecision(precision);}
template<class...Args>bool scan(Args&...values){
 if constexpr(sizeof...(Args)==0)return true;
 else{
  auto&io=blueberry::internal::contest_io::state();io.in.commit();
  return(io.input.read(values)&&...);
 }
}
namespace blueberry::internal::contest_io{
class fast_input_stream{
 bool ok=true;
public:
 template<class T>fast_input_stream&operator>>(T&value){if(ok)ok=::scan(value);return *this;}
 explicit operator bool()const{return ok;}
 bool fail()const{return!ok;}
 void clear(){ok=true;}
};
}
inline blueberry::internal::contest_io::fast_input_stream fin;
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

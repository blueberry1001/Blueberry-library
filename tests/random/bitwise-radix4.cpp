#include <atcoder/modint>
#include <cstdint>
#include <iostream>
#include <random>
#include <type_traits>
#include <vector>
#include "blueberry/math/bitwise-convolution.hpp"

struct Proxy { int value; };
struct Value {
  int value;
  Value(int n=0):value(n%101){}
  Value(const Value&)=default;
  Value(Value&&)=default;
  Value& operator=(const Value&)=delete;
  Value& operator=(Value&&)=delete;
  Value& operator=(Proxy x){value=x.value;return *this;}
  Value& operator+=(const Value& x){value=(value+x.value)%101;return *this;}
  Value& operator-=(const Value& x){value=(value-x.value+101)%101;return *this;}
  Value& operator*=(const Value& x){value=value*x.value%101;return *this;}
  friend Proxy operator+(const Value& x,const Value& y){return {(x.value+y.value)%101};}
  friend Proxy operator-(const Value& x,const Value& y){return {(x.value-y.value+101)%101};}
  friend Value operator/(const Value& x,const Value& y){
    int b=y.value,e=99,r=1;for(;e;e>>=1,b=b*b%101)if(e&1)r=r*b%101;return Value(x.value*r%101);
  }
};
static_assert(!std::is_convertible_v<Proxy, Value>);

struct Mat {
  int a,b,c,d;
  Mat(int v=0):a(v%101),b(0),c(0),d(v%101){}
  Mat& operator+=(const Mat& x){a=(a+x.a)%101;b=(b+x.b)%101;c=(c+x.c)%101;d=(d+x.d)%101;return *this;}
  Mat& operator-=(const Mat& x){a=(a-x.a+101)%101;b=(b-x.b+101)%101;c=(c-x.c+101)%101;d=(d-x.d+101)%101;return *this;}
  Mat& operator*=(const Mat& x){int z0=(a*x.a+b*x.c)%101,z1=(a*x.b+b*x.d)%101,z2=(c*x.a+d*x.c)%101,z3=(c*x.b+d*x.d)%101;a=z0;b=z1;c=z2;d=z3;return *this;}
  friend Mat operator+(Mat x,const Mat& y){return x+=y;}
  friend Mat operator-(Mat x,const Mat& y){return x-=y;}
  friend Mat operator*(Mat x,const Mat& y){return x*=y;}
  friend Mat operator/(const Mat& x,const Mat& y){int b0=y.a,e=99,r=1;for(;e;e>>=1,b0=b0*b0%101)if(e&1)r=r*b0%101;return x*Mat(r);}
  friend bool operator==(const Mat& x,const Mat& y){return x.a==y.a&&x.b==y.b&&x.c==y.c&&x.d==y.d;}
};

bool proxy_test(std::mt19937_64& gen) {
  for(int bits=0;bits<=7;++bits) for(int rep=0;rep<12;++rep) {
    int n=1<<bits;std::vector<Value>a,b;std::vector<int>u(n),v(n),w(n);
    for(int i=0;i<n;++i){a.emplace_back(int(gen()%101));b.emplace_back(int(gen()%101));}
    for(int i=0;i<n;++i)for(int j=0;j<n;++j){int z=a[i].value*b[j].value;u[i&j]=(u[i&j]+z)%101;v[i|j]=(v[i|j]+z)%101;w[i^j]=(w[i^j]+z)%101;}
    auto ands=blueberry::bitwise_convolution_and(a,b),ors=blueberry::bitwise_convolution_or(a,b),xors=blueberry::bitwise_convolution_xor(a,b);
    for(int i=0;i<n;++i)if(ands[i].value!=u[i]||ors[i].value!=v[i]||xors[i].value!=w[i])return false;
  }
  return true;
}

template<class T, bool Xor> bool test(std::mt19937_64& gen) {
  for(int bits=0;bits<=8;++bits) for(int rep=0;rep<15;++rep) {
    int n=1<<bits;std::vector<T> a(n),b(n),u(n),v(n),w(n);
    for(int i=0;i<n;++i) {a[i]=T(gen()%3);b[i]=T(gen()%3);}
    for(int i=0;i<n;++i) for(int j=0;j<n;++j) {
      u[i&j]+=a[i]*b[j];v[i|j]+=a[i]*b[j];
      if constexpr(Xor)w[i^j]+=a[i]*b[j];
    }
    if(blueberry::bitwise_convolution_and(a,b)!=u||blueberry::bitwise_convolution_or(a,b)!=v) {
      std::cerr<<"AND/OR mismatch bits="<<bits<<" rep="<<rep<<'\n';return false;
    }
    if constexpr(Xor) if(blueberry::bitwise_convolution_xor(a,b)!=w) {
      std::cerr<<"XOR mismatch bits="<<bits<<" rep="<<rep<<'\n';return false;
    }
  }
  return true;
}
int main(){
  std::mt19937_64 gen(7251231);
  if(!test<atcoder::modint998244353,true>(gen))return 1;
  if(!test<atcoder::static_modint<3>,true>(gen))return 2;
  if(!test<atcoder::static_modint<7>,true>(gen))return 3;
  if(!test<std::uint32_t,false>(gen)||!test<std::uint64_t,false>(gen))return 4;
  if(!test<std::int64_t,false>(gen)||!test<double,true>(gen))return 5;
  if(!proxy_test(gen))return 9;
  for(int bits=0;bits<=7;++bits)for(int rep=0;rep<5;++rep){
    int n=1<<bits;std::vector<Mat>a(n),b(n),u(n),v(n),w(n);
    for(int i=0;i<n;++i){a[i].a=int(gen()%101);a[i].b=int(gen()%101);a[i].c=int(gen()%101);a[i].d=int(gen()%101);b[i].a=int(gen()%101);b[i].b=int(gen()%101);b[i].c=int(gen()%101);b[i].d=int(gen()%101);}
    for(int i=0;i<n;++i)for(int j=0;j<n;++j){u[i&j]+=a[i]*b[j];v[i|j]+=a[i]*b[j];w[i^j]+=a[i]*b[j];}
    if(blueberry::bitwise_convolution_and(a,b)!=u||blueberry::bitwise_convolution_or(a,b)!=v||blueberry::bitwise_convolution_xor(a,b)!=w)return 10;
  }
  using M=atcoder::modint998244353;
  if(!blueberry::bitwise_convolution_and(std::vector<M>{},std::vector<M>{}).empty())return 6;
  if(!blueberry::bitwise_convolution_or(std::vector<M>{},std::vector<M>{}).empty())return 7;
  if(!blueberry::bitwise_convolution_xor(std::vector<M>{},std::vector<M>{}).empty())return 8;
  std::cout<<"PASS seed=7251231, lengths 0..256, 9 coefficient types\n";
}

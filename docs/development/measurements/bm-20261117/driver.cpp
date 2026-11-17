#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <atcoder/modint>
#include "baseline.hpp.txt"
#include "candidate.hpp.txt"
#include "../../../../blueberry/math/linear-recurrence.hpp"
using namespace std;
using Mint = atcoder::modint998244353;
struct Counted {
  Mint value;
  static inline uint64_t copies=0, multiplications=0, divisions=0;
  Counted(int x=0) : value(x) {}
  Counted(const Counted& x) : value(x.value) { ++copies; }
  Counted& operator=(const Counted& x) { value=x.value; ++copies; return *this; }
  Counted& operator+=(const Counted& x) { value+=x.value; return *this; }
  Counted& operator-=(const Counted& x) { value-=x.value; return *this; }
  friend Counted operator*(const Counted& a,const Counted& b) { ++multiplications; Counted r; r.value=a.value*b.value; return r; }
  friend Counted operator/(const Counted& a,const Counted& b) { ++divisions; Counted r; r.value=a.value/b.value; return r; }
  Counted operator-() const { Counted r; r.value=-value; return r; }
  friend bool operator==(const Counted& a,const Counted& b) { return a.value==b.value; }
};
template<class T> vector<T> input(int n,int kind) {
  vector<T> a(n); uint64_t x=1729;
  for(int i=0;i<n;++i) {
    x^=x<<7; x^=x>>9; x^=x<<8;
    if(kind==0) a[i]=int(x%998244353);
    else if(kind==1) { if(i>=2) { a[i]=a[i-1]; a[i]+=a[i-2]; } else a[i]=i+1; }
    else if(kind==2) a[i]=i%97==0?int(x%998244353):0;
    else if(kind==3) a[i]=0;
    else a[i]=i<n/2?0:int(x%998244353);
  }
  return a;
}
template<class T> vector<T> compute(int variant,const vector<T>& a) {
  if(variant==0) return baseline::berlekamp_massey(a);
  if(variant==1) return candidate::berlekamp_massey(a);
  return blueberry::berlekamp_massey(a);
}
int main(int argc,char** argv) {
  if(argc==1) {
    for(int kind=0;kind<5;++kind) for(int n=0;n<=200;++n) {
      auto a=input<Mint>(n,kind); auto old=compute(0,a);
      if(old!=compute(1,a)||old!=compute(2,a)) return 2;
    }
    cout<<"differential ok\n"; return 0;
  }
  const int variant=atoi(argv[1]),n=atoi(argv[2]),kind=atoi(argv[3]),reps=atoi(argv[4]);
  if(argc>5) {
    auto a=input<Counted>(n,kind); Counted::copies=Counted::multiplications=Counted::divisions=0;
    auto c=compute(variant,a);
    cout<<c.size()<<' '<<Counted::copies<<' '<<Counted::multiplications<<' '<<Counted::divisions<<'\n'; return 0;
  }
  auto a=input<Mint>(n,kind); uint64_t hash=0;
  const auto start=chrono::steady_clock::now();
  for(int rep=0;rep<reps;++rep) {
    auto result=compute(variant,a); hash=hash*1000003+result.size();
    for(const auto& y:result) hash=hash*37+y.val();
  }
  cout<<chrono::duration<double,milli>(chrono::steady_clock::now()-start).count()<<' '<<hash<<'\n';
}

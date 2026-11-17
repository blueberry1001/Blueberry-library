#include <cassert>
#include <cstdint>
#include <deque>
#include <iostream>
#include <string>
#include "blueberry/data-structure/aggregate-deque.hpp"
struct F { uint64_t a,b; friend bool operator==(F x,F y){return x.a==y.a&&x.b==y.b;} };
constexpr uint64_t mod=998244353;
F op(F x,F y){return{x.a*y.a%mod,(x.b*y.a+y.b)%mod};} F id(){return{1,0};}
std::string cat(std::string a,std::string b){return a+b;} std::string empty(){return{};}
struct C { int x; C(int x=0):x(x){} C(const C&)=default; C(C&&)=default; C& operator=(const C&)=delete; C& operator=(C&&)=delete; };
C plus_c(const C& a,const C& b){return C(a.x+b.x);} C zero_c(){return C();}
uint64_t state=0x3a6772;
uint64_t rand64(){state^=state<<7;state^=state>>9;return state;}
int main(){
  for(int n=1;n<=513;++n) for(int side=0;side<2;++side){
    blueberry::AggregateDeque<F,op,id>d;std::deque<F>b;
    for(int i=0;i<n;++i){F v{rand64()%mod,rand64()%mod};if(side){d.push_front(v);b.push_front(v);}else{d.push_back(v);b.push_back(v);}}
    for(int i=0;i<n;++i){F x=id();for(auto v:b)x=op(x,v);assert(d.prod()==x);assert(d.front()==b.front()&&d.back()==b.back());if(side){d.pop_back();b.pop_back();}else{d.pop_front();b.pop_front();}}
    assert(d.empty()&&d.prod()==id());
  }
  blueberry::AggregateDeque<F,op,id>d;std::deque<F>b;
  for(int i=0;i<1000000;++i){
    auto t=rand64()%6;
    if(b.empty()||((t<2)&&b.size()<100)){F x{rand64()%mod,rand64()%mod};if(t&1){d.push_front(x);b.push_front(x);}else{d.push_back(x);b.push_back(x);}}
    else if(t&1){d.pop_front();b.pop_front();}else{d.pop_back();b.pop_back();}
    F x=id();for(auto v:b)x=op(x,v);assert(d.prod()==x);assert(d.size()==int(b.size()));if(!b.empty())assert(d.front()==b.front()&&d.back()==b.back());
  }
  blueberry::AggregateDeque<std::string,cat,empty>s;
  for(int i=0;i<32;++i)s.push_back(std::string(1,char('A'+i%26)));
  for(int i=0;i<15;++i)s.pop_front(); assert(s.prod()=="PQRSTUVWXYZABCDEF");
  blueberry::AggregateDeque<C,plus_c,zero_c> c;C one(1),two(2);
  c.push_back(one);c.push_back(two);c.push_back(one);assert(c.prod().x==4);c.pop_front();assert(c.prod().x==3);c.pop_back();assert(c.prod().x==2);
  std::cout<<"PASS seed=0x3a6772 lengths=1..513 and 1000000 random steps\n";
}

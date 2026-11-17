#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <vector>
#include "blueberry/data-structure/dual-segment-tree.hpp"

struct S { std::uint32_t v; S()=delete; explicit S(std::uint32_t x):v(x){} };
struct F { std::uint32_t a,b; F()=delete; F(std::uint32_t x,std::uint32_t y):a(x),b(y){} };
S mapping(F f,S x){return S(f.a*x.v+f.b);}
F composition(F f,F g){return F(f.a*g.a,f.a*g.b+f.b);}
F id(){return F(1,0);}
using Tree=blueberry::DualSegmentTree<S,F,mapping,composition,id>;

int bitmap(bool f,int x){return f?~x:x;}
bool bitcomp(bool f,bool g){return f!=g;}
bool bitid(){return false;}
using Bits=blueberry::DualSegmentTree<int,bool,bitmap,bitcomp,bitid>;
bool boolmap(bool f,bool x){return f!=x;}
using Boolean=blueberry::DualSegmentTree<bool,bool,boolmap,bitcomp,bitid>;

std::string strmap(const std::string& f,const std::string& s){return s+f;}
std::string strcomp(const std::string& f,const std::string& g){return g+f;}
std::string strid(){return {};}
using Strings=blueberry::DualSegmentTree<std::string,std::string,strmap,strcomp,strid>;

int main(){
 std::mt19937 rng(732517);
 for(int n=0;n<=130;++n){
  std::vector<S>a(n,S(0));for(S& x:a)x=S(rng());Tree tree(a);
  for(int step=0;step<1000;++step){int l=n?rng()%(n+1):0,r=n?rng()%(n+1):0;if(l>r)std::swap(l,r);F f(rng(),rng());
   switch(rng()%6){
    case 0:case 1:tree.apply(l,r,f);for(int j=l;j<r;++j){a[j]=mapping(f,a[j]);}break;
    case 2:if(l<n){S value(rng());tree.set(l,value);a[l]=value;}break;
    case 3:if(l<n){tree.apply(l,f);a[l]=mapping(f,a[l]);}break;
    case 4:if(l<n&&tree.get(l).v!=a[l].v)return 1;break;
    case 5:{Tree copy=tree;Tree moved=std::move(copy);auto z=moved.get_all();if(z.size()!=a.size()||copy.size()!=0)return 2;for(int i=0;i<n;++i){if(z[i].v!=a[i].v)return 3;}break;}
   }
  }
  auto all=tree.get_all();if(all.size()!=a.size())return 4;for(int i=0;i<n;++i){if(all[i].v!=a[i].v)return 5;}
 }
 for(int n=1;n<=70;++n){
  std::vector<int> expected(n,7);Bits t(expected);
  for(int k=0;k<300;++k){int l=rng()%n,r=l+1+rng()%(n-l);bool f=rng()&1;t.apply(l,r,f);for(int i=l;i<r;++i)expected[i]=bitmap(f,expected[i]);int p=rng()%n;if(t.get(p)!=expected[p])return 6;}
  if(t.get_all()!=expected)return 7;
 }
 Boolean b(std::vector<bool>{false,true,false,true,false});b.apply(0,5,true);b.apply(1,4,true);
 if(b.get_all()!=std::vector<bool>{true,true,false,true,true})return 10;
 Strings s(std::vector<std::string>{"a","b","c","d","e"});
 s.apply(0,5,"X");s.apply(1,4,"Y");s.apply(2,5,"Z");
 if(s.get(2)!="cXYZ"||s.get(4)!="eXZ")return 8;
 s.set(2,"z");s.apply(0,3,"Q");
 if(s.get_all()!=std::vector<std::string>{"aXQ","bXYQ","zQ","dXYZ","eXZ"})return 9;
 std::cout<<"PASS seed=732517\n";
}

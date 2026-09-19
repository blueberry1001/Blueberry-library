#include "blueberry/data-structure/fast-set.hpp"
#include "blueberry/data-structure/dual-segment-tree.hpp"
#include "blueberry/data-structure/double-ended-priority-queue.hpp"
#include "blueberry/data-structure/skew-heap.hpp"
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <random>
#include <set>
#include <string>
#include <vector>
constexpr long long mod = 998244353;
struct F { long long a,b; F()=delete; F(long long x,long long y):a(x),b(y){} };
long long mapping(F f,long long x){return (f.a*x+f.b)%mod;}
F composition(F f,F g){return {f.a*g.a%mod,(f.a*g.b+f.b)%mod};}
F id(){return {1,0};}
using Dual=blueberry::DualSegmentTree<long long,F,mapping,composition,id>;
unsigned long long seed; int step;
void check(bool good,const char* label){if(!good){std::cerr<<"seed="<<seed<<" step="<<step<<" operation="<<label<<'\n';std::exit(1);}}
int main(int argc,char**argv){
 seed=argc>1?std::strtoull(argv[1],nullptr,10):1;std::mt19937_64 rng(seed);
 for(int n:{0,1,2,63,64,65,127,128,129,4095,4096,4097}){
  std::string bits(n,'0');std::set<int> oracle;
  for(int i=0;i<n;++i)if(rng()%3==0){bits[i]='1';oracle.insert(i);}
  blueberry::FastSet set(bits);
  for(step=0;step<1000;++step){
   int x=static_cast<int>(rng()%(n+3))-1;
   if(x>=0&&x<n){if(rng()%2)check(set.insert(x)==oracle.insert(x).second,"fast insert");else check(set.erase(x)==(oracle.erase(x)!=0),"fast erase");check(set.contains(x)==oracle.contains(x),"fast contains");}
   auto next=oracle.lower_bound(x);auto prev=oracle.upper_bound(x);
   check(set.next(x)==(next==oracle.end()?n:*next),"fast next");
   check(set.prev(x)==(prev==oracle.begin()?-1:*std::prev(prev)),"fast prev");
   check(set.size()==static_cast<int>(oracle.size())&&set.universe_size()==n&&set.empty()==oracle.empty(),"fast size");
  }
  auto copy=set;auto moved=std::move(copy);check(copy.size()==0&&copy.universe_size()==0,"fast move");set.clear();check(set.empty()&&set.next(0)==n&&set.prev(n)==-1,"fast clear");set=moved;check(set.size()==moved.size(),"fast copy");
 }
 for(int n:{0,1,2,3,7,16,17,63,64,65,127}){
  std::vector<long long>a(n);for(auto&x:a)x=rng()%mod;Dual tree(a);
  for(step=0;step<600;++step){
   int l=rng()%(n+1),r=rng()%(n+1);if(l>r)std::swap(l,r);F f(rng()%mod,rng()%mod);
   if(rng()%3||n==0){tree.apply(l,r,f);for(int i=l;i<r;++i)a[i]=mapping(f,a[i]);}
   else{int p=rng()%n;if(rng()%2){long long x=rng()%mod;tree.set(p,x);a[p]=x;}else{tree.apply(p,f);a[p]=mapping(f,a[p]);}}
   if(n){int p=rng()%n;check(tree.get(p)==a[p],"dual get");}
   if(step%20==0)check(tree.get_all()==a,"dual get_all");
  }
  auto copied=tree;auto moved=std::move(copied);check(copied.size()==0&&copied.get_all().empty(),"dual moved empty");check(moved.get_all()==a,"dual copied lazy");
 }
 for(int initial=0;initial<65;++initial){
  std::vector<int>a(initial);for(auto&x:a)x=static_cast<int>(rng()%101)-50;
  blueberry::DoubleEndedPriorityQueue<int>heap(a);std::multiset<int>oracle(a.begin(),a.end());
  for(step=0;step<1500;++step){
   int action=rng()%3;if(oracle.empty()||action==0){int x=static_cast<int>(rng()%101)-50;heap.push(x);oracle.insert(x);}
   else if(action==1){heap.pop_min();oracle.erase(oracle.begin());}else{heap.pop_max();oracle.erase(std::prev(oracle.end()));}
   check(heap.size()==oracle.size()&&heap.empty()==oracle.empty(),"depq size");
   if(!oracle.empty())check(heap.min()==*oracle.begin()&&heap.max()==*oracle.rbegin(),"depq extrema");
  }
  auto copy=heap;auto moved=std::move(copy);check(copy.empty()&&moved.size()==heap.size(),"depq move");
 }
 blueberry::DoubleEndedPriorityQueue<int,std::greater<int>>reverse(std::vector<int>{1,9,3,9});check(reverse.min()==9&&reverse.max()==1,"reverse comparator");reverse.pop_min();reverse.pop_max();check(reverse.min()==9&&reverse.max()==3,"reverse deletion");
 std::vector<blueberry::SkewHeap<int>>heaps(12);std::vector<std::multiset<int>>sets(12);
 for(step=0;step<12000;++step){int a=rng()%12,b=rng()%12;switch(rng()%4){
  case 0:{int x=static_cast<int>(rng()%1001)-500;heaps[a].push(x);sets[a].insert(x);break;}
  case 1:if(!sets[a].empty()){check(heaps[a].top()==*sets[a].begin(),"skew top before pop");heaps[a].pop();sets[a].erase(sets[a].begin());}break;
  case 2:heaps[a].meld(heaps[b]);if(a!=b){sets[a].insert(sets[b].begin(),sets[b].end());sets[b].clear();}break;
  default:if(a!=b){heaps[a]=std::move(heaps[b]);sets[a]=std::move(sets[b]);sets[b].clear();}break;
 }
 for(int i=0;i<12;++i){check(heaps[i].size()==sets[i].size(),"skew size");if(!sets[i].empty())check(heaps[i].top()==*sets[i].begin(),"skew top");}}
 blueberry::SkewHeap<int>deep;for(int i=0;i<100000;++i)deep.push(i);deep.clear();check(deep.empty(),"skew iterative clear");
}

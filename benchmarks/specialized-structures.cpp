// Same deterministic inputs for each pair; see specialized-structures-comparison.md.
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <queue>
#include <random>
#include <set>
#include <string>
#include <vector>
#include <atcoder/lazysegtree>
#include "blueberry/data-structure/fast-set.hpp"
#include "blueberry/data-structure/dual-segment-tree.hpp"
#include "blueberry/data-structure/double-ended-priority-queue.hpp"
#include "blueberry/data-structure/skew-heap.hpp"
#include "blueberry/data-structure/slope-trick.hpp"
#include "blueberry/data-structure/splay-slope-trick.hpp"
using U = std::uint64_t;
struct F { U a,b; };
U map_value(F f,U x){return f.a*x+f.b;}
F compose(F f,F g){return {f.a*g.a,f.a*g.b+f.b};}
F identity(){return {1,0};}
struct S{U sum,count;};
S op(S a,S b){return {a.sum+b.sum,a.count+b.count};}
S e(){return {0,0};}
S mapping(F f,S x){return {f.a*x.sum+f.b*x.count,x.count};}
int main(int argc,char** argv){
 if(argc!=4)return 1;
 std::string group=argv[1],variant=argv[2]; int n=std::stoi(argv[3]);
 const int q=100000;
 std::mt19937 rng(20260919);
 std::vector<unsigned> input(4*q);
 U hash=1469598103934665603ULL;
 for(auto& x:input){x=rng();hash=(hash^x)*1099511628211ULL;}
 U checksum=0;
 auto start=std::chrono::steady_clock::now();
 if(group=="fast"){
  if(variant=="blueberry"){
   blueberry::FastSet tree(n);
   for(int i=0;i<q;++i){int x=input[4*i]%n,type=input[4*i+1]%3;
    if(type==0)tree.insert(x);else if(type==1)tree.erase(x);else checksum+=tree.next(x);
   }
  }else{
   std::set<int> tree;
   for(int i=0;i<q;++i){int x=input[4*i]%n,type=input[4*i+1]%3;
    if(type==0)tree.insert(x);else if(type==1)tree.erase(x);else{auto it=tree.lower_bound(x);checksum+=it==tree.end()?n:*it;}
   }
  }
 }else if(group=="dual"){
  // Construct only the measured candidate; unsigned arithmetic is modulo 2^64.
  auto run=[&](auto& tree,auto get){for(int i=0;i<q;++i){int l=input[4*i]%n,r=input[4*i+1]%n;if(l>r)std::swap(l,r);
   if(i%2)checksum+=get(tree,l);else tree.apply(l,r+1,F{input[4*i+2],input[4*i+3]});}};
  if(variant=="blueberry"){blueberry::DualSegmentTree<U,F,map_value,compose,identity> tree(n,0);run(tree,[](auto& t,int p){return t.get(p);});}
  else{atcoder::lazy_segtree<S,op,e,F,mapping,compose,identity> tree(std::vector<S>(n,{0,1}));run(tree,[](auto& t,int p){return t.get(p).sum;});}
 }else if(group=="depq"){
  if(variant=="blueberry"){blueberry::DoubleEndedPriorityQueue<unsigned> tree;for(int i=0;i<q;++i){if(i<n||input[4*i]%3==0||tree.empty())tree.push(input[4*i+1]);else if(input[4*i]&1){checksum+=tree.min();tree.pop_min();}else{checksum+=tree.max();tree.pop_max();}}}
  else{std::multiset<unsigned> tree;for(int i=0;i<q;++i){if(i<n||input[4*i]%3==0||tree.empty())tree.insert(input[4*i+1]);else{auto it=(input[4*i]&1)?tree.begin():std::prev(tree.end());checksum+=*it;tree.erase(it);}}}
 }else if(group=="meld"){
  if(variant=="blueberry"){std::vector<blueberry::SkewHeap<unsigned>> heaps(32);for(int i=0;i<q;++i){int a=input[4*i]%32,b=input[4*i+1]%32;heaps[a].push(input[4*i+2]);if(i%32==0)heaps[a].meld(heaps[b]);if(i%2&&!heaps[a].empty()){checksum+=heaps[a].top();heaps[a].pop();}}for(auto& h:heaps)while(!h.empty()){checksum+=h.top();h.pop();}}
  else{std::vector<std::priority_queue<unsigned,std::vector<unsigned>,std::greater<unsigned>>> heaps(32);for(int i=0;i<q;++i){int a=input[4*i]%32,b=input[4*i+1]%32;heaps[a].push(input[4*i+2]);if(i%32==0&&a!=b)while(!heaps[b].empty()){heaps[a].push(heaps[b].top());heaps[b].pop();}if(i%2&&!heaps[a].empty()){checksum+=heaps[a].top();heaps[a].pop();}}for(auto& h:heaps)while(!h.empty()){checksum+=h.top();h.pop();}}
 }else if(group=="slope"){
  auto run=[&](auto& f,auto minimum){for(int i=0;i<q;++i){f.add_abs(static_cast<long long>(input[4*i]%n));if(i%7==0)f.shift(1);checksum+=static_cast<U>(minimum(f));}};
  if(variant=="blueberry"){blueberry::SlopeTrick<> f;run(f,[](auto& t){return t.min();});}
  else{blueberry::SplaySlopeTrick<> f(-1000000,1000000);run(f,[](auto& t){return *t.min();});}
 }else return 1;
 auto stop=std::chrono::steady_clock::now();
 long long rss=0;std::ifstream status("/proc/self/status");std::string line;
 while(std::getline(status,line))if(line.rfind("VmHWM:",0)==0)rss=std::stoll(line.substr(6));
 std::cout<<"{\"milliseconds\":"<<std::chrono::duration<double,std::milli>(stop-start).count()<<",\"checksum\":"<<checksum<<",\"input_hash\":"<<hash<<",\"peak_rss_kib\":"<<rss<<"}\n";
}

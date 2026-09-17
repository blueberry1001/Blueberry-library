#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "blueberry/data-structure/disjoint-sparse-table.hpp"
#include "blueberry/math/longest-increasing-subsequence.hpp"
#include "blueberry/graph/cartesian-tree.hpp"
#include "blueberry/string/prefix-function.hpp"
long long sink;
template<class F> void bench(const char* name,F f) {
  std::cout << name;
  for(int run=0;run<3;++run){auto begin=std::chrono::steady_clock::now();sink += f();std::cout << ',' << std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count();}
  std::cout << '\n';
}
int recursive(const std::vector<int>& a,int l,int r){if(l==r)return 0;int m=std::min_element(a.begin()+l,a.begin()+r)-a.begin();return m+recursive(a,l,m)+recursive(a,m+1,r);}
int main(){
 std::mt19937 rng(20260918);const int n=4000;std::vector<int>a(n);for(int&x:a)x=rng()%10000;
 std::cout << "seed=20260918 n=4000 runs=3 unit=ms\n";
 bench("lis_binary",[&]{return blueberry::longest_increasing_subsequence(a).size();});
 bench("lis_quadratic",[&]{std::vector<int>d(n,1);for(int i=0;i<n;++i)for(int j=0;j<i;++j)if(a[j]<a[i])d[i]=std::max(d[i],d[j]+1);return *std::max_element(d.begin(),d.end());});
 bench("cartesian_stack",[&]{return blueberry::cartesian_tree(a)[0];});
 bench("cartesian_recursive_min",[&]{return recursive(a,0,n);});
 std::string text(n,'a'),pattern(n/2,'a');pattern.back()='b';
 bench("kmp_scan",[&]{return blueberry::kmp_search(text,pattern).size();});
 bench("kmp_naive",[&]{int count=0;for(int i=0;i+(int)pattern.size()<=n;++i){std::size_t j=0;while(j<pattern.size()&&text[i+j]==pattern[j])++j;count+=j==pattern.size();}return count;});
 std::vector<long long> v(a.begin(),a.end());
 bench("dst_build_queries",[&]{blueberry::DisjointSparseTable t(v,[](long long x,long long y){return x+y;});long long sum=0;for(int i=0;i<n;++i)sum+=t.prod(i,n);return sum;});
 bench("segment_tree_build_queries",[&]{std::vector<long long> t(2*n);std::copy(v.begin(),v.end(),t.begin()+n);for(int i=n-1;i>0;--i)t[i]=t[2*i]+t[2*i+1];long long sum=0;for(int i=0;i<n;++i)for(int l=i+n,r=2*n;l<r;l/=2,r/=2){if(l&1)sum+=t[l++];if(r&1)sum+=t[--r];}return sum;});
 std::cout << "checksum=" << sink << '\n';
}

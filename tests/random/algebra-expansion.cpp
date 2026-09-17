#include "blueberry/math/bitwise-convolution.hpp"
#include "blueberry/math/subset-convolution.hpp"
#include "blueberry/math/matrix.hpp"
#include <atcoder/modint>
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <vector>

using Mint = atcoder::modint998244353;
using Small = atcoder::static_modint<3>;
unsigned long long seed;
int trial;
void check(bool ok, const char* what) {
  if (!ok) { std::cerr << "seed=" << seed << " trial=" << trial << " failure=" << what << '\n'; std::exit(1); }
}
template<class T>
std::vector<T> product(const blueberry::Matrix<T>& a, const std::vector<T>& x) {
  std::vector<T> result(a.rows());
  for (int i=0;i<a.rows();++i) for(int j=0;j<a.cols();++j) result[i]+=a(i,j)*x[j];
  return result;
}
int main(int argc,char** argv) {
  seed=argc>1?std::strtoull(argv[1],nullptr,10):1; std::mt19937_64 rng(seed);
  check(blueberry::bitwise_convolution_and<Mint>({},{}).empty(),"empty and");
  check(blueberry::bitwise_convolution_or<Mint>({},{}).empty(),"empty or");
  check(blueberry::bitwise_convolution_xor<Mint>({},{}).empty(),"empty xor");
  check(blueberry::subset_convolution<Mint>({},{}).empty(),"empty subset");
  std::vector<long long> signed_a{-2,3,1,-4},signed_b{5,-1,2,7};
  std::vector<long long> signed_and(4),signed_or(4),signed_subset(4);
  for(int i=0;i<4;++i)for(int j=0;j<4;++j){
    signed_and[i&j]+=signed_a[i]*signed_b[j];signed_or[i|j]+=signed_a[i]*signed_b[j];
    if(!(i&j))signed_subset[i|j]+=signed_a[i]*signed_b[j];
  }
  check(blueberry::bitwise_convolution_and(signed_a,signed_b)==signed_and,"signed ring and");
  check(blueberry::bitwise_convolution_or(signed_a,signed_b)==signed_or,"signed ring or");
  check(blueberry::subset_convolution(signed_a,signed_b)==signed_subset,"signed ring subset");
  check(blueberry::bitwise_convolution_xor(std::vector<Small>{1,2},std::vector<Small>{2,1})==std::vector<Small>({1,2}),"xor characteristic three");
  using Binary=atcoder::static_modint<2>;
  blueberry::Matrix<Binary> binary(2,2);
  binary(0,1)=1;binary(1,0)=1;binary(1,1)=1;
  check(binary.det()==Binary(1)&&binary.rank()==2&&binary.inv().has_value(),"characteristic two field");
  for(trial=0;trial<90;++trial) {
    int n=1<<(rng()%7); std::vector<Mint>a(n),b(n),and_result(n),or_result(n),xor_result(n),subset(n);
    for(int i=0;i<n;++i){a[i]=rng()%100;b[i]=rng()%100;}
    for(int i=0;i<n;++i)for(int j=0;j<n;++j){
      and_result[i&j]+=a[i]*b[j]; or_result[i|j]+=a[i]*b[j]; xor_result[i^j]+=a[i]*b[j];
      if(!(i&j))subset[i|j]+=a[i]*b[j];
    }
    check(blueberry::bitwise_convolution_and(a,b)==and_result,"and brute");
    check(blueberry::bitwise_convolution_or(a,b)==or_result,"or brute");
    check(blueberry::bitwise_convolution_xor(a,b)==xor_result,"xor brute");
    check(blueberry::subset_convolution(a,b)==subset,"subset brute");
  }
  blueberry::Matrix<Small> empty;
  check(empty.rows()==0&&empty.cols()==0&&empty.rank()==0&&empty.det()==Small(1),"0x0");
  check(empty.inv().has_value()&&empty.solve({})->basis.empty(),"0x0 inverse/solve");
  blueberry::Matrix<Small> zero_rows(0,3),zero_cols(3,0);
  check(zero_rows.solve({})->basis.size()==3,"0xm free variables");
  check(zero_cols.solve(std::vector<Small>(3)).has_value(),"nx0 consistent");
  check(!zero_cols.solve({0,1,0}),"nx0 inconsistent");
  for(trial=0;trial<140;++trial) {
    const int n=rng()%5,m=rng()%5;
    blueberry::Matrix<Small>a(n,m); std::vector<Small>b(n);
    for(int i=0;i<n;++i){b[i]=rng()%3;for(int j=0;j<m;++j)a(i,j)=rng()%3;}
    int combinations=1;for(int j=0;j<m;++j)combinations*=3;
    std::set<std::vector<int>> image,solutions;
    for(int code=0;code<combinations;++code){
      int t=code;std::vector<Small>x(m);std::vector<int> raw(m);
      for(int j=0;j<m;++j){raw[j]=t%3;x[j]=t%3;t/=3;}
      auto y=product(a,x);std::vector<int> yr(n);for(int i=0;i<n;++i)yr[i]=y[i].val();
      image.insert(yr);if(y==b)solutions.insert(raw);
    }
    int expected_rank=0;for(int s=image.size();s>1;s/=3)++expected_rank;
    check(a.rank()==expected_rank,"rank by exhaustive image cardinality");
    auto solution=a.solve(b);
    check(bool(solution)==!solutions.empty(),"solve consistency exhaustive");
    if(solution){
      check(product(a,solution->particular)==b,"particular");
      check(solution->basis.size()==static_cast<unsigned>(m-expected_rank),"nullity");
      std::set<std::vector<int>> generated;
      int count=1;for(auto&v:solution->basis){check(product(a,v)==std::vector<Small>(n),"kernel");count*=3;}
      for(int code=0;code<count;++code){
        int t=code;auto x=solution->particular;
        for(auto&v:solution->basis){for(int j=0;j<m;++j)x[j]+=Small(t%3)*v[j];t/=3;}
        std::vector<int>raw(m);for(int j=0;j<m;++j)raw[j]=x[j].val();generated.insert(raw);
      }
      check(generated==solutions,"affine space completeness and independence");
    }
    if(n==m){
      std::vector<int>p(n);std::iota(p.begin(),p.end(),0);Small determinant=0;
      do{Small term=1;int inversions=0;for(int i=0;i<n;++i){term*=a(i,p[i]);for(int j=0;j<i;++j)inversions+=p[j]>p[i];}determinant+=(inversions%2?-term:term);}while(std::next_permutation(p.begin(),p.end()));
      check(a.det()==determinant,"determinant by permutation expansion");
      auto inverse=a.inv();check(bool(inverse)==(determinant!=Small(0)),"inverse singular");
      if(inverse)for(int i=0;i<n;++i)for(int j=0;j<n;++j){Small s=0;for(int k=0;k<n;++k)s+=a(i,k)*(*inverse)(k,j);check(s==Small(i==j),"inverse product");}
    }
    auto copy=a; blueberry::Matrix<Small> assigned;assigned=a;
    if(n&&m){copy(0,0)+=1;check(copy(0,0)!=a(0,0),"copy independent");}
    auto moved=std::move(assigned);check(assigned.rows()==0&&assigned.cols()==0,"moved empty");
    assigned=std::move(moved);check(moved.rows()==0&&moved.cols()==0,"move assign empty");
    check(assigned.rows()==n&&assigned.cols()==m,"move shape");
    const auto&constant=a;if(n&&m)check(constant(0,0)==a(0,0),"const accessor");
  }
  std::cout<<"PASS algebra-expansion seed="<<seed<<'\n';
}

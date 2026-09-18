// Independently written candidate algorithms; no external implementation copied.
#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>
#include <atcoder/modint>
#include "blueberry/math/polynomial-interpolation.hpp"
#include "blueberry/math/sample-point-shift.hpp"
using M = atcoder::modint998244353;
using V = std::vector<M>;
M horner(const V& f, M x) { M v = 0; for (auto i=f.rbegin(); i!=f.rend(); ++i) v=v*x+*i; return v; }
V direct_eval(const V& f, const V& x) { V y; for (auto v:x) y.push_back(horner(f,v)); return y; }
V direct_interpolate(const V& x, const V& y) {
  int n=x.size(); V product(n+1); product[0]=1;
  for(int i=0;i<n;++i) for(int j=i+1;j>=0;--j) product[j]=(j?product[j-1]:M(0))-x[i]*product[j];
  V result(n), q(n);
  for(int i=0;i<n;++i) {
    q[n-1]=product[n]; for(int j=n-2;j>=0;--j) q[j]=product[j+1]+x[i]*q[j+1];
    M w=y[i]/horner(q,x[i]); for(int j=0;j<n;++j) result[j]+=q[j]*w;
  }
  return result;
}
template<class F> void run(const char* name,int n,const V& expected,F f) {
  for(int r=0;r<3;++r) {
    auto begin=std::chrono::steady_clock::now(); V result=f();
    auto end=std::chrono::steady_clock::now();
    if(result!=expected) { std::cerr<<"mismatch "<<name; std::exit(1); }
    std::cout<<name<<','<<n<<','<<r<<','<<std::chrono::duration<double,std::milli>(end-begin).count()<<'\n';
  }
}
int main() {
  std::cout<<"candidate,n,run,milliseconds\n"; std::mt19937 rng(1729);
  for(int n:{512,2048}) {
    V f(n),x(n),shiftx(n); for(auto& a:f)a=rng();
    for(int i=0;i<n;++i) {x[i]=i;shiftx[i]=i+n;}
    auto y=direct_eval(f,x), shifted=direct_eval(f,shiftx);
    run("evaluation_horner",n,y,[&]{return direct_eval(f,x);});
    run("evaluation_remainder_tree",n,y,[&]{return blueberry::multipoint_evaluation(f,x);});
    run("interpolation_quadratic",n,f,[&]{return direct_interpolate(x,y);});
    run("interpolation_product_tree",n,f,[&]{return blueberry::polynomial_interpolation(x,y);});
    run("shift_general_interpolation_evaluation",n,shifted,[&]{return blueberry::multipoint_evaluation(blueberry::polynomial_interpolation(x,y),shiftx);});
    run("shift_convolution",n,shifted,[&]{return blueberry::sample_point_shift(y,M(n),n);});
  }
}

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>
#define blueberry original
#include "baseline.hpp"
#undef blueberry
#define blueberry candidate
#include "candidate.hpp"
#undef blueberry

struct Matrix {
  int a[4];
  Matrix(int x=0):a{x%17,0,0,x%17}{}
  Matrix(int a0,int a1,int a2,int a3):a{a0,a1,a2,a3}{}
  friend bool operator==(const Matrix& x,const Matrix& y) { for(int i=0;i<4;++i) if(x.a[i]!=y.a[i]) return false; return true; }
  Matrix& operator+=(const Matrix& x){for(int i=0;i<4;++i)a[i]=(a[i]+x.a[i])%17;return *this;}
  Matrix& operator-=(const Matrix& x){for(int i=0;i<4;++i)a[i]=(a[i]-x.a[i]+17)%17;return *this;}
  Matrix& operator*=(const Matrix& x){auto t=*this;for(int i=0;i<2;++i)for(int j=0;j<2;++j)a[2*i+j]=(t.a[2*i]*x.a[j]+t.a[2*i+1]*x.a[2+j])%17;return *this;}
  friend Matrix operator+(Matrix x,const Matrix& y){return x+=y;}
  friend Matrix operator-(Matrix x,const Matrix& y){return x-=y;}
  friend Matrix operator/(const Matrix& x,const Matrix& y){int z=1;while((z*y.a[0])%17!=1)++z;return Matrix((x.a[0]*z)%17);}
};

struct Expression { int x; };
struct AssignOnly {
  int x;
  explicit AssignOnly(int value=0):x(value%17){}
  AssignOnly(const AssignOnly&)=default;
  AssignOnly& operator=(const AssignOnly&)=delete;
  AssignOnly& operator=(Expression v){x=v.x;return *this;}
  AssignOnly& operator+=(const AssignOnly& y){x=(x+y.x)%17;return *this;}
  AssignOnly& operator-=(const AssignOnly& y){x=(x-y.x+17)%17;return *this;}
  AssignOnly& operator*=(const AssignOnly& y){x=(x*y.x)%17;return *this;}
  friend Expression operator+(const AssignOnly& a,const AssignOnly& b){return{(a.x+b.x)%17};}
  friend Expression operator-(const AssignOnly& a,const AssignOnly& b){return{(a.x-b.x+17)%17};}
  friend AssignOnly operator/(const AssignOnly& a,const AssignOnly& b){int z=1;while(z*b.x%17!=1)++z;return AssignOnly(a.x*z%17);}
};

int main(){
 std::mt19937 rng(54321);
 for(int n : {0,1,2,4,8,16}) for(int iter=0;iter<500;++iter){
  std::vector<Matrix>a,b;for(int i=0;i<n;++i){a.emplace_back(rng()%17,rng()%17,rng()%17,rng()%17);b.emplace_back(rng()%17,rng()%17,rng()%17,rng()%17);}
  for(int kind=0;kind<3;++kind){std::vector<Matrix> expected(n);for(int i=0;i<n;++i)for(int j=0;j<n;++j){int k=kind==0?i&j:kind==1?i|j:i^j;Matrix x=a[i];x*=b[j];expected[k]+=x;}
   auto actual=kind==0?candidate::bitwise_convolution_and(a,b):kind==1?candidate::bitwise_convolution_or(a,b):candidate::bitwise_convolution_xor(a,b);
   auto old=kind==0?original::bitwise_convolution_and(a,b):kind==1?original::bitwise_convolution_or(a,b):original::bitwise_convolution_xor(a,b);
   if(actual!=expected||old!=expected){std::printf("MISMATCH %d %d %d\n",n,iter,kind);return 1;}
  }
  std::vector<AssignOnly>u,v;for(int i=0;i<n;++i){u.emplace_back(rng()%17);v.emplace_back(rng()%17);}
  for(int kind=0;kind<3;++kind){auto old=kind==0?original::bitwise_convolution_and(u,v):kind==1?original::bitwise_convolution_or(u,v):original::bitwise_convolution_xor(u,v);auto cur=kind==0?candidate::bitwise_convolution_and(u,v):kind==1?candidate::bitwise_convolution_or(u,v):candidate::bitwise_convolution_xor(u,v);for(int i=0;i<n;++i)if(old[i].x!=cur[i].x)return 2;}
 }
 std::puts("PASS 3000 cases, noncommutative matrices and assign-only expression type");
}

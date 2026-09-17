#include <atcoder/modint>
#include <algorithm>
#include <bit>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "blueberry/math/bitwise-convolution.hpp"
#include "blueberry/math/subset-convolution.hpp"
#include "blueberry/math/matrix.hpp"
using Mint = atcoder::modint998244353;

// Independent branch-per-mask textbook transforms for a comparable reference.
std::vector<Mint> reference_bitwise(std::vector<Mint> a, std::vector<Mint> b, bool xor_mode) {
  const int n = a.size();
  auto transform = [&](std::vector<Mint>& v, bool inverse) {
    for (int bit = 1; bit < n; bit <<= 1) for (int mask = 0; mask < n; ++mask)
      if (!(mask & bit)) {
        if (xor_mode) { Mint x=v[mask],y=v[mask|bit];v[mask]=x+y;v[mask|bit]=x-y; }
        else if(inverse) v[mask]-=v[mask|bit]; else v[mask]+=v[mask|bit];
      }
  };
  transform(a,false);transform(b,false);
  for(int i=0;i<n;++i)a[i]*=b[i];
  transform(a,true);
  if(xor_mode){Mint inv=Mint(n).inv();for(auto&x:a)x*=inv;}
  return a;
}
std::vector<Mint> reference_subset(const std::vector<Mint>& a,const std::vector<Mint>& b) {
  int n=a.size(),bits=std::countr_zero(static_cast<unsigned>(n));
  std::vector<std::vector<Mint>> f(bits+1,std::vector<Mint>(n)),g=f,h=f;
  for(int mask=0;mask<n;++mask){int k=std::popcount(static_cast<unsigned>(mask));f[k][mask]=a[mask];g[k][mask]=b[mask];}
  for(int k=0;k<=bits;++k)for(int bit=1;bit<n;bit<<=1)for(int mask=0;mask<n;++mask)if(mask&bit){f[k][mask]+=f[k][mask^bit];g[k][mask]+=g[k][mask^bit];}
  for(int k=0;k<=bits;++k)for(int j=0;j<=k;++j)for(int mask=0;mask<n;++mask)h[k][mask]+=f[j][mask]*g[k-j][mask];
  for(int k=0;k<=bits;++k)for(int bit=1;bit<n;bit<<=1)for(int mask=0;mask<n;++mask)if(mask&bit)h[k][mask]-=h[k][mask^bit];
  std::vector<Mint> result(n);
  for(int mask=0;mask<n;++mask)result[mask]=h[std::popcount(static_cast<unsigned>(mask))][mask];
  return result;
}
Mint reference_det(std::vector<std::vector<Mint>> a) {
  const int n=a.size();Mint answer=1;
  for(int col=0;col<n;++col){
    int pivot=col;while(pivot<n&&a[pivot][col]==Mint(0))++pivot;
    if(pivot==n)return 0;
    if(pivot!=col){std::swap(a[pivot],a[col]);answer=-answer;}
    Mint diagonal=a[col][col],inv=diagonal.inv();answer*=diagonal;
    for(int j=col;j<n;++j)a[col][j]*=inv;
    for(int i=col+1;i<n;++i){Mint factor=a[i][col];for(int j=col;j<n;++j)a[i][j]-=factor*a[col][j];}
  }
  return answer;
}
int main(int argc,char**argv){
  const std::string family=argc>1?argv[1]:"and",mode=argc>2?argv[2]:"modern";
  const int n=family=="subset"?1<<18:family=="matrix"?500:1<<18;
  std::mt19937 rng(1729);std::vector<Mint>a(family=="matrix"?n*n:n),b(a.size());
  for(auto&x:a)x=rng();for(auto&x:b)x=rng();
  auto start=std::chrono::steady_clock::now();std::uint64_t checksum=0;
  if(family=="matrix"){
    if(mode=="modern"){
      blueberry::Matrix<Mint> matrix(n,n);for(int i=0;i<n;++i)for(int j=0;j<n;++j)matrix(i,j)=a[i*n+j];
      checksum=matrix.det().val();
    }else{
      std::vector<std::vector<Mint>> matrix(n,std::vector<Mint>(n));for(int i=0;i<n;++i)for(int j=0;j<n;++j)matrix[i][j]=a[i*n+j];
      checksum=reference_det(matrix).val();
    }
  }else{
    std::vector<Mint> c;
    if(family=="subset")c=mode=="modern"?blueberry::subset_convolution(a,b):reference_subset(a,b);
    else if(mode=="reference")c=reference_bitwise(a,b,family=="xor");
    else if(family=="xor")c=blueberry::bitwise_convolution_xor(a,b);
    else c=blueberry::bitwise_convolution_and(a,b);
    for(auto x:c)checksum=checksum*1000003+x.val();
  }
  double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
  std::cout<<"{\"family\":\""<<family<<"\",\"mode\":\""<<mode<<"\",\"n\":"<<n<<",\"ms\":"<<ms<<",\"checksum\":"<<checksum<<"}\n";
}

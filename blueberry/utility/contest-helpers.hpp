#pragma once
#include "blueberry/utility/internal/contest-helpers-base.hpp"
#include "blueberry/math/factorize.hpp"
namespace blueberry{
inline std::vector<std::uint64_t>divisors(std::uint64_t n){
 const auto factors=factorize(n);
 std::vector<std::uint64_t>result{1};
 for(std::size_t i=0;i<factors.size();){
  const auto prime=factors[i];
  const auto count=result.size();
  std::uint64_t power=1;
  do{
   power*=prime;
   for(std::size_t j=0;j<count;++j)result.push_back(result[j]*power);
   ++i;
  }while(i<factors.size()&&factors[i]==prime);
 }
 std::sort(result.begin(),result.end());
 return result;
}
}

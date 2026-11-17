#include <cstdio>
#include <numeric>
#include <vector>
#include "candidate.hpp"
int main(){
  long long cases=0, ph_bases=0, ph_cases=0, absent=0;
  for(int m:{3063,3027,3103,4076}) {
    int selected=0;
    for(int a=2;a<m && selected<8;++a){
      if(std::gcd(a,m)!=1)continue;
      std::vector<int> answer(m,-1); int x=1,step=0;
      while(answer[x]<0){answer[x]=step++;x=(long long)x*a%m;}
      if(step<=512)continue;
      ++selected;++ph_bases;
      for(int b=0;b<m;++b){
        auto got=blueberry::discrete_log(a,b,m);int expected=answer[b];++cases;
        if(expected>=512)++ph_cases;if(expected<0)++absent;
        if((expected<0)!=(!got)||(got && *got!=expected)){std::printf("FAIL a=%d b=%d m=%d expected=%d actual=%lld\n",a,b,m,expected,got?*got:-1LL);return 1;}
      }
    }
    if(!selected){std::printf("FAIL no base m=%d\n",m);return 1;}
  }
  std::printf("PASS %lld cases %lld long-order bases %lld PH-solutions %lld no-solutions\n",cases,ph_bases,ph_cases,absent);
}

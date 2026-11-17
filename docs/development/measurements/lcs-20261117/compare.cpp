#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>
#include <string_view>
#include <vector>
#define blueberry baseline
#include "baseline.hpp"
#undef blueberry
#define blueberry candidate
#include "candidate.hpp"
#undef blueberry

int brute(const std::string& s,const std::string& t) {
  int answer=0;
  for (int i=0;i<(int)s.size();++i) for(int j=0;j<(int)t.size();++j) {
    int k=0;while(i+k<(int)s.size()&&j+k<(int)t.size()&&s[i+k]==t[j+k])++k;
    answer=std::max(answer,k);
  }
  return answer;
}
bool check(const std::string& s,const std::string& t,std::array<int,4> a,int length) {
  return a[0]>=0&&a[0]<=a[1]&&a[1]<=(int)s.size()&&a[2]>=0&&a[2]<=a[3]&&a[3]<=(int)t.size()&&a[1]-a[0]==length&&a[3]-a[2]==length&&s.compare(a[0],length,t,a[2],length)==0;
}
int main(int argc,char** argv) {
  std::mt19937 rng(1337);
  if(argc==1) {
    for(int z=0;z<50000;++z) {
      int n=rng()%25,m=rng()%25,alphabet=1+rng()%256;std::string s(n,'\0'),t(m,'\0');
      for(char& c:s)c=char(rng()%alphabet);for(char& c:t)c=char(rng()%alphabet);
      int expected=brute(s,t);auto a=baseline::longest_common_substring(s,t),b=candidate::longest_common_substring(s,t);
      if(!check(s,t,a,expected)||!check(s,t,b,expected)){std::printf("failed iteration %d\n",z);return 1;}
    }
    std::puts("OK 50000");return 0;
  }
  int ver=std::atoi(argv[1]),n=std::atoi(argv[2]),kind=std::atoi(argv[3]),reps=std::atoi(argv[4]);
  std::string s(n,'a'),t(n,'a');int alphabet=kind==0?26:kind==1?2:256;
  if(kind<3) {for(char& c:s)c=char(rng()%alphabet);for(char& c:t)c=char(rng()%alphabet);}
  if(kind==4)for(int i=0;i<n;++i){s[i]=char('a'+i%7);t[i]=char('a'+(i+3)%7);}
  if(kind==5){for(char& c:s)c=char(rng()%256);t=s.substr(n/2);}
  if(kind==6){for(char& c:s)c=char(rng()%256);t=s;t.back()^=1;}
  int total=0;auto start=std::chrono::steady_clock::now();
  for(int i=0;i<reps;++i){auto a=ver?candidate::longest_common_substring(s,t):baseline::longest_common_substring(s,t);total+=a[1]-a[0];}
  auto end=std::chrono::steady_clock::now();std::printf("%.3f %d\n",std::chrono::duration<double,std::milli>(end-start).count(),total);
}

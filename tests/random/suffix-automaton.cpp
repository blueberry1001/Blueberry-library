#include <cassert>
#include <cstdlib>
#include <iostream>
#include <random>
#include <set>
#include <string>
#include <vector>
#include "blueberry/string/suffix-automaton.hpp"
using namespace std;
int main(int argc,char**argv){
  auto seed=argc>1?strtoull(argv[1],nullptr,10):1;mt19937_64 rng(seed);
  blueberry::SuffixAutomaton<> literal("ababa");assert(literal.size()==5&&literal.count("aba")==2&&literal.contains(""));
  blueberry::SuffixAutomaton<int> integers(vector<int>{-5,7,-5});assert(integers.count(vector<int>{-5})==2);
  for(int trial=0;trial<100;++trial){
    blueberry::SuffixAutomaton<> sam;string text;
    for(int step=0;step<50;++step){
      set<string> substrings;
      for(int l=0;l<(int)text.size();++l)for(int r=l+1;r<=(int)text.size();++r)substrings.insert(text.substr(l,r-l));
      if(sam.distinct_substrings()!=(long long)substrings.size()){cerr<<"seed="<<seed<<" text="<<text<<" expected="<<substrings.size()<<" actual="<<sam.distinct_substrings()<<'\n';return 1;}
      for(int q=0;q<30;++q){string pattern;for(int k=rng()%8;k--;)pattern+=char('a'+rng()%4);int count=0;for(int i=0;i+(int)pattern.size()<=(int)text.size();++i)count+=text.compare(i,pattern.size(),pattern)==0;
        if(sam.count(pattern)!=count||sam.contains(pattern)!=(count!=0)){cerr<<"seed="<<seed<<" text="<<text<<" pattern="<<pattern<<" expected="<<count<<" actual="<<sam.count(pattern)<<'\n';return 1;}
        auto [a,b,len]=sam.longest_common_substring(pattern);int expected=0;
        for(int i=0;i<(int)pattern.size();++i)for(int j=i+1;j<=(int)pattern.size();++j)if(text.find(pattern.substr(i,j-i))!=string::npos)expected=max(expected,j-i);
        assert(len==expected&&text.substr(a,len)==pattern.substr(b,len));
      }
      char c='a'+rng()%3;text+=c;sam.append(c);
    }
  }
}

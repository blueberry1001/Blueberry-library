#define PROBLEM "https://judge.yosupo.jp/problem/ordered_set"
#include <iostream>
#include "blueberry/data-structure/ordered-set.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int n,q; cin>>n>>q;
  blueberry::OrderedSet<int> s;
  for(int i=0;i<n;++i) { int x; cin>>x; s.insert(x); }
  while(q--) {
    int t,x; cin>>t>>x;
    if(t==0) s.insert(x);
    if(t==1) s.erase(x);
    if(t==2) cout<<s.kth(x-1).value_or(-1)<<'\n';
    if(t==3) cout<<s.rank(x)+s.contains(x)<<'\n';
    if(t==4) cout<<s.floor(x).value_or(-1)<<'\n';
    if(t==5) cout<<s.lower_bound(x).value_or(-1)<<'\n';
  }
}

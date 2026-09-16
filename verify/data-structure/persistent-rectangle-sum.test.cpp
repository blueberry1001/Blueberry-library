#define PROBLEM "https://judge.yosupo.jp/problem/rectangle_sum"
#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <vector>
#include "blueberry/data-structure/persistent-segment-tree.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false); cin.tie(nullptr);
  int n,q; cin>>n>>q;
  vector<array<int,3>> points(n);
  vector<int> xs,ys;
  for(auto& p:points) { cin>>p[0]>>p[1]>>p[2]; ys.push_back(p[1]); }
  sort(points.begin(),points.end());
  sort(ys.begin(),ys.end()); ys.erase(unique(ys.begin(),ys.end()),ys.end());
  blueberry::PersistentSegmentTree tree(static_cast<int>(ys.size()),plus<long long>{},0LL);
  for(int i=0;i<n;++i) {
    xs.push_back(points[i][0]);
    int y=static_cast<int>(lower_bound(ys.begin(),ys.end(),points[i][1])-ys.begin());
    tree.apply(i,y,points[i][2]);
  }
  // Each x-prefix is a version; queries use two genuinely historical roots.
  while(q--) {
    int l,d,r,u; cin>>l>>d>>r>>u;
    int a=static_cast<int>(lower_bound(xs.begin(),xs.end(),l)-xs.begin());
    int b=static_cast<int>(lower_bound(xs.begin(),xs.end(),r)-xs.begin());
    int lo=static_cast<int>(lower_bound(ys.begin(),ys.end(),d)-ys.begin());
    int hi=static_cast<int>(lower_bound(ys.begin(),ys.end(),u)-ys.begin());
    cout<<tree.prod(b,lo,hi)-tree.prod(a,lo,hi)<<'\n';
  }
}

#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <vector>
namespace rankfreefast {
template<class Area=long long,class Coord>
Area rectangle_union_area(const std::vector<std::array<Coord,4>>& rectangles) {
  if(rectangles.size()<=2) {
    if(rectangles.empty()) return Area{};
    auto area=[](const auto& a)->Area{return a[0]==a[2] || a[1]==a[3]? Area{}:(Area(a[2])-Area(a[0]))*(Area(a[3])-Area(a[1]));};
    const auto& a=rectangles[0];
    if(rectangles.size()==1) return area(a);
    const auto& b=rectangles[1];
    Coord l=std::max(a[0],b[0]), r=std::min(a[2],b[2]), d=std::max(a[1],b[1]), u=std::min(a[3],b[3]);
    Area overlap{};
    if(l<r && d<u) overlap=(Area(r)-Area(l))*(Area(u)-Area(d));
    return area(a)+(area(b)-overlap);
  }
  struct Event { Coord x; int low,high,delta; };
  struct Endpoint { Coord y; int index; };
  std::size_t count=0;
  for(const auto& a:rectangles) if(!(a[0]==a[2] || a[1]==a[3])) ++count;
  if(!count) return Area{};
  std::vector<Event> events; events.reserve(2*count);
  std::vector<Coord> ys; ys.reserve(2*count);
  {
    std::vector<Endpoint> endpoints; endpoints.reserve(2*count);
    for(const auto& a:rectangles) {
      if(a[0]==a[2]||a[1]==a[3]) continue;
      int low=static_cast<int>(endpoints.size()), high=low+1;
      endpoints.push_back({a[1],low}); endpoints.push_back({a[3],high});
      events.push_back({a[0],low,high,1}); events.push_back({a[2],low,high,-1});
    }
    std::sort(endpoints.begin(),endpoints.end(),[](const auto& a,const auto& b){return a.y<b.y;});
    std::vector<int> rank(endpoints.size());
    for(const auto& e:endpoints) {if(ys.empty() || !(ys.back()==e.y)) ys.push_back(e.y);rank[e.index]=static_cast<int>(ys.size())-1;}
    for(auto& e:events) {e.low=rank[e.low];e.high=rank[e.high];}
  }
  std::sort(events.begin(),events.end(),[](const auto& a,const auto& b){return a.x<b.x;});
  int n=static_cast<int>(ys.size())-1;
  std::vector<int> cover(4*n); std::vector<Area> length(4*n);
  auto update=[&](auto&& self,int p,int l,int r,int ql,int qr,int delta)->void {
    if(ql<=l && r<=qr) cover[p]+=delta;
    else {int m=l+(r-l)/2;if(ql<m)self(self,2*p,l,m,ql,qr,delta);if(m<qr)self(self,2*p+1,m,r,ql,qr,delta);}
    length[p]=cover[p]? Area(ys[r])-Area(ys[l]): r-l==1? Area{}:length[2*p]+length[2*p+1];
  };
  Area answer{}; Coord last=events.front().x;
  for(const auto& e:events) {answer+=(Area(e.x)-Area(last))*length[1];update(update,1,0,n,e.low,e.high,e.delta);last=e.x;}
  return answer;
}
}

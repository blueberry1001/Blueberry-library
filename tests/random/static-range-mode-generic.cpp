#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <type_traits>
#include <vector>
#include "blueberry/data-structure/static-range-mode.hpp"

struct CopyOnly {
  int x;
  explicit CopyOnly(int v): x(v) {}
  CopyOnly(const CopyOnly&) = default;
  CopyOnly& operator=(const CopyOnly&) = default;
  CopyOnly(CopyOnly&&) = delete;
  CopyOnly& operator=(CopyOnly&&) = delete;
  friend bool operator<(const CopyOnly& a, const CopyOnly& b) { return a.x < b.x; }
  friend bool operator==(const CopyOnly& a, const CopyOnly& b) { return a.x == b.x; }
};
static_assert(std::is_copy_constructible_v<CopyOnly> && std::is_copy_assignable_v<CopyOnly>);
static_assert(!std::is_move_constructible_v<CopyOnly> && !std::is_move_assignable_v<CopyOnly>);

struct NoSwap {
  int x;
  explicit NoSwap(int v): x(v) {}
  NoSwap(const NoSwap&) = default;
  NoSwap& operator=(const NoSwap&) = default;
  NoSwap(NoSwap&&) = default;
  NoSwap& operator=(NoSwap&&) = default;
  friend void swap(NoSwap&, NoSwap&) = delete;
  friend bool operator<(const NoSwap& a, const NoSwap& b) { return a.x < b.x; }
  friend bool operator==(const NoSwap& a, const NoSwap& b) { return a.x == b.x; }
};
static_assert(std::is_move_constructible_v<NoSwap> && std::is_move_assignable_v<NoSwap> && !std::is_swappable_v<NoSwap>);

template<class T> bool check(const std::vector<T>& a) {
  std::vector<std::pair<int,int>> q;
  const int n = static_cast<int>(a.size());
  for (int l = 0; l <= n; ++l) for (int r = l; r <= n; ++r) q.emplace_back(l,r);
  std::reverse(q.begin(),q.end());
  const auto ans = blueberry::static_range_mode(a,q);
  if (ans.size()!=q.size()) return false;
  for (std::size_t i=0;i<q.size();++i) {
    auto [l,r]=q[i]; std::map<int,int> freq; int expected=0;
    for(int j=l;j<r;++j) expected=std::max(expected,++freq[a[j].x]);
    auto [idx,count]=ans[i];
    if(count!=expected || (!count && idx!=-1) || (count && (idx<0 || idx>=n || freq[a[idx].x]!=expected))) {
      std::cerr<<"n="<<n<<" l="<<l<<" r="<<r<<" idx="<<idx<<" count="<<count<<" expected="<<expected<<'\n';return false;
    }
  }
  return blueberry::static_range_mode(a,{}).empty();
}
int main(){
  const std::vector<CopyOnly> empty;
  const std::vector<CopyOnly> one{CopyOnly(4)};
  const std::vector<CopyOnly> same{CopyOnly(4),CopyOnly(4),CopyOnly(4)};
  const std::vector<CopyOnly> distinct{CopyOnly(1),CopyOnly(2),CopyOnly(3)};
  const std::vector<CopyOnly> binary{CopyOnly(1),CopyOnly(2),CopyOnly(1),CopyOnly(2)};
  const std::vector<CopyOnly> mixed{CopyOnly(4),CopyOnly(2),CopyOnly(4),CopyOnly(3),CopyOnly(2),CopyOnly(2),CopyOnly(1),CopyOnly(4)};
  const std::vector<NoSwap> movable_no_swap{NoSwap(4),NoSwap(2),NoSwap(4),NoSwap(3),NoSwap(2),NoSwap(2),NoSwap(1),NoSwap(4)};
  return !(check(empty) && check(one) && check(same) && check(distinct) && check(binary) && check(mixed) && check(movable_no_swap));
}

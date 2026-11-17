#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include "blueberry/data-structure/static-range-inversions.hpp"
#include "blueberry/data-structure/static-range-mode.hpp"
#include "blueberry/string/longest-common-substring.hpp"
struct MutableModeKey {
  int value;
  bool operator<(const MutableModeKey& rhs) const { return value < rhs.value; }
  friend bool operator==(MutableModeKey& x, MutableModeKey& y) { return x.value == y.value; }
};
int main(int argc, char** argv) {
  unsigned seed = argc > 1 ? static_cast<unsigned>(std::stoul(argv[1])) : 20260918;
  std::mt19937 rng(seed);
  for (int iteration = 0; iteration < 150; ++iteration) {
    int n = rng() % 40;
    std::vector<int> a(n); for (int& x : a) x = static_cast<int>(rng() % 11) - 5;
    std::vector<std::pair<int,int>> queries;
    for (int l = 0; l <= n; ++l) for (int r = l; r <= n; ++r) queries.emplace_back(l,r);
    std::shuffle(queries.begin(), queries.end(), rng);
    auto inv = blueberry::static_range_inversions(a, queries);
    auto mode = blueberry::static_range_mode(a, queries);
    for (int k = 0; k < static_cast<int>(queries.size()); ++k) {
      auto [l,r] = queries[k]; long long expected = 0; int frequency = 0;
      std::map<int,int> count;
      for (int i = l; i < r; ++i) {
        frequency = std::max(frequency, ++count[a[i]]);
        for (int j = i + 1; j < r; ++j) expected += a[i] > a[j];
      }
      auto [index,f] = mode[k];
      if (inv[k] != expected || f != frequency || (f ? index < 0 || index >= n || count[a[index]] != f : index != -1)) {
        std::cerr << "seed=" << seed << " input="; for (int x:a) std::cerr << x << ',';
        std::cerr << " range=" << l << ',' << r << " expected=" << expected << ',' << frequency << " actual=" << inv[k] << ',' << f << '\n'; return 1;
      }
    }
    std::string s(rng()%40, '\0'), t(rng()%40, '\0');
    for (char& c : s) c = static_cast<char>(rng()%6 == 0 ? 255 : rng()%4);
    for (char& c : t) c = static_cast<char>(rng()%6 == 0 ? 255 : rng()%4);
    int best = 0;
    for (int i=0;i<static_cast<int>(s.size());++i) for(int j=0;j<static_cast<int>(t.size());++j) {
      int k=0; while(i+k<static_cast<int>(s.size()) && j+k<static_cast<int>(t.size()) && s[i+k]==t[j+k]) ++k;
      best=std::max(best,k);
    }
    auto [l,r,x,y] = blueberry::longest_common_substring(s,t);
    if(r-l!=best || y-x!=best || s.substr(l,r-l)!=t.substr(x,y-x)) {
      std::cerr << "seed=" << seed << " iteration=" << iteration << " expected=" << best << " actual=" << r-l << '\n'; return 1;
    }
  }
  {
    std::vector<int> a(80); for (int& x : a) x = static_cast<int>(rng() % 13) - 6;
    for (int groups : {1, 2, 8, 9, 16}) for (int side = 0; side < 2; ++side) {
      std::vector<std::pair<int,int>> qs;
      for (int i = 0; i < 300; ++i) {
        int end = i % groups;
        qs.emplace_back(side ? static_cast<int>(rng() % 60) : end, side ? 80 - end : 20 + static_cast<int>(rng() % 61));
      }
      std::shuffle(qs.begin(), qs.end(), rng);
      auto ans = blueberry::static_range_mode(a, qs);
      for (int j = 0; j < static_cast<int>(qs.size()); ++j) {
        auto [l, r] = qs[j]; std::map<int, int> count; int best = 0;
        for (int i = l; i < r; ++i) best = std::max(best, ++count[a[i]]);
        auto [idx, frequency] = ans[j];
        if (frequency != best || (best ? idx < 0 || idx >= static_cast<int>(a.size()) || count[a[idx]] != best : idx != -1)) {
          std::cerr << "seed=" << seed << " groups=" << groups << " side=" << side << " range=" << l << ',' << r << " expected=" << best << " actual=" << frequency << '\n'; return 1;
        }
      }
    }
    for (auto query : {std::pair{0,80}, std::pair{20,20}, std::pair{10,14}}) {
      std::vector<std::pair<int,int>> qs(100, query);
      auto ans = blueberry::static_range_mode(a, qs);
      std::map<int,int> count; int best = 0;
      for (int i = query.first; i < query.second; ++i) best = std::max(best, ++count[a[i]]);
      for (auto [idx, frequency] : ans)
        if (frequency != best || (best ? idx < 0 || idx >= static_cast<int>(a.size()) || count[a[idx]] != best : idx != -1)) {
          std::cerr << "seed=" << seed << " duplicate range=" << query.first << ',' << query.second << '\n'; return 1;
        }
    }
  }
  {
    std::vector<MutableModeKey> a(80);
    for (auto& x : a) x.value = static_cast<int>(rng() % 13) - 6;
    auto check = [&](const std::vector<std::pair<int,int>>& qs) {
      auto ans = blueberry::static_range_mode(a, qs);
      if (ans.size() != qs.size()) return false;
      for (int j = 0; j < static_cast<int>(qs.size()); ++j) {
        auto [l, r] = qs[j]; std::map<int,int> count; int best = 0;
        for (int i = l; i < r; ++i) best = std::max(best, ++count[a[i].value]);
        auto [idx, frequency] = ans[j];
        if (frequency != best || (best ? idx < 0 || idx >= static_cast<int>(a.size()) || count[a[idx].value] != best : idx != -1)) return false;
      }
      return true;
    };
    std::vector<std::pair<int,int>> varied;
    for (int i = 0; i < 30; ++i) varied.emplace_back(i, 80 - i);
    if (!check({}) || !check({{3,3}}) || !check({{1,40}}) || !check(std::vector<std::pair<int,int>>(20, {2,60})) || !check({{0,80},{1,80},{2,80}}) || !check(varied)) {
      std::cerr << "seed=" << seed << " mutable equality mode\n"; return 1;
    }
  }
}

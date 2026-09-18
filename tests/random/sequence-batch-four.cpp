#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include "blueberry/data-structure/static-range-inversions.hpp"
#include "blueberry/data-structure/static-range-mode.hpp"
#include "blueberry/string/longest-common-substring.hpp"
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
}

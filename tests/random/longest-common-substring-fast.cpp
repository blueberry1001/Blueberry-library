#include <algorithm>
#include <cassert>
#include <random>
#include <string>
#include "blueberry/string/longest-common-substring.hpp"

int main() {
  std::mt19937 gen(7741);
  for (int rep = 0; rep < 30000; ++rep) {
    int n = gen() % 24, m = gen() % 24, alphabet = 1 + gen() % 256;
    std::string s(n, '\0'), t(m, '\0');
    for (char& c : s) c = static_cast<char>(gen() % alphabet);
    for (char& c : t) c = static_cast<char>(gen() % alphabet);
    if (rep % 8 == 0 && n && m) {
      int k = std::min(n, m);
      if (rep % 16 == 0) t.replace(0, k, s, 0, k);
      else t.replace(m - k, k, s, n - k, k);
    }
    int best = 0;
    for (int i = 0; i < n; ++i) for (int j = 0; j < m; ++j) {
      int k = 0;
      while (i + k < n && j + k < m && s[i + k] == t[j + k]) ++k;
      best = std::max(best, k);
    }
    auto [l, r, u, v] = blueberry::longest_common_substring(s, t);
    assert(0 <= l && l <= r && r <= n && 0 <= u && u <= v && v <= m);
    assert(r - l == best && v - u == best && s.compare(l, best, t, u, best) == 0);
  }
}

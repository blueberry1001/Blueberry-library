#pragma once
#include <algorithm>
#include <array>
#include <limits>
#include <string_view>
#include <vector>
#include <atcoder/string>

namespace blueberry {
inline std::array<int, 4> longest_common_substring(std::string_view s, std::string_view t) {
  if (s.empty() || t.empty()) return {0, 0, 0, 0};
  const std::size_t len = std::min(s.size(), t.size());
  if (s.substr(0, len) == t.substr(0, len)) return {0, static_cast<int>(len), 0, static_cast<int>(len)};
  if (s.size() != t.size() && s.substr(s.size() - len) == t.substr(t.size() - len)) {
    const int a = static_cast<int>(s.size() - len), b = static_cast<int>(t.size() - len);
    return {a, a + static_cast<int>(len), b, b + static_cast<int>(len)};
  }
  const int n = static_cast<int>(s.size());
  std::vector<int> text;
  text.reserve(s.size() + t.size() + 1);
  for (unsigned char c : s) text.push_back(c + 1);
  text.push_back(0);
  for (unsigned char c : t) text.push_back(c + 1);
  const auto sa = atcoder::suffix_array(text, 256);
  const int total = static_cast<int>(text.size());
  std::vector<int> rank(total);
  for (int i = 0; i < total; ++i) rank[sa[i]] = i;
  std::array<int, 4> answer{0, 0, 0, 0};
  int best = 0, h = 0;
  for (int i = 0; i < total; ++i) {
    const int r = rank[i];
    if (r == 0) { h = 0; continue; }
    const int j = sa[r - 1];
    while (i + h < total && j + h < total && text[i + h] == text[j + h]) ++h;
    if (h > best && ((i < n && j > n) || (j < n && i > n))) {
      best = h;
      const int a = i < n ? i : j, b = i < n ? j : i;
      answer = {a, a + h, b - n - 1, b - n - 1 + h};
    }
    if (h) --h;
  }
  return answer;
}
}

#pragma once
#include <array>
#include <cassert>
#include <limits>
#include <string_view>
#include <utility>
#include <vector>
#include <atcoder/string>

namespace blueberry {
// Returns {s_begin, s_end, t_begin, t_end}; ties are unspecified.
inline std::array<int, 4> longest_common_substring(std::string_view s,
                                                std::string_view t) {
  assert(s.size() + t.size() < static_cast<std::size_t>(std::numeric_limits<int>::max()));
  if (s.empty() || t.empty()) return {0, 0, 0, 0};
  const int n = static_cast<int>(s.size());
  std::vector<int> text;
  text.reserve(s.size() + t.size() + 1);
  for (unsigned char c : s) text.push_back(c + 1);
  text.push_back(0);
  for (unsigned char c : t) text.push_back(c + 1);
  const auto sa = atcoder::suffix_array(text, 256);
  const auto lcp = atcoder::lcp_array(text, sa);
  std::array<int, 4> answer{0, 0, 0, 0};
  for (int i = 0; i + 1 < static_cast<int>(sa.size()); ++i) {
    int a = sa[i], b = sa[i + 1];
    if (a > b) std::swap(a, b);
    if (a < n && b > n && lcp[i] > answer[1] - answer[0])
      answer = {a, a + lcp[i], b - n - 1, b - n - 1 + lcp[i]};
  }
  return answer;
}
}  // namespace blueberry

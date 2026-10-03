#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <map>
#include <ranges>
#include <tuple>
#include <string_view>
#include <type_traits>
#include <vector>

namespace blueberry {

// Online construction. Occurrence counts are rebuilt lazily after append.
template <class Symbol = char>
class SuffixAutomaton {
  struct State {
    std::map<Symbol, int> next;
    int link = -1, length = 0, first = -1;
    bool clone = false;
  };
  std::vector<State> states_{State{}};
  int last_ = 0, n_ = 0;
  long long distinct_ = 0;
  mutable std::vector<int> occurrences_;
  mutable bool dirty_ = true;

  template <class Range>
  int walk(const Range& pattern) const {
    int state = 0;
    for (const auto& symbol : pattern) {
      auto found = states_[state].next.find(symbol);
      if (found == states_[state].next.end()) return -1;
      state = found->second;
    }
    return state;
  }
  void rebuild_counts() const {
    if (!dirty_) return;
    std::vector<int> lengths(n_ + 1), order(states_.size());
    for (const auto& state : states_) ++lengths[state.length];
    for (int i = 1; i <= n_; ++i) lengths[i] += lengths[i - 1];
    for (int i = 0; i < static_cast<int>(states_.size()); ++i) order[--lengths[states_[i].length]] = i;
    occurrences_.assign(states_.size(), 0);
    for (int i = 1; i < static_cast<int>(states_.size()); ++i) occurrences_[i] = !states_[i].clone;
    for (int i = static_cast<int>(order.size()) - 1; i > 0; --i) {
      int state = order[i];
      occurrences_[states_[state].link] += occurrences_[state];
    }
    dirty_ = false;
  }

 public:
  SuffixAutomaton() = default;
  template <class Range> requires (std::ranges::input_range<const Range> && !std::is_array_v<Range>)
  explicit SuffixAutomaton(const Range& text) { for (const auto& symbol : text) append(symbol); }
  explicit SuffixAutomaton(std::basic_string_view<Symbol> text) { for (const auto& symbol : text) append(symbol); }
  int size() const { return n_; }
  int states() const { return static_cast<int>(states_.size()); }
  long long distinct_substrings() const { return distinct_; }
  void append(const Symbol& symbol) {
    assert(n_ < std::numeric_limits<int>::max() / 2);
    int current = static_cast<int>(states_.size());
    states_.push_back({{}, 0, n_ + 1, n_, false});
    ++n_;
    int p = last_;
    while (p != -1 && !states_[p].next.contains(symbol)) {
      states_[p].next.emplace(symbol, current);
      p = states_[p].link;
    }
    if (p != -1) {
      int q = states_[p].next.find(symbol)->second;
      if (states_[q].length == states_[p].length + 1) {
        states_[current].link = q;
      } else {
        State copied = states_[q];
        copied.length = states_[p].length + 1;
        copied.clone = true;
        int clone = static_cast<int>(states_.size());
        states_.push_back(copied);
        while (p != -1) {
          auto found = states_[p].next.find(symbol);
          if (found == states_[p].next.end() || found->second != q) break;
          found->second = clone;
          p = states_[p].link;
        }
        states_[q].link = states_[current].link = clone;
      }
    }
    last_ = current;
    distinct_ += states_[current].length - states_[states_[current].link].length;
    dirty_ = true;
  }
  template <class Range> requires (std::ranges::input_range<const Range> && !std::is_array_v<Range>)
  bool contains(const Range& pattern) const { return walk(pattern) != -1; }
  template <class Range> requires (std::ranges::input_range<const Range> && !std::is_array_v<Range>)
  int count(const Range& pattern) const {
    const int state = walk(pattern);
    if (state < 0) return 0;
    if (state == 0) return n_ + 1;
    rebuild_counts();
    return occurrences_[state];
  }
  bool contains(std::basic_string_view<Symbol> pattern) const { return contains<std::basic_string_view<Symbol>>(pattern); }
  int count(std::basic_string_view<Symbol> pattern) const { return count<std::basic_string_view<Symbol>>(pattern); }
  std::tuple<int, int, int> longest_common_substring(std::basic_string_view<Symbol> other) const {
    return longest_common_substring<std::basic_string_view<Symbol>>(other);
  }
  // Returns (start in original text, start in other, length).
  template <class Range> requires (std::ranges::input_range<const Range> && !std::is_array_v<Range>)
  std::tuple<int, int, int> longest_common_substring(const Range& other) const {
    int state = 0, length = 0, position = 0, best = 0, first = 0, second = 0;
    for (const auto& symbol : other) {
      assert(position < std::numeric_limits<int>::max());
      while (state && !states_[state].next.contains(symbol)) {
        state = states_[state].link;
        length = states_[state].length;
      }
      auto found = states_[state].next.find(symbol);
      if (found != states_[state].next.end()) { state = found->second; ++length; }
      if (length > best) {
        best = length;
        first = states_[state].first - best + 1;
        second = position - best + 1;
      }
      ++position;
    }
    return {first, second, best};
  }
};

}  // namespace blueberry


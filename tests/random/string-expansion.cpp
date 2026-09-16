#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <string_view>
#include <vector>

#include "blueberry/string/aho-corasick.hpp"
#include "blueberry/string/eertree.hpp"
#include "blueberry/string/lyndon-factorization.hpp"

unsigned long long seed;
void require(bool ok, const std::string& input, const std::string& operation,
             long long expected, long long actual) {
  if (!ok) {
    std::cerr << "seed=" << seed << " input=" << input << " operation=" << operation
              << " expected=" << expected << " actual=" << actual << '\n';
    std::exit(1);
  }
}
long long occurrences(const std::string& text, const std::string& pattern) {
  long long answer = 0;
  for (std::size_t i = 0; i + pattern.size() <= text.size(); ++i)
    answer += text.compare(i, pattern.size(), pattern) == 0;
  return answer;
}
bool palindrome(const std::string& s) {
  return std::equal(s.begin(), s.end(), s.rbegin());
}
void check_aho(const std::vector<std::string>& patterns, const std::string& text) {
  blueberry::AhoCorasick<> ac;
  std::map<std::string, int> ids{{"", 0}};
  std::vector<std::string> words{""};
  for (const auto& pattern : patterns) {
    for (std::size_t n = 1; n <= pattern.size(); ++n) {
      auto prefix = pattern.substr(0, n);
      if (!ids.count(prefix)) {
        ids[prefix] = static_cast<int>(words.size());
        words.push_back(prefix);
      }
    }
    const int v = ac.add(pattern);
    require(v == ids.at(pattern), pattern, "add id", ids.at(pattern), v);
  }
  ac.build();
  ac.build();
  const auto counts = ac.count(text);
  require(ac.size() == static_cast<int>(words.size()), text, "nodes", words.size(), ac.size());
  assert(counts.size() == words.size());
  for (int v = 0; v < ac.size(); ++v) {
    const auto& word = words[v];
    const auto expected = occurrences(text, word);
    require(counts[v] == expected, text + "/" + word, "count", expected, counts[v]);
    int suffix = 0;
    for (std::size_t first = 1; first < word.size(); ++first) {
      if (ids.count(word.substr(first))) { suffix = ids.at(word.substr(first)); break; }
    }
    require(ac.link(v) == suffix, word, "link", suffix, ac.link(v));
    const int parent = word.empty() ? -1 : ids.at(word.substr(0, word.size() - 1));
    require(ac.parent(v) == parent, word, "parent", parent, ac.parent(v));
    for (char c : std::string("abc")) {
      const auto appended = word + c;
      int next = 0;
      for (std::size_t first = 0; first < appended.size(); ++first) {
        if (ids.count(appended.substr(first))) { next = ids.at(appended.substr(first)); break; }
      }
      require(ac.next(v, c) == next, appended, "next", next, ac.next(v, c));
    }
  }
  assert(ac.count(text) == counts);
  auto copied = ac;
  assert(copied.count(text) == counts);
}
void check_eertree(const std::string& text) {
  blueberry::Eertree<> tree;
  std::map<std::string, int> ids{{"", 0}};
  std::vector<std::string> words{""};
  assert(tree.size() == 0 && tree.suffix() == 0);
  assert(tree.length(-1) == -1 && tree.length(0) == 0);
  assert(tree.link(-1) == -1 && tree.link(0) == -1);
  for (std::size_t end = 1; end <= text.size(); ++end) {
    std::string suffix;
    for (std::size_t begin = 0; begin < end; ++begin) {
      const auto candidate = text.substr(begin, end - begin);
      if (palindrome(candidate)) {
        if (suffix.empty()) suffix = candidate;
        if (!ids.count(candidate)) {
          ids[candidate] = static_cast<int>(words.size());
          words.push_back(candidate);
        }
      }
    }
    const int actual = tree.add(text[end - 1]);
    require(actual == ids.at(suffix), text.substr(0, end), "suffix", ids.at(suffix), actual);
    assert(actual == tree.suffix());
    // Counting must not consume accumulated suffix counts or prevent appending.
    const auto counts = tree.count();
    assert(counts.size() == words.size());
    for (int v = 0; v <= tree.size(); ++v) {
      const auto expected = occurrences(text.substr(0, end), words[v]);
      require(counts[v] == expected, text.substr(0, end) + "/" + words[v], "pal count", expected, counts[v]);
    }
  }
  assert(tree.size() + 1 == static_cast<int>(words.size()));
  for (int v = 1; v <= tree.size(); ++v) {
    const auto& word = words[v];
    const int parent = word.size() == 1 ? -1 : ids.at(word.substr(1, word.size() - 2));
    assert(tree.parent(v) == parent && tree.length(v) == static_cast<int>(word.size()));
    int suffix = 0;
    for (std::size_t begin = 1; begin < word.size(); ++begin) {
      if (palindrome(word.substr(begin))) { suffix = ids.at(word.substr(begin)); break; }
    }
    require(tree.link(v) == suffix, word, "pal link", suffix, tree.link(v));
  }
  blueberry::Eertree<> batch(text);
  assert(batch.count() == tree.count());
  auto copied = tree;
  assert(copied.count() == tree.count());
}
bool lyndon(const std::string& s) {
  for (std::size_t cut = 1; cut < s.size(); ++cut)
    if (!(s < s.substr(cut) + s.substr(0, cut))) return false;
  return !s.empty();
}
void check_lyndon(const std::string& text) {
  const int n = static_cast<int>(text.size());
  std::vector<int> expected;
  if (n == 0) expected = {0};
  for (int mask = 0; n && mask < (1 << (n - 1)); ++mask) {
    std::vector<int> bounds{0};
    for (int i = 1; i < n; ++i) if (mask >> (i - 1) & 1) bounds.push_back(i);
    bounds.push_back(n);
    bool valid = true;
    std::string previous;
    for (std::size_t i = 1; i < bounds.size(); ++i) {
      const auto word = text.substr(bounds[i - 1], bounds[i] - bounds[i - 1]);
      if (!lyndon(word) || (i > 1 && previous < word)) { valid = false; break; }
      previous = word;
    }
    if (valid) { assert(expected.empty()); expected = bounds; }
  }
  const auto actual = blueberry::lyndon_factorization(text);
  if (actual != expected) {
    std::cerr << "seed=" << seed << " input=" << text << " expected=";
    for (int x : expected) std::cerr << x << ',';
    std::cerr << " actual=";
    for (int x : actual) std::cerr << x << ',';
    std::cerr << '\n';
    std::exit(1);
  }
}
struct Symbol {
  int value;
  bool operator<(const Symbol& other) const { return value < other.value; }
};
int main(int argc, char** argv) {
  seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  std::mt19937_64 random(seed);
  check_aho({}, "");
  check_aho({"", "a", "aa", "a", "aba", "ba"}, "aababa");
  for (const std::string text : {"", "a", "aaaaaa", "abacaba", "abcabc", "abbabba"}) {
    check_eertree(text);
    check_lyndon(text);
  }
  for (int trial = 0; trial < 180; ++trial) {
    std::vector<std::string> patterns(random() % 15);
    for (auto& s : patterns) for (int i = static_cast<int>(random() % 9); i; --i)
      s += static_cast<char>('a' + random() % 3);
    std::string text;
    for (int i = static_cast<int>(random() % 30); i; --i) text += static_cast<char>('a' + random() % 3);
    check_aho(patterns, text);
    check_eertree(text);
    check_lyndon(text.substr(0, 9));
  }
  for (int n = 0; n <= 8; ++n) for (int mask = 0; mask < (1 << n); ++mask) {
    std::string text(n, 'a');
    for (int i = 0; i < n; ++i) text[i] += (mask >> i) & 1;
    check_lyndon(text);
  }
  assert((blueberry::lyndon_factorization(std::vector<Symbol>{{2}, {1}, {2}, {1}}) == std::vector<int>{0, 1, 3, 4}));
  blueberry::AhoCorasick<2, '0'> bits;
  const int v = bits.add("010");
  bits.build();
  assert(bits.count("01010")[v] == 2);
  blueberry::Eertree<2, '0'> pal("0110");
  assert(pal.size() == 4 && pal.length(pal.suffix()) == 4);
  // Byte alphabets must not treat NUL as a terminator or sign-extend high bytes.
  const std::string bytes("\0\xff\0\xff\0", 5);
  blueberry::AhoCorasick<256, 0> byte_ac;
  const int byte_pattern = byte_ac.add(std::string_view(bytes).substr(0, 3));
  byte_ac.build();
  assert(byte_ac.count(bytes)[byte_pattern] == 2);
  blueberry::Eertree<256, 0> byte_pal(bytes);
  assert(byte_pal.size() == 5 && byte_pal.length(byte_pal.suffix()) == 5);
  assert(byte_pal.count()[1] == 3 && byte_pal.count()[0] == 6);
  assert((blueberry::lyndon_factorization(std::vector<unsigned char>{255, 0, 255, 0}) ==
          std::vector<int>{0, 1, 3, 4}));
  const std::string repeated(100000, 'a');
  blueberry::Eertree<> deep(repeated);
  assert(deep.size() == 100000 && deep.count()[1] == 100000);
  blueberry::AhoCorasick<> ac(100001);
  const int tail = ac.add(repeated);
  ac.build();
  assert(ac.count(repeated)[tail] == 1);
  const auto bounds = blueberry::lyndon_factorization(repeated);
  assert(bounds.size() == repeated.size() + 1 && bounds.back() == 100000);
}

#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <string_view>
#include <vector>

namespace blueberry {

// Node IDs: -1 = odd root, 0 = empty palindrome, 1..size() = discovered palindromes.
template <int Alphabet = 26, unsigned char First = 'a'>
class Eertree {
  static_assert(Alphabet > 0 && static_cast<int>(First) + Alphabet <= 256);
  struct Node {
    std::array<int, Alphabet> next{};
    int length, parent, link;
    long long hits = 0;
    Node(int len, int p, int failure) : length(len), parent(p), link(failure) {}
  };
  std::vector<Node> nodes_;
  std::vector<unsigned char> text_;
  int last_ = 0;

  int extendable(int v, int pos, unsigned char c) const {
    while (pos - 1 - nodes_[v + 1].length < 0 ||
           text_[pos - 1 - nodes_[v + 1].length] != c)
      v = nodes_[v + 1].link;
    return v;
  }

 public:
  explicit Eertree(int reserve_length = 0) {
    assert(0 <= reserve_length && reserve_length <= std::numeric_limits<int>::max() - 2);
    nodes_.reserve(static_cast<std::size_t>(reserve_length) + 2);
    text_.reserve(static_cast<std::size_t>(reserve_length));
    nodes_.emplace_back(-1, -1, -1);
    nodes_.emplace_back(0, -1, -1);
  }
  explicit Eertree(std::string_view text) : Eertree() {
    assert(text.size() <= static_cast<std::size_t>(std::numeric_limits<int>::max() - 2));
    nodes_.reserve(text.size() + 2);
    text_.reserve(text.size());
    for (char c : text) add(c);
  }
  int size() const { return static_cast<int>(nodes_.size()) - 2; }
  int suffix() const { return last_; }

  int add(char c) {
    const int x = static_cast<unsigned char>(c) - static_cast<int>(First);
    assert(0 <= x && x < Alphabet);
    assert(text_.size() < static_cast<std::size_t>(std::numeric_limits<int>::max() - 2));
    const int pos = static_cast<int>(text_.size());
    text_.push_back(static_cast<unsigned char>(c));
    const int parent = extendable(last_, pos, static_cast<unsigned char>(c));
    if (nodes_[parent + 1].next[x] == 0) {
      const int len = nodes_[parent + 1].length + 2;
      const int failure = len == 1 ? 0 : nodes_[extendable(nodes_[parent + 1].link, pos,
                                                         static_cast<unsigned char>(c)) + 1].next[x];
      const int id = size() + 1;
      nodes_.emplace_back(len, parent, failure);
      nodes_[parent + 1].next[x] = id;
    }
    last_ = nodes_[parent + 1].next[x];
    ++nodes_[last_ + 1].hits;
    return last_;
  }
  int length(int v) const {
    assert(-1 <= v && v <= size());
    return nodes_[v + 1].length;
  }
  int parent(int v) const {
    assert(1 <= v && v <= size());
    return nodes_[v + 1].parent;
  }
  int link(int v) const {
    assert(-1 <= v && v <= size());
    return nodes_[v + 1].link;
  }
  std::vector<long long> count() const {
    std::vector<long long> result(static_cast<std::size_t>(size()) + 1);
    for (int v = 1; v <= size(); ++v) result[v] = nodes_[v + 1].hits;
    // A suffix-link destination was always discovered before its source.
    for (int v = size(); v >= 1; --v) result[nodes_[v + 1].link] += result[v];
    result[0] = static_cast<long long>(text_.size()) + 1;
    return result;
  }
};

}  // namespace blueberry

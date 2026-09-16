// Independent sparse-transition candidates; no third-party submission code.
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <vector>
#include "blueberry/string/aho-corasick.hpp"
#include "blueberry/string/eertree.hpp"
#include "blueberry/string/lyndon-factorization.hpp"

struct SparseAho {
  struct Node { std::vector<std::pair<char, int>> edges; int link = 0; };
  std::vector<Node> nodes{1};
  std::vector<int> bfs{0};
  int edge(int v, char c) const {
    for (auto [letter, child] : nodes[v].edges) if (letter == c) return child;
    return -1;
  }
  int add(const std::string& s) {
    int v = 0;
    for (char c : s) {
      int u = edge(v, c);
      if (u == -1) {
        u = static_cast<int>(nodes.size());
        nodes.emplace_back();
        nodes[v].edges.emplace_back(c, u);
      }
      v = u;
    }
    return v;
  }
  int next(int v, char c) const {
    int u = edge(v, c);
    while (u == -1 && v != 0) { v = nodes[v].link; u = edge(v, c); }
    return u == -1 ? 0 : u;
  }
  void build() {
    for (auto [c, u] : nodes[0].edges) { (void)c; bfs.push_back(u); }
    for (std::size_t i = 1; i < bfs.size(); ++i) {
      const int v = bfs[i];
      for (auto [c, u] : nodes[v].edges) {
        nodes[u].link = next(nodes[v].link, c);
        bfs.push_back(u);
      }
    }
  }
  std::vector<long long> count(const std::string& text) const {
    std::vector<long long> result(nodes.size());
    int v = 0;
    for (char c : text) { v = next(v, c); ++result[v]; }
    for (std::size_t i = bfs.size(); i > 1; --i) result[nodes[bfs[i - 1]].link] += result[bfs[i - 1]];
    ++result[0];
    return result;
  }
};

struct SparseEertree {
  struct Node { int length, link, head = -1; long long hits = 0; };
  struct Edge { char c; int to, next; };
  std::vector<Node> nodes{{-1, 0}, {0, 0}};
  std::vector<Edge> edges;
  std::string text;
  int last = 1;
  int edge(int v, char c) const {
    for (int e = nodes[v].head; e != -1; e = edges[e].next)
      if (edges[e].c == c) return edges[e].to;
    return -1;
  }
  int parent(int v, int p, char c) const {
    while (p - 1 - nodes[v].length < 0 || text[p - 1 - nodes[v].length] != c) v = nodes[v].link;
    return v;
  }
  void add(char c) {
    const int p = static_cast<int>(text.size());
    text += c;
    const int from = parent(last, p, c);
    int v = edge(from, c);
    if (v == -1) {
      const int len = nodes[from].length + 2;
      const int link = len == 1 ? 1 : edge(parent(nodes[from].link, p, c), c);
      v = static_cast<int>(nodes.size());
      nodes.push_back({len, link});
      edges.push_back({c, v, nodes[from].head});
      nodes[from].head = static_cast<int>(edges.size()) - 1;
    }
    last = v;
    ++nodes[v].hits;
  }
  std::vector<long long> count() const {
    std::vector<long long> result(nodes.size() - 1);
    for (int v = 2; v < static_cast<int>(nodes.size()); ++v) result[v - 1] = nodes[v].hits;
    for (int v = static_cast<int>(nodes.size()) - 1; v >= 2; --v) result[nodes[v].link - 1] += result[v - 1];
    result[0] = static_cast<long long>(text.size()) + 1;
    return result;
  }
};

std::uint64_t checksum(const std::vector<long long>& data) {
  std::uint64_t sum = 0;
  for (auto value : data) sum = sum * 1000003 + static_cast<std::uint64_t>(value);
  return sum;
}
int main(int argc, char** argv) {
  if (argc != 3) return 2;
  const std::string mode = argv[1], shape = argv[2];
  std::mt19937_64 rng(1729);
  std::string text(200000, 'a');
  for (std::size_t i = 0; i < text.size(); ++i) {
    if (shape == "random") text[i] = static_cast<char>('a' + rng() % 26);
    else if (shape == "periodic") text[i] = "abacaba"[i % 7];
  }
  std::vector<std::string> patterns;
  if (shape == "repeated") patterns.emplace_back(100000, 'a');
  else if (shape == "periodic") {
    for (int i = 0; i < 100; ++i) patterns.push_back(text.substr(i % 7, 4000));
  } else {
    patterns.resize(12000, std::string(32, 'a'));
    for (auto& s : patterns) for (char& c : s) c = static_cast<char>('a' + rng() % 26);
  }
  const auto start = std::chrono::steady_clock::now();
  std::uint64_t sum = 0;
  if (mode == "aho-dense") {
    blueberry::AhoCorasick<> ac;
    for (auto& s : patterns) ac.add(s);
    ac.build();
    sum = checksum(ac.count(text));
  } else if (mode == "aho-sparse") {
    SparseAho ac;
    for (auto& s : patterns) ac.add(s);
    ac.build();
    sum = checksum(ac.count(text));
  } else if (mode == "eertree-dense") {
    blueberry::Eertree<> tree;
    for (char c : text) tree.add(c);
    sum = checksum(tree.count());
  } else if (mode == "eertree-sparse") {
    SparseEertree tree;
    for (char c : text) tree.add(c);
    sum = checksum(tree.count());
  } else if (mode == "lyndon") {
    const auto bounds = blueberry::lyndon_factorization(text);
    for (int x : bounds) sum = sum * 1000003 + static_cast<std::uint64_t>(x);
  } else return 2;
  const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
  std::cout << "{\"mode\":\"" << mode << "\",\"shape\":\"" << shape
            << "\",\"milliseconds\":" << ms << ",\"checksum\":" << sum << "}\n";
}

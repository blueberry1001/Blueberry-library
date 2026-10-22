#include "blueberry/data-structure/implicit-treap.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
std::string concat(std::string a, std::string b) { return a + b; }
std::string empty_string() { return {}; }
struct Action {
  int a, b;
  Action() = delete;
  Action(int x, int y) : a(x), b(y) {}
};
std::string mapping(Action f, std::string s) {
  for (auto& c : s) c = char('a' + (f.a * (c - 'a') + f.b) % 26);
  return s;
}
Action composition(Action f, Action g) { return {f.a * g.a % 26, (f.a * g.b + f.b) % 26}; }
Action id() { return {1, 0}; }
using Tree = blueberry::ImplicitTreap<std::string, concat, empty_string, Action, mapping, composition, id>;
int main(int argc, char** argv) {
  std::uint64_t seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  int round = -1, l = 0, r = 0;
  std::vector<std::string> a;
  auto fail = [&](const char* op, const std::string& expected, const std::string& actual) {
    std::cerr << "seed=" << seed << " round=" << round << " op=" << op << " l=" << l
              << " r=" << r << " expected=" << expected << " actual=" << actual << " input=";
    for (const auto& x : a) std::cerr << '[' << x << ']';
    std::cerr << '\n'; std::exit(1);
  };
  auto expected = [&](int first, int last) {
    std::string s;
    for (int i = first; i < last; ++i) s += a[i];
    return s;
  };
  for (int n : {0, 1, 2, 17, 65}) {
    a.resize(n);
    for (int i = 0; i < n; ++i) {
      a[i].clear();
      for (int j = 0; j < (i + 1) % 5; ++j) a[i] += char('a' + (i * 7 + j) % 26);
    }
    Tree tree(a, seed + n);
    auto apply = [&](int first, int last, Action f) {
      tree.apply(first, last, f);
      for (int i = first; i < last; ++i) a[i] = mapping(f, a[i]);
    };
    auto reverse = [&](int first, int last) {
      tree.reverse(first, last); std::reverse(a.begin() + first, a.begin() + last);
    };
    for (round = 0; round < 8; ++round) {
      apply(0, n, {2, 3}); reverse(0, n);
      apply(n / 3, n - n / 3, {5, 7});
      reverse(0, n - n / 4); apply(n / 4, n, {round % 3, round + 1});
      apply(0, n, {1, 0}); reverse(n / 2, n); reverse(0, n);
      auto total = expected(0, n);
      if (tree.all_prod() != total) fail("all_prod", total, tree.all_prod());
      l = 0; r = n;
      if (tree.prod(l, r) != total) fail("full prod", total, tree.prod(l, r));
      for (l = n; l >= 0; --l) for (r = n; r >= l; --r) {
        auto want = expected(l, r), got = tree.prod(l, r);
        if (want != got) fail("prod", want, got);
      }
      auto got = tree.to_vector();
      if (got.size() != a.size()) fail("to_vector size", std::to_string(a.size()), std::to_string(got.size()));
      for (l = 0; l < n; ++l) if (got[l] != a[l]) fail("to_vector", a[l], got[l]);
    }
  }
}

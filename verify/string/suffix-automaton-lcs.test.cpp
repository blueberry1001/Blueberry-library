#define PROBLEM "https://judge.yosupo.jp/problem/longest_common_substring"
#include <iostream>
#include <string>
#include "blueberry/string/suffix-automaton.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  string s, t;
  cin >> s >> t;
  blueberry::SuffixAutomaton<> sam(s);
  auto [a, b, length] = sam.longest_common_substring(t);
  cout << a << ' ' << a + length << ' ' << b << ' ' << b + length << '\n';
}

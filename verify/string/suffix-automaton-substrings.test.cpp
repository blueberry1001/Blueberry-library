#define PROBLEM "https://judge.yosupo.jp/problem/number_of_substrings"
#include <iostream>
#include <string>
#include "blueberry/string/suffix-automaton.hpp"
using namespace std;
int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  string s;
  cin >> s;
  blueberry::SuffixAutomaton<> sam(s);
  cout << sam.distinct_substrings() << '\n';
}

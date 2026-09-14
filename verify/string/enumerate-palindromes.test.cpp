#define PROBLEM "https://judge.yosupo.jp/problem/enumerate_palindromes"

#include <iostream>
#include <string>

#include "blueberry/string/manacher.hpp"

using namespace std;

// 文字と文字間を中心とする最長回文の長さを左から順に列挙する。
int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  string text;
  cin >> text;
  const auto length = blueberry::manacher(text);
  for (int i = 0; i < static_cast<int>(length.size()); ++i) {
    if (i) cout << ' ';
    cout << length[i];
  }
  cout << '\n';
}

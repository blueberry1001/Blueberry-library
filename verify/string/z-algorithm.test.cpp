#define PROBLEM "https://judge.yosupo.jp/problem/zalgorithm"

#include <iostream>
#include <string>

#include "blueberry/string/z-algorithm.hpp"

using namespace std;

// 各接尾辞と文字列全体の最長共通接頭辞長をZ配列として出力する。

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  string text;
  cin >> text;
  const auto z = blueberry::z_algorithm(text);
  for (int i = 0; i < static_cast<int>(z.size()); ++i) {
    if (i > 0) cout << ' ';
    cout << z[i];
  }
  cout << '\n';
}

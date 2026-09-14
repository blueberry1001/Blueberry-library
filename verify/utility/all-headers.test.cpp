#define PROBLEM "https://judge.yosupo.jp/problem/many_aplusb"

#include <iostream>

#include "blueberry/all.hpp"

using namespace std;

// all.hppを単独includeできることを確認する。実行内容はmany_aplusbの入出力のみ。

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  int cases;
  cin >> cases;
  while (cases--) {
    long long a, b;
    cin >> a >> b;
    cout << a + b << '\n';
  }
}

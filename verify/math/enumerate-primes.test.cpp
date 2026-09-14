#define PROBLEM "https://judge.yosupo.jp/problem/enumerate_primes"

#include <iostream>

#include "blueberry/math/prime-sieve.hpp"

using namespace std;

// N以下の素数を列挙し、問題の指定に従いoffset番目からstep個おきに出力する。

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  int n, step, offset;
  cin >> n >> step >> offset;
  const blueberry::PrimeSieve sieve(n);
  const auto& primes = sieve.primes();
  int selected = 0;
  for (int i = offset; i < static_cast<int>(primes.size()); i += step) ++selected;

  cout << primes.size() << ' ' << selected << '\n';
  bool first = true;
  for (int i = offset; i < static_cast<int>(primes.size()); i += step) {
    if (!first) cout << ' ';
    first = false;
    cout << primes[i];
  }
  cout << '\n';
}

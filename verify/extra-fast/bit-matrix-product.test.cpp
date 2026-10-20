#define PROBLEM "https://judge.yosupo.jp/problem/matrix_product_mod_2"
#include <string>
#include <vector>
#include "blueberry/extra-fast/bitset.hpp"
#include "blueberry/utility/fast-io.hpp"

using namespace std;
using blueberry::extra_fast::Bitset;

int main() {
  blueberry::FastInput input;
  blueberry::FastOutput output;
  int n, m, k;
  input.read(n, m, k);
  vector<string> a(n);
  for (auto& row : a) input.read(row);
  vector<Bitset> b;
  b.reserve(m);
  for (int i = 0; i < m; ++i) {
    string row;
    input.read(row);
    b.emplace_back(k);
    for (int j = 0; j < k; ++j) if (row[j] == '1') b.back().set(j);
  }
  // A row of AB over F_2 is the XOR of B's rows selected by A's 1 bits.
  Bitset result(k);
  string row(k, '0');
  for (int i = 0; i < n; ++i) {
    result.clear();
    for (int j = 0; j < m; ++j) if (a[i][j] == '1') result.xor_with(b[j]);
    for (int j = 0; j < k; ++j) row[j] = result.test(j) ? '1' : '0';
    output.write(row);
    output.write('\n');
  }
}

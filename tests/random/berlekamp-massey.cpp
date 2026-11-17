#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>

#include "blueberry/math/linear-recurrence.hpp"

using namespace std;

template<int P> struct Field {
  int value;
  static inline int divisions = 0;
  Field(int x = 0) : value((x % P + P) % P) {}
  Field& operator+=(Field x) { value = (value + x.value) % P; return *this; }
  Field& operator-=(Field x) { value = (value - x.value + P) % P; return *this; }
  friend Field operator*(Field a, Field b) { return Field(a.value * b.value); }
  friend Field operator/(Field a, Field b) {
    if (b.value == 0) abort();
    ++divisions;
    for (int k = 1; k < P; ++k) if (b.value * k % P == 1) return a * Field(k);
    abort();
  }
  Field operator-() const { return Field(-value); }
  friend bool operator==(Field a, Field b) { return a.value == b.value; }
  friend bool operator!=(Field a, Field b) { return !(a == b); }
};

void require(bool condition, unsigned long long seed, int trial, const char* message) {
  if (condition) return;
  cerr << "seed=" << seed << " trial=" << trial << " operation=" << message << '\n';
  exit(1);
}

template<class M> bool fits_degree(const vector<M>& s, int d) {
  const int rows = static_cast<int>(s.size()) - d;
  vector<vector<M>> a(rows, vector<M>(d + 1));
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < d; ++j) a[i][j] = s[i + d - j - 1];
    a[i][d] = s[i + d];
  }
  int r = 0;
  for (int c = 0; c < d && r < rows; ++c) {
    int p = r;
    while (p < rows && a[p][c] == M(0)) ++p;
    if (p == rows) continue;
    swap(a[p], a[r]);
    const M inverse = M(1) / a[r][c];
    for (int j = c; j <= d; ++j) a[r][j] = a[r][j] * inverse;
    for (int i = r + 1; i < rows; ++i) {
      const M scale = a[i][c];
      for (int j = c; j <= d; ++j) a[i][j] -= scale * a[r][j];
    }
    ++r;
  }
  for (int i = r; i < rows; ++i) if (a[i][d] != M(0)) return false;
  return true;
}

template<class M> void check(const vector<M>& sequence, unsigned long long seed, int trial) {
  const auto coefficients = blueberry::berlekamp_massey(sequence);
  int minimum = 0;
  while (!fits_degree(sequence, minimum)) ++minimum;
  require(static_cast<int>(coefficients.size()) == minimum, seed, trial, "minimal degree");
  for (int i = minimum; i < static_cast<int>(sequence.size()); ++i) {
    M value = 0;
    for (int j = 0; j < minimum; ++j) value += coefficients[j] * sequence[i - j - 1];
    require(value == sequence[i], seed, trial, "recurrence");
  }
}

int main(int argc, char** argv) {
  const auto seed = argc > 1 ? strtoull(argv[1], nullptr, 10) : 1;
  mt19937_64 random(seed);
  for (int n = 0; n <= 9; ++n) for (int mask = 0; mask < (1 << n); ++mask) {
    vector<Field<2>> sequence(n);
    for (int i = 0; i < n; ++i) sequence[i] = (mask >> i) & 1;
    check(sequence, seed, mask);
  }
  for (int trial = 0; trial < 500; ++trial) {
    vector<Field<7>> sequence(random() % 25);
    for (auto& x : sequence) x = random() % 7;
    check(sequence, seed, trial);
  }
  vector<Field<7>> zero(300), sequence(300);
  Field<7>::divisions = 0;
  require(blueberry::berlekamp_massey(zero).empty(), seed, 0, "zero sequence");
  require(Field<7>::divisions == 0, seed, 0, "zero divisions");
  for (auto& x : sequence) x = random() % 7;
  Field<7>::divisions = 0;
  const auto coefficients = blueberry::berlekamp_massey(sequence);
  require(Field<7>::divisions <= 151, seed, 0, "cached inverses");
  for (int i = static_cast<int>(coefficients.size()); i < 300; ++i) {
    Field<7> value = 0;
    for (int j = 0; j < static_cast<int>(coefficients.size()); ++j) value += coefficients[j] * sequence[i - j - 1];
    require(value == sequence[i], seed, i, "long recurrence");
  }
}

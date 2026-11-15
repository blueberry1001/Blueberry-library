#include <cassert>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <vector>
#include "weighted-wavelet-matrix.hpp"

template<class T> void check() {
  std::vector<int> values(16);
  std::iota(values.begin(), values.end(), 0);
  std::vector<T> weights(16);
  weights[0] = -1;
  weights[8] = 1;
  weights[12] = std::numeric_limits<T>::max();
  blueberry::WeightedWaveletMatrix<int,T> tree(values, weights);
  assert(tree.sum(0,16,10,14) == std::numeric_limits<T>::max());
}

int main() {
  check<int>();
  check<long long>();
  std::vector<int> values(16);
  std::iota(values.begin(), values.end(), 0);
  std::vector<double> weights(16);
  weights[0] = 1e16; weights[8] = 1; weights[12] = -1e16;
  blueberry::WeightedWaveletMatrix<int,double> tree(values, weights);
  std::cout << std::hexfloat << tree.sum(0,16,10,14) << '\n';
}

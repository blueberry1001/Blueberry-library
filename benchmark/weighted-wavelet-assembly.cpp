#include "weighted-wavelet-matrix.hpp"
using Tree = blueberry::WeightedWaveletMatrix<long long>;
extern "C" void weighted_add(Tree& tree, int p, long long value) { tree.add(p, value); }
extern "C" long long weighted_sum(const Tree& tree, int l, int r, long long low, long long high) {
  return tree.sum(l, r, low, high);
}

#include <cassert>
#include <cstdio>
#include <utility>
#include <vector>
#include "instrumented.hpp"
int main() {
  std::vector<std::pair<int, int>> edges;
  for (int g = 0; g < 8; ++g) for (int i = 0; i < 5; ++i) for (int j = i + 1; j < 5; ++j) edges.emplace_back(5 * g + i, 5 * g + j);
  auto result = blueberry::maximum_independent_set(40, edges);
  assert(blueberry::budget_exhausted);
  assert(result.size() == 8);
  printf("fallback verified\n");
}

#include "candidate.hpp"
std::vector<int> solve_max_independent_set(int n, const std::vector<std::pair<int, int>>& edges) {
  return blueberry::maximum_independent_set(n, edges);
}

#include <iostream>
#include <optional>
#include <random>
#include <set>
#include <vector>
#include "blueberry/data-structure/partially-retroactive-priority-queue.hpp"
int main(int argc, char** argv) {
  const unsigned seed = argc > 1 ? std::stoul(argv[1]) : 1;
  std::mt19937 rng(seed);
  for (int n : {1, 2, 3, 7, 31, 32, 33, 100}) {
    blueberry::PartiallyRetroactivePriorityQueue<int> ds(n);
    std::vector<int> kind(n), value(n);
    for (int step = 0; step < 20000; ++step) {
      const int t = rng() % n, type = rng() % 3, x = int(rng() % 11) - 5;
      auto next = kind;
      auto next_value = value;
      bool expected = type == 2 ? kind[t] != 0 : kind[t] == 0;
      if (expected) {
        next[t] = type == 2 ? 0 : type + 1;
        next_value[t] = x;
      }
      std::multiset<std::pair<int, int>> queue;
      for (int i = 0; i < n && expected; ++i) {
        if (next[i] == 1) queue.emplace(next_value[i], i);
        if (next[i] == 2) {
          if (queue.empty()) expected = false;
          else queue.erase(queue.begin());
        }
      }
      bool actual = type == 0 ? ds.insert_op(t, x) : type == 1 ? ds.pop_op(t) : ds.erase_op(t);
      if (actual != expected) {
        std::cerr << "accept seed=" << seed << " n=" << n << " step=" << step << " time=" << t << " type=" << type << " expected=" << expected << " actual=" << actual << '\n';
        return 1;
      }
      if (expected) { kind = next; value = next_value; }
      queue.clear();
      for (int i = 0; i < n; ++i) {
        if (kind[i] == 1) queue.emplace(value[i], i);
        if (kind[i] == 2) queue.erase(queue.begin());
        if (ds.has_op(i) != (kind[i] != 0)) return 1;
      }
      std::optional<int> want = queue.empty() ? std::nullopt : std::optional<int>(queue.begin()->first);
      if (ds.min() != want || ds.size() != int(queue.size()) || ds.empty() != queue.empty()) {
        std::cerr << "state seed=" << seed << " n=" << n << " step=" << step << " time=" << t << " type=" << type << " size=" << ds.size() << " expected=" << queue.size() << '\n';
        for (int i = 0; i < n; ++i) std::cerr << i << ':' << kind[i] << ',' << value[i] << ' ';
        return 1;
      }
    }
  }
}

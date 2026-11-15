#include "blueberry/data-structure/fast-set.hpp"
#include "blueberry/utility/fast-io.hpp"
#include <chrono>
#include <cstdio>
#include <string>
#include <vector>

using Clock = std::chrono::steady_clock;
using Time = Clock::time_point;
double elapsed(Time a, Time b) { return std::chrono::duration<double, std::milli>(b - a).count(); }
struct Query { int type, x; };
int main() {
  blueberry::FastInput in;
  int n, q; std::string bits;
  auto a = Clock::now();
  in.read(n, q, bits);
  std::vector<Query> queries(q);
  for (auto& query : queries) in.read(query.type, query.x);
  auto b = Clock::now();
  blueberry::FastSet set(bits);
  auto c = Clock::now();
  std::vector<int> answers; answers.reserve(q);
  for (auto [type, x] : queries) {
    if (type == 0) set.insert(x);
    else if (type == 1) set.erase(x);
    else if (type == 2) answers.push_back(set.contains(x));
    else if (type == 3) { int y = set.next(x); answers.push_back(y == n ? -1 : y); }
    else answers.push_back(set.prev(x));
  }
  auto d = Clock::now();
  {
    blueberry::FastOutput out;
    for (int answer : answers) out.writeln(answer);
  }
  auto e = Clock::now();
  std::fprintf(stderr, "{\"parse_ms\":%.6f,\"build_ms\":%.6f,\"operation_ms\":%.6f,\"output_ms\":%.6f}\n",
    elapsed(a,b), elapsed(b,c), elapsed(c,d), elapsed(d,e));
}

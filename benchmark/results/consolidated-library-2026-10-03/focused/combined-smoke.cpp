#include "blueberry/all.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace integration_check {
void require(bool ok, const char* label) {
  if (!ok) { std::cerr << label << '\n'; std::abort(); }
}
struct Affine { long long a, b; };
Affine vertex(Affine value, long long light) { return {value.a, value.a * light + value.b}; }
long long edge(Affine path) { return path.b; }
Affine compress(Affine left, Affine right) { return {left.a * right.a, left.a * right.b + left.b}; }
long long rake(long long left, long long right) { return left + right; }
long long identity() { return 0; }
using TopTree = blueberry::StaticTopTree<Affine, Affine, long long, vertex, edge, compress, rake, identity>;

// These are the same minimal generic contracts exercised by the permanent point tests.
struct Additive {
  long long value = 0;
  Additive() = default;
  explicit Additive(long long x) : value(x) {}
  Additive(const Additive&) = default;
  Additive& operator=(const Additive&) = delete;
  Additive& operator+=(const Additive& rhs) { value += rhs.value; return *this; }
  Additive& operator-=(const Additive&) = delete;
  friend Additive operator-(const Additive& lhs, const Additive& rhs) { return Additive(lhs.value - rhs.value); }
};
struct Value {
  std::string text;
  Value() = delete;
  explicit Value(std::string s) : text(std::move(s)) {}
  Value(const Value&) = default;
  Value& operator=(const Value&) = delete;
};
struct Concat {
  bool reverse;
  int* calls;
  Concat() = delete;
  Concat(bool backwards, int& counter) : reverse(backwards), calls(&counter) {}
  Value operator()(const Value& left, const Value& right) const {
    ++*calls;
    return Value(reverse ? right.text + left.text : left.text + right.text);
  }
};
static_assert(!std::is_copy_assignable_v<Additive> && !std::is_default_constructible_v<Value>);
static_assert(!std::is_copy_assignable_v<Value> && !std::is_default_constructible_v<Concat>);

void graph_checks() {
  const std::vector<std::vector<int>> graph{{1,2},{3},{3},{4},{},{}};
  require(blueberry::dominator_tree(graph, 0) == std::vector<int>({0,0,0,0,3,-1}), "dominator tree through all.hpp");
  require(blueberry::dominator_tree({}, -1).empty(), "empty dominator tree");
  const std::vector<std::pair<int,int>> edges{{0,1},{1,2},{2,0},{2,3},{3,4},{0,1},{4,4}};
  const auto mate = blueberry::general_matching(5, edges);
  int pairs = 0;
  for (int v = 0; v < 5; ++v) {
    const int to = mate[v];
    if (to == -1) continue;
    require(0 <= to && to < 5 && to != v && mate[to] == v, "matching indices/symmetry");
    require(std::find(edges.begin(), edges.end(), std::pair<int,int>{v,to}) != edges.end() ||
            std::find(edges.begin(), edges.end(), std::pair<int,int>{to,v}) != edges.end(), "matching edge exists");
    pairs += v < to;
  }
  require(pairs == 2 && blueberry::general_matching(0, {}).empty(), "maximum matching size/empty");
}
void geometry_checks() {
  using Point = std::pair<long long,long long>;
  const std::vector<Point> points{{0,0},{2,0},{2,2},{0,2},{1,1},{2,2}};
  require(blueberry::convex_hull(points) == std::vector<Point>({{0,0},{2,0},{2,2},{0,2}}), "strict hull order");
  const auto [i,j] = blueberry::furthest_pair(points);
  require(0 <= i && i < j && j < static_cast<int>(points.size()), "furthest original indices");
  const auto dx = points[i].first-points[j].first, dy = points[i].second-points[j].second;
  require(dx*dx+dy*dy == 8, "furthest square diagonal");
  constexpr long long bound = (1LL<<62)-1;
  require(blueberry::furthest_pair(std::vector<Point>(3,{bound,-bound})) == std::pair<int,int>{0,1}, "all-identical boundary shortcut");
  require(blueberry::furthest_pair(std::vector<Point>{}) == std::pair<int,int>{-1,-1}, "empty furthest pair");
}
void top_tree_checks() {
  const std::vector<std::vector<int>> chain{{1},{0,2},{1}};
  TopTree tree(chain, {{2,1},{3,4},{5,6}}, 2);
  require(tree.size() == 3 && tree.all_prod().b == 41, "noncommutative top-tree direction");
  tree.set(0, {7,2});
  require(tree.all_prod().b == 56 && tree.get(0).a == 7, "top-tree update/get");
  auto copy = tree.get(0); copy.b = 99;
  require(copy.b != tree.get(0).b, "top-tree accessor value semantics");
}
void point_checks() {
  constexpr long long n = 1LL<<40;
  blueberry::DynamicFenwickTree<Additive> fenwick(n);
  fenwick.add(7, Additive(-3)); fenwick.add(7, Additive(4)); fenwick.add(n-1, Additive(2));
  require(fenwick.get(0).value == 0 && fenwick.get(7).value == 1 && fenwick.get(n-1).value == 2,
          "sparse generic Fenwick point access");
  require(fenwick.sum(0,n).value == 3, "generic Fenwick sum coexistence");
  blueberry::DynamicFenwickTree<long long,int> limits(4);
  limits.add(0, std::numeric_limits<long long>::max());
  limits.add(1, -std::numeric_limits<long long>::max());
  limits.add(2, std::numeric_limits<long long>::max());
  require(limits.get(3) == 0, "Fenwick cancellation must not overflow intermediate subtraction");
  for (bool reverse : {false,true}) {
    int calls = 0;
    blueberry::PersistentSegmentTree<Value,Concat> tree(std::vector<Value>{Value("a"),Value("b"),Value("c")}, Concat(reverse,calls), Value(""));
    const int one = tree.set(0,1,Value("X"));
    const int two = tree.apply(0,0,Value("Y"));
    calls = 0;
    require(tree.get(one,1).text == "X" && tree.get(0,1).text == "b" &&
            tree.get(two,0).text == (reverse ? "Ya" : "aY") && calls == 0,
            "persistent get keeps generic values, history and zero callback");
    require(tree.prod(one,0,3).text == (reverse ? "cXa" : "aXc"), "persistent noncommutative range order");
    auto copy = tree.get(one,1); copy.text = "changed";
    require(tree.get(one,1).text == "X" && tree.versions() == 3, "persistent get returns independent value");
  }
}
} // namespace integration_check
int main() {
  integration_check::graph_checks();
  integration_check::geometry_checks();
  integration_check::top_tree_checks();
  integration_check::point_checks();
  std::cout << "PASS combined graph/geometry/top-tree/point-access integration\n";
}

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <utility>
#include <vector>
#include "blueberry/data-structure/disjoint-sparse-table.hpp"
#include "blueberry/data-structure/linear-rmq.hpp"
#include "blueberry/data-structure/sqrt-tree.hpp"

using Clock = std::chrono::steady_clock;
using U = std::uint64_t;
struct Affine { U a, b; };
struct Matrix { U a, b, c, d; };
U sum(U a, U b) { return a + b; }
U zero() { return 0; }
U minimum(U a, U b) { return std::min(a, b); }
U infinity() { return std::numeric_limits<U>::max(); }
Affine compose(Affine a, Affine b) { return {a.a*b.a, a.b*b.a+b.b}; }
Affine identity() { return {1, 0}; }
Matrix multiply(Matrix a, Matrix b) {
  return {a.a*b.a+a.b*b.c, a.a*b.b+a.b*b.d,
          a.c*b.a+a.d*b.c, a.c*b.b+a.d*b.d};
}
Matrix unit() { return {1, 0, 0, 1}; }
U hash(U x) { return x; }
U hash(Affine x) { return x.a ^ (x.b*31); }
U hash(Matrix x) { return x.a ^ (x.b*31) ^ (x.c*37) ^ (x.d*41); }
double ms(Clock::time_point a, Clock::time_point b) {
  return std::chrono::duration<double, std::milli>(b-a).count();
}
template<class T, auto op, auto e>
void run(const std::vector<T>& values, const std::vector<std::pair<int,int>>& queries,
         const std::string& structure) {
  U checksum = 0;
  const auto start = Clock::now();
  if (structure == "sqrt") {
    blueberry::SqrtTree<T, op, e> tree(values);
    const auto built = Clock::now();
    for (auto [l,r] : queries) checksum = checksum*31 + hash(tree.prod(l,r));
    const auto queried = Clock::now();
    std::cout << ms(start,built) << ',' << ms(built,queried) << ',' << checksum << '\n';
  } else {
    blueberry::DisjointSparseTable<T, decltype(op)> tree(values, op);
    const auto built = Clock::now();
    for (auto [l,r] : queries) checksum = checksum*31 + hash(tree.prod(l,r));
    const auto queried = Clock::now();
    std::cout << ms(start,built) << ',' << ms(built,queried) << ',' << checksum << '\n';
  }
}
// All inputs are generated before timing, without any benchmark-only public API.
// Unsigned ring arithmetic preserves associativity without signed overflow.
int main(int argc, char** argv) {
  if (argc != 6) return 2;
  std::string structure=argv[1], algebra=argv[2], distribution=argv[5];
  int n=std::atoi(argv[3]), q=std::atoi(argv[4]);
  if (n <= 0 || q < 0) return 2;
  std::mt19937_64 rng(718239);
  std::vector<U> values(n);
  for (int i=0;i<n;++i) values[i] = distribution=="equal" ? 7 :
      distribution=="ascending" ? static_cast<U>(i) :
      distribution=="descending" ? static_cast<U>(n-i) : rng();
  std::vector<std::pair<int,int>> queries(q);
  for (auto& [l,r] : queries) {
    l=rng()%n; r=rng()%n;
    if (l>r) std::swap(l,r);
    ++r;
    if (distribution=="short") r=std::min(n,l+1+static_cast<int>(rng()%16));
  }
  if (structure=="linear") {
    const auto start=Clock::now();
    blueberry::LinearRMQ<U> tree(values);
    const auto built=Clock::now();
    U checksum=0;
    for (auto [l,r]:queries) checksum=checksum*31+static_cast<U>(tree.argmin(l,r));
    const auto queried=Clock::now();
    std::cout << ms(start,built) << ',' << ms(built,queried) << ',' << checksum << '\n';
  } else if (algebra=="min") run<U,minimum,infinity>(values,queries,structure);
  else if (algebra=="sum") run<U,sum,zero>(values,queries,structure);
  else if (algebra=="affine") {
    std::vector<Affine> a; a.reserve(n);
    for (auto x:values) a.push_back({x,x+1});
    run<Affine,compose,identity>(a,queries,structure);
  } else if (algebra=="matrix") {
    std::vector<Matrix> a; a.reserve(n);
    for (auto x:values) a.push_back({x,x+1,x+2,x+3});
    run<Matrix,multiply,unit>(a,queries,structure);
  } else return 2;
}

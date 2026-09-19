// Same public API and workloads for every variant; no input/output in timed phases.
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#ifndef LCT_HEADER
#define LCT_HEADER "candidate.hpp"
#endif
#include LCT_HEADER

std::uint64_t calls = 0;
void count_op() {
#ifdef LCT_COUNT_OPS
  ++calls;
#endif
}
struct Sum {
  using Value = std::uint64_t;
  static Value make(unsigned x) { return x % 1000000; }
  static Value op(Value a, Value b) { count_op(); return a + b; }
  static Value e() { return 0; }
  static std::uint64_t hash(Value a) { return a; }
};
struct Affine {
  using Value = std::array<std::uint64_t, 2>;
  static constexpr std::uint64_t mod = 998244353;
  static Value make(unsigned x) { return {1 + x % (mod - 1), (x * 37ull) % mod}; }
  static Value op(Value a, Value b) {
    count_op(); return {a[0] * b[0] % mod, (a[1] * b[0] + b[1]) % mod};
  }
  static Value e() { return {1, 0}; }
  static std::uint64_t hash(Value a) { return a[0] * mod + a[1]; }
};
struct Matrix {
  using Value = std::array<std::uint64_t, 4>;
  static constexpr std::uint64_t mod = 998244353;
  static Value make(unsigned x) { return {x % mod, (x * 7ull) % mod, (x * 13ull) % mod, (x * 31ull + 1) % mod}; }
  static Value op(Value a, Value b) {
    count_op();
    return {(a[0] * b[0] + a[1] * b[2]) % mod, (a[0] * b[1] + a[1] * b[3]) % mod,
            (a[2] * b[0] + a[3] * b[2]) % mod, (a[2] * b[1] + a[3] * b[3]) % mod};
  }
  static Value e() { return {1, 0, 0, 1}; }
  static std::uint64_t hash(Value a) { return (a[0] * mod + a[1]) ^ (a[2] * mod + a[3]); }
};
struct Query { int type, u, v, old; unsigned value; };
using Clock = std::chrono::steady_clock;
double ms(Clock::time_point a, Clock::time_point b) { return std::chrono::duration<double, std::milli>(b-a).count(); }
void require(bool ok) { if (!ok) std::abort(); }
template<class Monoid> void run(const std::string& shape, int n, int q, unsigned seed) {
  using S = typename Monoid::Value;
  std::mt19937 rng(seed);
  std::vector<S> values; values.reserve(n);
  for (int i=0;i<n;++i) values.push_back(Monoid::make(rng()));
  std::vector<int> parent(n, -1);
  const int core = n / 2;
  for (int v=1;v<n;++v) {
    if (shape=="path") parent[v]=v-1;
    else if (shape=="star") parent[v]=0;
    else parent[v]=static_cast<int>(rng()%((shape=="dynamic" && v>=core)?core:v));
  }
  auto current_parent=parent;
  std::vector<Query> queries; queries.reserve(q);
  for (int i=0;i<q;++i) {
    Query x{0,static_cast<int>(rng()%n),static_cast<int>(rng()%n),0,static_cast<unsigned>(rng())};
    const unsigned k=rng()%100;
    if (shape=="dynamic" && k<20) {
      x.type=6; x.u=core+static_cast<int>(rng()%(n-core)); x.v=rng()%core;
      x.old=current_parent[x.u];current_parent[x.u]=x.v;
    } else if (k<60) x.type=0;
    else if (k<80) x.type=1;
    else if (k<90) x.type=2;
    else if (k<94) x.type=3;
    else if (k<97) x.type=4;
    else x.type=5;
    queries.push_back(x);
  }
  const auto start=Clock::now(); calls=0;
  blueberry::LinkCutTree<S,Monoid::op,Monoid::e> tree(values);
  for (int i=1;i<n;++i) require(tree.link(i,parent[i]));
  const auto built=Clock::now(); const auto build_calls=calls;calls=0;
  std::uint64_t checksum=0;
  for (const auto& x:queries) {
    if (x.type==0) checksum=checksum*1000003+Monoid::hash(tree.prod(x.u,x.v));
    else if (x.type==1) { auto a=Monoid::make(x.value);tree.set(x.u,a);require(tree.get(x.u)==a); }
    else if (x.type==2) { tree.evert(x.u);require(tree.leader(x.u)==x.u); }
    else if (x.type==3) require(tree.same(x.u,x.v));
    else if (x.type==4) require(!tree.link(x.u,x.v));
    else if (x.type==5) require(!tree.cut(x.u,x.u));
    else { require(tree.cut(x.u,x.old));require(tree.link(x.u,x.v)); }
  }
  const auto end=Clock::now();
  std::cout<<"{\"build_ms\":"<<ms(start,built)<<",\"operations_ms\":"<<ms(built,end)
           <<",\"total_ms\":"<<ms(start,end)<<",\"checksum\":"<<checksum
           <<",\"build_op_calls\":"<<build_calls<<",\"operation_op_calls\":"<<calls<<"}\n";
}
int main(int argc,char**argv) {
  if(argc!=6)return 2;
  const std::string monoid=argv[1],shape=argv[2];
  const int n=std::atoi(argv[3]),q=std::atoi(argv[4]);const unsigned seed=std::strtoul(argv[5],nullptr,10);
  if(n<2||q<1||(shape!="path"&&shape!="star"&&shape!="random"&&shape!="dynamic"))return 2;
  if(monoid=="sum")run<Sum>(shape,n,q,seed);
  else if(monoid=="affine")run<Affine>(shape,n,q,seed);
  else if(monoid=="matrix")run<Matrix>(shape,n,q,seed);
  else return 2;
}

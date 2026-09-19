#include "blueberry/data-structure/disjoint-sparse-table.hpp"
#include "blueberry/data-structure/linear-rmq.hpp"
#include "blueberry/data-structure/sqrt-tree.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>

struct Item {
  int key, identity;
  Item() = delete;
  Item(int key_, int identity_) : key(key_), identity(identity_) {}
};
struct Compare {
  bool reverse;
  int* calls;
  bool operator()(const Item& a, const Item& b) const {
    ++*calls;
    return reverse ? a.key > b.key : a.key < b.key;
  }
};
struct Word {
  std::vector<int> letters;
  Word() = delete;
  explicit Word(std::vector<int> v) : letters(std::move(v)) {}
  bool operator==(const Word&) const = default;
};
Word concat(Word a, const Word& b) {
  a.letters.insert(a.letters.end(), b.letters.begin(), b.letters.end());
  return a;
}
Word empty() { return Word(std::vector<int>{}); }

int main(int argc, char** argv) {
  const auto seed = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1;
  std::mt19937_64 rng(seed);
  for (int n : {0,1,2,3,7,8,9,15,16,17,31,32,33,63,64,65,127,128,129,255,256,257,1023,1024,1025}) {
    for (bool reverse : {false,true}) {
      std::vector<Item> a;
      for (int i=0;i<n;++i) a.emplace_back(static_cast<int>(rng()%7),i);
      int calls=0;
      blueberry::LinearRMQ<Item,Compare> tree(a,Compare{reverse,&calls});
      if (tree.size()!=n) return 1;
      for (int t=0;t<1000 && n;++t) {
        int l=rng()%n,r=rng()%n;
        if (l>r) std::swap(l,r);
        ++r;
        if (t==0) { l=0; r=n; }
        int expected=l;
        for (int i=l+1;i<r;++i)
          if (reverse ? a[i].key>a[expected].key : a[i].key<a[expected].key) expected=i;
        calls=0;
        const int actual=tree.argmin(l,r);
        if (actual!=expected || tree.prod(l,r).identity!=expected) {
          std::cerr << "seed=" << seed << " n=" << n << " reverse=" << reverse
                    << " l=" << l << " r=" << r << " expected=" << expected
                    << " actual=" << actual << " keys=";
          for (auto v:a) std::cerr << v.key << ',';
          std::cerr << '\n'; return 1;
        }
      }
    }
    if (n>129) continue;
    std::vector<Word> words;
    for (int i=0;i<n;++i) words.emplace_back(std::vector<int>{i});
    blueberry::DisjointSparseTable dst(words,concat);
    blueberry::SqrtTree<Word,concat,empty> sqrt(words);
    for (int l=0;l<=n;++l) {
      Word expected=empty();
      if (sqrt.prod(l,l)!=expected) return 1;
      for (int r=l+1;r<=n;++r) {
        expected=concat(expected,words[r-1]);
        if (sqrt.prod(l,r)!=expected || dst.prod(l,r)!=expected) {
          std::cerr << "seed=" << seed << " concatenation n=" << n
                    << " l=" << l << " r=" << r << '\n'; return 1;
        }
      }
    }
  }
}

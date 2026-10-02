"""Generated-page cleanup and correctness of copyable documentation code."""
import importlib.util
import os
import shlex
import subprocess

from pathlib import Path
import tempfile
import unittest

import yaml

SPEC = importlib.util.spec_from_file_location(
    "generate_docs", Path(__file__).resolve().parents[1] / "scripts/generate_docs.py")
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class GeneratedDocumentationTest(unittest.TestCase):
    def test_removed_sources_are_pruned_without_touching_other_pages(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            output = root / ".verify-helper/markdown"
            names = ("blueberry/old.hpp.md", "blueberry/math/new.hpp.md",
                     "verify/math/old.test.cpp.md", "verify/math/new.test.cpp.md",
                     "migration.md", "blueberry/notes.md")
            for name in names:
                page = output / name
                page.parent.mkdir(parents=True, exist_ok=True)
                page.write_text("generated or static content")
            for name in ("blueberry/math/new.hpp", "verify/math/new.test.cpp"):
                source = root / name
                source.parent.mkdir(parents=True, exist_ok=True)
                source.write_text("source")
            self.assertCountEqual(MODULE.prune_removed_sources(root),
                                  ["blueberry/old.hpp.md", "verify/math/old.test.cpp.md"])
            for name in names:
                self.assertEqual((output / name).exists(), "old." not in name)
            self.assertEqual(MODULE.prune_removed_sources(root), [])

    def test_no_generated_directory_is_a_noop(self):
        with tempfile.TemporaryDirectory() as temporary:
            self.assertEqual(MODULE.prune_removed_sources(Path(temporary)), [])

    def test_external_output_is_not_removed(self):
        with tempfile.TemporaryDirectory() as temporary, tempfile.TemporaryDirectory() as external:
            root = Path(temporary)
            (root / ".verify-helper").mkdir()
            try:
                (root / ".verify-helper/markdown").symlink_to(external, target_is_directory=True)
            except OSError:
                self.skipTest("creating a directory symlink is unavailable")
            with self.assertRaises(ValueError):
                MODULE.prune_removed_sources(root)


# Exact copyable monoid snippets: compilation and fixed-seed brute-force checks.
ROOT = Path(__file__).resolve().parents[1]
DATA = ROOT / ".verify-helper/docs/static/_data/monoids.yml"

# These are deliberately independent of blueberry/algebra/monoids.hpp: readers
# copy the code field, so testing the library classes would miss snippet errors.
TESTS = {
    "sum": r"""
void test() {
 auto view=[](S s){return vector<long long>{s.sum,s.len};};
 for(trial=0;trial<40;++trial) {
  int n=trial%10; vector<long long>a(n); vector<S>v;
  for(int i=0;i<n;++i){a[i]=draw(21)-10;v.push_back(leaf(a[i]));}
  atcoder::lazy_segtree<S,op,e,F,mapping,composition,id> seg(v);
  for(step=0;step<80;++step){
   int l=draw(n+1),r=draw(n+1);if(l>r)swap(l,r);
   F f{draw(3)-1,draw(9)-4}; F g=id(); g={0,3};
   action_laws(v,f,g,op,e,mapping,composition,id,view);
   if(step%3){seg.apply(l,r,f);for(int i=l;i<r;++i){a[i]=f.a*a[i]+f.b;}}
   long long sum=0;
   for(int i=l;i<r;++i){sum+=a[i];}
   auto expected=vector<long long>{sum,r-l};EQ(view(seg.prod(l,r)),expected);
   if(n && step%9==0){int i=draw(n);a[i]=draw(13)-6;seg.set(i,leaf(a[i]));}
  }
 }
}
""",
    "ap": r"""
void test() {
 auto view=[](S s){return vector<long long>{s.sum,s.index_sum,s.len};};
 for(trial=0;trial<40;++trial) {
  int n=trial%10; vector<long long>a(n); vector<S>v;
  for(int i=0;i<n;++i){a[i]=draw(21)-10;v.push_back(leaf(a[i],i));}
  atcoder::lazy_segtree<S,op,e,F,mapping,composition,id> seg(v);
  for(step=0;step<80;++step){
   int l=draw(n+1),r=draw(n+1);if(l>r)swap(l,r);
   F f{draw(3)-1,draw(5)-2,draw(9)-4}; F g=id(); g={0,1,-2};
   action_laws(v,f,g,op,e,mapping,composition,id,view);
   if(step%3){seg.apply(l,r,f);for(int i=l;i<r;++i){a[i]=f.a*a[i]+f.b*i+f.c;}}
   long long sum=0;long long index=0;
   for(int i=l;i<r;++i){sum+=a[i];index+=i;}
   auto expected=vector<long long>{sum,index,r-l};EQ(view(seg.prod(l,r)),expected);
   if(n && step%9==0){int i=draw(n);a[i]=draw(13)-6;seg.set(i,leaf(a[i],i));}
  }
 }
}
""",
    "squares": r"""
void test() {
 auto view=[](S s){return vector<long long>{s.sum,s.square_sum,s.len};};
 for(trial=0;trial<40;++trial) {
  int n=trial%10; vector<long long>a(n); vector<S>v;
  for(int i=0;i<n;++i){a[i]=draw(21)-10;v.push_back(leaf(a[i]));}
  atcoder::lazy_segtree<S,op,e,F,mapping,composition,id> seg(v);
  for(step=0;step<80;++step){
   int l=draw(n+1),r=draw(n+1);if(l>r)swap(l,r);
   F f{draw(3)-1,draw(9)-4}; F g=id(); g={0,3};
   action_laws(v,f,g,op,e,mapping,composition,id,view);
   if(step%3){seg.apply(l,r,f);for(int i=l;i<r;++i){a[i]=f.a*a[i]+f.b;}}
   long long sum=0;long long square=0;
   for(int i=l;i<r;++i){sum+=a[i];square+=a[i]*a[i];}
   auto expected=vector<long long>{sum,square,r-l};EQ(view(seg.prod(l,r)),expected);
   if(n && step%9==0){int i=draw(n);a[i]=draw(13)-6;seg.set(i,leaf(a[i]));}
  }
 }
}
""",
    "inv": r"""
void test(){
 auto view=[](S s){return vector<long long>{s.zero,s.one,s.inversions};};
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<bool>a(n);vector<S>v;
  for(int i=0;i<n;++i){a[i]=draw(2);v.push_back(leaf(a[i]));}
  atcoder::lazy_segtree<S,op,e,F,mapping,composition,id>seg(v);
  for(step=0;step<80;++step){
   int l=draw(n+1),r=draw(n+1);if(l>r)swap(l,r);bool f=draw(2);
   action_laws(v,f,true,op,e,mapping,composition,id,view);
   seg.apply(l,r,f);for(int i=l;i<r;++i)a[i]=a[i]!=f;
   long long ones=0,inv=0;for(int i=l;i<r;++i){if(a[i])++ones;else inv+=ones;}
   vector<long long>expected{r-l-ones,ones,inv};EQ(view(seg.prod(l,r)),expected);
  }
 }
}
""",
    "maxsub": r"""
void test(){
 auto view=[](S s){return vector<long long>{s.sum,s.prefix,s.suffix,s.best};};
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<long long>a(n);vector<S>v;
  for(auto& x:a){x=draw(21)-10;v.push_back(leaf(x));}
  atcoder::segtree<S,op,e>seg(v);monoid_laws(v,op,e,view);
  for(step=0;step<40;++step){
   if(n){int p=draw(n);a[p]=draw(21)-10;seg.set(p,leaf(a[p]));}
   for(int l=0;l<=n;++l)for(int r=l;r<=n;++r){
    long long sum=0,prefix=0,suffix=0,best=0;
    for(int i=l;i<r;++i){sum+=a[i];prefix=max(prefix,sum);}
    long long s=0;for(int i=r;i-->l;){s+=a[i];suffix=max(suffix,s);}
    for(int i=l;i<r;++i){s=0;for(int j=i;j<r;++j){s+=a[j];best=max(best,s);}}
    vector<long long>expected{sum,prefix,suffix,best};EQ(view(seg.prod(l,r)),expected);
   }
  }
 }
}
""",
    "bracket": r"""
void test(){
 auto view=[](S s){return vector<long long>{s.sum,s.min_prefix};};
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<char>a(n);vector<S>v;
  for(auto&x:a){x=draw(2)?'(':')';v.push_back(leaf(x));}
  atcoder::segtree<S,op,e>seg(v);monoid_laws(v,op,e,view);
  for(step=0;step<40;++step){
   if(n){int p=draw(n);a[p]=a[p]=='('?')':'(';seg.set(p,leaf(a[p]));}
   for(int l=0;l<=n;++l)for(int r=l;r<=n;++r){
    long long sum=0,mn=0;for(int i=l;i<r;++i){sum+=a[i]=='('?1:-1;mn=min(mn,sum);}
    vector<long long>expected{sum,mn};EQ(view(seg.prod(l,r)),expected);
   }
  }
 }
}
""",
    "compose": r"""
void test(){
 auto view=[](S s){return vector<long long>{s.a,s.b};};
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<S>v;for(int i=0;i<n;++i)v.push_back(leaf(draw(3)-1,draw(9)-4));
  atcoder::segtree<S,op,e>seg(v);monoid_laws(v,op,e,view);
  for(step=0;step<40;++step){
   if(n){int p=draw(n);v[p]=leaf(draw(3)-1,draw(9)-4);seg.set(p,v[p]);}
   for(int l=0;l<=n;++l)for(int r=l;r<=n;++r){
    for(long long x:{-3LL,0LL,7LL}){long long ans=x;for(int i=l;i<r;++i)ans=v[i].a*ans+v[i].b;auto s=seg.prod(l,r);EQ(s.a*x+s.b,ans);}
   }
  }
 }
}
""",
    "maxcount": r"""
void test(){
 auto view=[](S s){return vector<long long>{s.count?s.value:0,s.count};};
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<long long>a(n);vector<S>v;
  for(auto&x:a){x=draw(11)-5;v.push_back(leaf(x));}
  atcoder::segtree<S,op,e>seg(v);monoid_laws(v,op,e,view);
  for(step=0;step<40;++step){
   if(n){int p=draw(n);a[p]=draw(11)-5;seg.set(p,leaf(a[p]));}
   for(int l=0;l<=n;++l)for(int r=l;r<=n;++r){
    long long mx=0,cnt=0;for(int i=l;i<r;++i)if(!cnt||a[i]>mx){mx=a[i];cnt=1;}else if(a[i]==mx)++cnt;
    vector<long long>expected{mx,cnt};EQ(view(seg.prod(l,r)),expected);
   }
  }
 }
}
""",
    "basic": r"""
void test(){
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<S>v;for(int i=0;i<n;++i)v.push_back(leaf(draw(101)-50));
  atcoder::segtree<S,op,e>seg(v);monoid_laws(v,op,e,[](S x){return x;});
  for(step=0;step<30;++step){
   if(n){int p=draw(n);v[p]=leaf(draw(101)-50);seg.set(p,v[p]);}
   for(int l=0;l<=n;++l)for(int r=l;r<=n;++r){
    long long ans=0;for(int i=l;i<r;++i)ans=std::gcd(ans,v[i]);EQ(seg.prod(l,r),ans);
   }
  }
 }
}
""",
    "run": r"""
void test(){
 auto view=[](S s){return vector<int>{s.len,s.prefix[0],s.prefix[1],s.suffix[0],s.suffix[1],s.best[0],s.best[1]};};
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<bool>a(n);vector<S>v;for(int i=0;i<n;++i){a[i]=draw(2);v.push_back(leaf(a[i]));}
  atcoder::lazy_segtree<S,op,e,F,mapping,composition,id>seg(v);
  for(step=0;step<80;++step){
   int l=draw(n+1),r=draw(n+1);if(l>r)swap(l,r);bool f=draw(2);
   action_laws(v,f,true,op,e,mapping,composition,id,view);
   seg.apply(l,r,f);for(int i=l;i<r;++i)a[i]=a[i]!=f;
   for(int ql=0;ql<=n;++ql)for(int qr=ql;qr<=n;++qr){
    S expected{};expected.len=qr-ql;
    for(int b=0;b<2;++b){
     for(int i=ql;i<qr&&a[i]==b;++i)++expected.prefix[b];
     for(int i=qr;i-->ql&&a[i]==b;)++expected.suffix[b];
     int cur=0;for(int i=ql;i<qr;++i){cur=a[i]==b?cur+1:0;expected.best[b]=max(expected.best[b],cur);}
    }
    EQ(view(seg.prod(ql,qr)),view(expected));
   }
  }
 }
}
""",
    "top2": r"""
void test(){
 auto view=[](S s){return vector<long long>{s.count[0]?s.value[0]:0,s.count[1]?s.value[1]:0,s.count[0],s.count[1]};};
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<long long>a(n);vector<S>v;for(auto&x:a){x=draw(7)-3;v.push_back(leaf(x));}
  atcoder::segtree<S,op,e>seg(v);monoid_laws(v,op,e,view);
  for(step=0;step<30;++step){
   if(n){int p=draw(n);a[p]=draw(7)-3;seg.set(p,leaf(a[p]));}
   for(int l=0;l<=n;++l)for(int r=l;r<=n;++r){
    map<long long,long long,greater<long long>>counts;for(int i=l;i<r;++i)++counts[a[i]];
    S expected{};int j=0;for(auto [x,c]:counts){if(j==2)break;expected.value[j]=x;expected.count[j++]=c;}
    EQ(view(seg.prod(l,r)),view(expected));
   }
  }
 }
}
""",
    "product": r"""
void test(){
 auto view=[](S s){return vector<long long>{s.a,s.b,s.ab,s.len};};
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<long long>a(n),b(n);vector<S>v;
  for(int i=0;i<n;++i){a[i]=draw(11)-5;b[i]=draw(11)-5;v.push_back(leaf(a[i],b[i]));}
  atcoder::lazy_segtree<S,op,e,F,mapping,composition,id>seg(v);
  for(step=0;step<80;++step){
   int l=draw(n+1),r=draw(n+1);if(l>r)swap(l,r);F f{draw(7)-3,draw(7)-3};
   action_laws(v,f,F{-1,2},op,e,mapping,composition,id,view);
   seg.apply(l,r,f);for(int i=l;i<r;++i){a[i]+=f.a;b[i]+=f.b;}
   l=draw(n+1);r=draw(n+1);if(l>r)swap(l,r);
   S expected{};expected.len=r-l;for(int i=l;i<r;++i){expected.a+=a[i];expected.b+=b[i];expected.ab+=a[i]*b[i];}
   EQ(view(seg.prod(l,r)),view(expected));
  }
 }
}
""",
    "poly": r"""
void test(){
 auto view=[](S s){return vector<long long>{s.sum,s.i1,s.i2,s.len};};
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<long long>a(n);vector<S>v;for(int i=0;i<n;++i){a[i]=draw(11)-5;v.push_back(leaf(a[i],i));}
  atcoder::lazy_segtree<S,op,e,F,mapping,composition,id>seg(v);
  for(step=0;step<80;++step){
   int l=draw(n+1),r=draw(n+1);if(l>r)swap(l,r);F f{draw(7)-3,draw(7)-3,draw(7)-3};
   action_laws(v,f,F{1,-2,3},op,e,mapping,composition,id,view);
   seg.apply(l,r,f);for(int i=l;i<r;++i)a[i]+=f.c0+f.c1*i+f.c2*i*i;
   l=draw(n+1);r=draw(n+1);if(l>r)swap(l,r);
   S expected{};expected.len=r-l;for(int i=l;i<r;++i){expected.sum+=a[i];expected.i1+=i;expected.i2+=i*i;}
   EQ(view(seg.prod(l,r)),view(expected));
  }
 }
}
""",
    "xor_sum": r"""
void test(){
 auto view=[](S s){vector<long long>v(s.ones.begin(),s.ones.end());v.push_back(s.len);return v;};
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<unsigned>a(n);vector<S>v;for(auto&x:a){x=rng()&((1U<<B)-1);v.push_back(leaf(x));}
  atcoder::lazy_segtree<S,op,e,F,mapping,composition,id>seg(v);
  for(step=0;step<80;++step){
   int l=draw(n+1),r=draw(n+1);if(l>r)swap(l,r);F f=rng()&((1U<<B)-1);
   action_laws(v,f,F{1U<<(B-1)},op,e,mapping,composition,id,view);
   seg.apply(l,r,f);for(int i=l;i<r;++i)a[i]^=f;
   l=draw(n+1);r=draw(n+1);if(l>r)swap(l,r);
   S expected{};expected.len=r-l;long long total=0;
   for(int i=l;i<r;++i){total+=a[i];for(int b=0;b<B;++b)expected.ones[b]+=(a[i]>>b)&1U;}
   EQ(view(seg.prod(l,r)),view(expected));EQ(sum(seg.prod(l,r)),total);
  }
 }
}
""",
    "hash": r"""
void test(){
 auto view=[](S s){return vector<int>{s.hash.val(),s.power.val(),s.len};};
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<int>a(n);vector<S>v;for(auto&x:a){x=draw(26)+1;v.push_back(leaf(x));}
  atcoder::segtree<S,op,e>seg(v);monoid_laws(v,op,e,view);
  for(step=0;step<30;++step){
   if(n){int p=draw(n);a[p]=draw(26)+1;seg.set(p,leaf(a[p]));}
   for(int l=0;l<=n;++l)for(int r=l;r<=n;++r){
    T h=0,p=1;for(int i=l;i<r;++i){h=h*BASE+a[i];p*=BASE;}
    vector<int>expected{h.val(),p.val(),r-l};EQ(view(seg.prod(l,r)),expected);
   }
  }
 }
}
""",
    "matrix": r"""
void test(){
 auto view=[](S s){return vector<int>{s[0][0].val(),s[0][1].val(),s[1][0].val(),s[1][1].val()};};
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<int>a(n);vector<S>v;for(auto&x:a){x=draw(10);v.push_back(leaf(x));}
  atcoder::segtree<S,op,e>seg(v);monoid_laws(v,op,e,view);
  for(step=0;step<30;++step){
   if(n){int p=draw(n);a[p]=draw(10);seg.set(p,leaf(a[p]));}
   for(int l=0;l<=n;++l)for(int r=l;r<=n;++r){
    auto s=seg.prod(l,r);
    for(int col=0;col<2;++col){
     T x=col==0,y=col==1;for(int i=l;i<r;++i){T next=a[i]*x+y;y=x;x=next;}
     EQ(s[0][col].val(),x.val());EQ(s[1][col].val(),y.val());
    }
   }
  }
 }
}
""",
    "basis": r"""
void test(){
 auto view=[](S s){return s;};
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<unsigned>a(n);vector<S>v;for(auto&x:a){x=(unsigned)draw(32);if(draw(3)==0)x|=1U<<(B-1);v.push_back(leaf(x));}
  atcoder::segtree<S,op,e>seg(v);monoid_laws(v,op,e,view);
  for(step=0;step<20;++step){
   if(n){int p=draw(n);a[p]=draw(32);seg.set(p,leaf(a[p]));}
   for(int l=0;l<=n;++l)for(int r=l;r<=n;++r){
    set<unsigned>span{0};for(int i=l;i<r;++i){auto old=span;for(auto x:old)span.insert(x^a[i]);}
    auto s=seg.prod(l,r);EQ(maximum(s),*span.rbegin());
    set<unsigned>actual{0};for(auto v:s){auto old=actual;for(auto x:old)actual.insert(x^v);}EQ(actual,span);
    // A canonical basis must not depend on insertion order, not just its maximum.
    S reverse=e();for(int i=r;i-->l;)insert(reverse,a[i]);EQ(s,reverse);
   }
  }
 }
}
""",
    "gcdadd": r"""
void test(){
 auto view=[](S s){return vector<long long>{s.first,s.last,s.d,s.empty};};
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<long long>a(n);vector<S>v;for(auto&x:a){x=draw(21)-10;v.push_back(leaf(x));}
  atcoder::lazy_segtree<S,op,e,F,mapping,composition,id>seg(v);
  for(step=0;step<80;++step){
   int l=draw(n+1),r=draw(n+1);if(l>r)swap(l,r);F f=draw(11)-5;
   action_laws(v,f,F{-3},op,e,mapping,composition,id,view);
   seg.apply(l,r,f);for(int i=l;i<r;++i)a[i]+=f;
   l=draw(n+1);r=draw(n+1);if(l>r)swap(l,r);
   long long expected=0,d=0;for(int i=l;i<r;++i){expected=std::gcd(expected,a[i]);if(i>l)d=std::gcd(d,a[i]-a[i-1]);}
   auto s=seg.prod(l,r);EQ(gcd(s),expected);EQ(s.empty,l==r);
   if(l<r){EQ(s.first,a[l]);EQ(s.last,a[r-1]);EQ(s.d,d);}
  }
 }
}
""",
    "mex": r"""
void test(){
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<S>v;for(int i=0;i<n;++i)v.push_back(leaf(draw(3)));
  atcoder::segtree<S,op,e>seg(v);monoid_laws(v,op,e,[](S x){return x;});
  for(step=0;step<80;++step){
   if(n){int p=draw(n);v[p]=leaf(draw(3));seg.set(p,v[p]);}
   int expected=0;while(expected<n&&v[expected]>0)++expected;
   EQ(seg.max_right(0,[](S s){return s>0;}),expected);
   for(int l=0;l<=n;++l)for(int r=l;r<=n;++r){
    long long mn=numeric_limits<long long>::max();for(int i=l;i<r;++i)mn=min(mn,v[i]);EQ(seg.prod(l,r),mn);
   }
  }
 }
}
""",
    "cover": r"""
void test(){
 auto view=[](S s){return vector<long long>{s.minimum,s.min_length,s.length};};
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<long long>a(n),length(n);vector<S>v;
  for(int i=0;i<n;++i){length[i]=draw(7)+1;v.push_back(leaf(length[i]));}
  atcoder::lazy_segtree<S,op,e,F,mapping,composition,id>seg(v);
  for(step=0;step<80;++step){
   int l=draw(n+1),r=draw(n+1);if(l>r)swap(l,r);F f=1;
   if(l<r&&*min_element(a.begin()+l,a.begin()+r)>0&&draw(2))f=-1;
   action_laws(v,F{1},F{2},op,e,mapping,composition,id,view);
   seg.apply(l,r,f);for(int i=l;i<r;++i)a[i]+=f;
   l=draw(n+1);r=draw(n+1);if(l>r)swap(l,r);
   long long mn=0,mnlen=0,total=0,ans=0;
   if(l<r)mn=*min_element(a.begin()+l,a.begin()+r);
   for(int i=l;i<r;++i){total+=length[i];if(a[i]==mn)mnlen+=length[i];if(a[i]>0)ans+=length[i];}
   vector<long long>expected{mn,mnlen,total};auto s=seg.prod(l,r);EQ(view(s),expected);EQ(covered(s),ans);
  }
 }
}
""",
}

TESTS.update({
    "add_min": r"""
void test(){
 auto view=[](S s){return vector<long long>{s.empty?0:s.minimum,s.empty};};
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<long long>a(n);vector<S>v;
  for(auto& x:a){x=draw(21)-10;v.push_back(S{x,false});}
  atcoder::lazy_segtree<S,op,e,F,mapping,composition,id>seg(v);
  for(step=0;step<80;++step){
   int l=draw(n+1),r=draw(n+1);if(l>r)swap(l,r);
   F f=draw(11)-5;
   action_laws(v,f,F{0},op,e,mapping,composition,id,view);
   action_laws(v,f,F{-3},op,e,mapping,composition,id,view);
   seg.apply(l,r,f);for(int i=l;i<r;++i){a[i]+=f;}
   l=draw(n+1);r=draw(n+1);if(l>r)swap(l,r);
   long long mn=0,cnt=0;for(int i=l;i<r;++i){if(!cnt||a[i]<mn){mn=a[i];cnt=1;}else if(a[i]==mn)++cnt;}
   vector<long long>expected{mn,l==r};EQ(view(seg.prod(l,r)),expected);
   if(n&&step%9==0){int p=draw(n);a[p]=draw(9)-4;seg.set(p,S{a[p],false});}
  }
 }
}
""",
    "assign_min": r"""
void test(){
 auto view=[](S s){return vector<long long>{s.empty?0:s.minimum,s.empty};};
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<long long>a(n);vector<S>v;
  for(auto& x:a){x=draw(21)-10;v.push_back(S{x,false});}
  atcoder::lazy_segtree<S,op,e,F,mapping,composition,id>seg(v);
  for(step=0;step<80;++step){
   int l=draw(n+1),r=draw(n+1);if(l>r)swap(l,r);
   F f=step%4==0?F{}:F{draw(11)-5};
   action_laws(v,f,F{0},op,e,mapping,composition,id,view);
   action_laws(v,f,F{-3},op,e,mapping,composition,id,view);
   seg.apply(l,r,f);for(int i=l;i<r;++i){if(f)a[i]=*f;}
   l=draw(n+1);r=draw(n+1);if(l>r)swap(l,r);
   long long mn=0,cnt=0;for(int i=l;i<r;++i){if(!cnt||a[i]<mn){mn=a[i];cnt=1;}else if(a[i]==mn)++cnt;}
   vector<long long>expected{mn,l==r};EQ(view(seg.prod(l,r)),expected);
   if(n&&step%9==0){int p=draw(n);a[p]=draw(9)-4;seg.set(p,S{a[p],false});}
  }
 }
}
""",
    "min_count_add": r"""
void test(){
 auto view=[](S s){return vector<long long>{s.count?s.minimum:0,s.count};};
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<long long>a(n);vector<S>v;
  for(auto& x:a){x=draw(21)-10;v.push_back(S{x,1});}
  atcoder::lazy_segtree<S,op,e,F,mapping,composition,id>seg(v);
  for(step=0;step<80;++step){
   int l=draw(n+1),r=draw(n+1);if(l>r)swap(l,r);
   F f=draw(11)-5;
   action_laws(v,f,F{0},op,e,mapping,composition,id,view);
   action_laws(v,f,F{-3},op,e,mapping,composition,id,view);
   seg.apply(l,r,f);for(int i=l;i<r;++i){a[i]+=f;}
   l=draw(n+1);r=draw(n+1);if(l>r)swap(l,r);
   long long mn=0,cnt=0;for(int i=l;i<r;++i){if(!cnt||a[i]<mn){mn=a[i];cnt=1;}else if(a[i]==mn)++cnt;}
   vector<long long>expected{mn,cnt};EQ(view(seg.prod(l,r)),expected);
   if(n&&step%9==0){int p=draw(n);a[p]=draw(9)-4;seg.set(p,S{a[p],1});}
  }
 }
}
""",
    "affine_extrema": r"""
void test(){
 auto view=[](S s){return vector<long long>{s.empty?0:s.minimum,s.empty?0:s.maximum,s.empty};};
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<long long>a(n);vector<S>v;
  for(auto&x:a){x=draw(21)-10;v.push_back(S{x,x,false});}
  atcoder::lazy_segtree<S,op,e,F,mapping,composition,id>seg(v);
  for(step=0;step<80;++step){
   int l=draw(n+1),r=draw(n+1);if(l>r)swap(l,r);F f{draw(3)-1,draw(11)-5};
   action_laws(v,f,F{-1,3},op,e,mapping,composition,id,view);
   action_laws(v,f,F{0,-2},op,e,mapping,composition,id,view);
   seg.apply(l,r,f);for(int i=l;i<r;++i)a[i]=f.a*a[i]+f.b;
   l=draw(n+1);r=draw(n+1);if(l>r)swap(l,r);
   long long mn=0,mx=0;for(int i=l;i<r;++i){if(i==l)mn=mx=a[i];else{mn=min(mn,a[i]);mx=max(mx,a[i]);}}
   vector<long long>expected{mn,mx,l==r};EQ(view(seg.prod(l,r)),expected);
  }
 }
}
""",
    "binary_assign_flip": r"""
void test(){
 auto view=[](S s){return vector<long long>{s.ones,s.len};};
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<bool>a(n);vector<S>v;
  for(int i=0;i<n;++i){a[i]=draw(2);v.push_back(S{a[i],1});}
  atcoder::lazy_segtree<S,op,e,F,mapping,composition,id>seg(v);
  for(int f=0;f<4;++f)for(int g=0;g<4;++g)
   action_laws(v,F{bool(f&1),bool(f&2)},F{bool(g&1),bool(g&2)},op,e,mapping,composition,id,view);
  for(step=0;step<80;++step){
   int l=draw(n+1),r=draw(n+1);if(l>r)swap(l,r);F f{bool(draw(2)),bool(draw(2))};
   seg.apply(l,r,f);for(int i=l;i<r;++i)a[i]=a[i]?f.one:f.zero;
   l=draw(n+1);r=draw(n+1);if(l>r)swap(l,r);
   long long ones=0;for(int i=l;i<r;++i)ones+=a[i];
   vector<long long>expected{ones,r-l};EQ(view(seg.prod(l,r)),expected);
  }
 }
}
""",
    "digit_assign": r"""
void test(){
 auto view=[](S s){return vector<int>{s.value.val(),s.power.val(),s.repunit.val(),s.len};};
 for(trial=0;trial<48;++trial){
  int n=trial%12;vector<int>a(n);vector<S>v;
  for(auto&x:a){x=trial%3==0?9:draw(10);v.push_back(S{x,10,1,1});}
  atcoder::lazy_segtree<S,op,e,F,mapping,composition,id>seg(v);
  for(step=0;step<80;++step){
   int l=draw(n+1),r=draw(n+1);if(l>r)swap(l,r);F f=step%4==0?F{}:F{draw(10)};
   action_laws(v,f,F{0},op,e,mapping,composition,id,view);
   action_laws(v,f,F{9},op,e,mapping,composition,id,view);
   seg.apply(l,r,f);for(int i=l;i<r;++i)if(f)a[i]=*f;
   for(int ql=0;ql<=n;++ql)for(int qr=ql;qr<=n;++qr){
    long long value=0,power=1,repunit=0;
    for(int i=ql;i<qr;++i){value=(value*10+a[i])%998244353;power=power*10%998244353;repunit=(repunit*10+1)%998244353;}
    vector<int>expected{int(value),int(power),int(repunit),qr-ql};EQ(view(seg.prod(ql,qr)),expected);
   }
  }
 }
}
""",
    "weighted_add": r"""
void test(){
 auto view=[](S s){return vector<long long>{s.sum,s.weights};};
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<long long>a(n),weight(n);vector<S>v;
  for(int i=0;i<n;++i){a[i]=draw(21)-10;weight[i]=draw(7)-3;v.push_back(S{a[i]*weight[i],weight[i]});}
  atcoder::lazy_segtree<S,op,e,F,mapping,composition,id>seg(v);
  for(step=0;step<80;++step){
   int l=draw(n+1),r=draw(n+1);if(l>r)swap(l,r);F f=draw(11)-5;
   action_laws(v,f,F{-3},op,e,mapping,composition,id,view);
   seg.apply(l,r,f);for(int i=l;i<r;++i)a[i]+=f;
   l=draw(n+1);r=draw(n+1);if(l>r)swap(l,r);
   long long sum=0,weights=0;for(int i=l;i<r;++i){sum+=a[i]*weight[i];weights+=weight[i];}
   vector<long long>expected{sum,weights};EQ(view(seg.prod(l,r)),expected);
  }
 }
}
""",
    "add_variation": r"""
void test(){
 auto view=[](S s){return vector<long long>{s.empty?0:s.first,s.empty?0:s.last,s.variation,s.empty};};
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<long long>a(n);vector<S>v;
  for(auto&x:a){x=draw(21)-10;v.push_back(S{x,x,0,false});}
  atcoder::lazy_segtree<S,op,e,F,mapping,composition,id>seg(v);
  for(step=0;step<80;++step){
   int l=draw(n+1),r=draw(n+1);if(l>r)swap(l,r);F f=draw(11)-5;
   action_laws(v,f,F{-3},op,e,mapping,composition,id,view);
   seg.apply(l,r,f);for(int i=l;i<r;++i)a[i]+=f;
   // Query beyond the update so both changed boundary differences are exercised.
   for(int ql=0;ql<=n;++ql)for(int qr=ql;qr<=n;++qr){
    long long variation=0;for(int i=ql+1;i<qr;++i)variation+=std::abs(a[i]-a[i-1]);
    vector<long long>expected{ql==qr?0:a[ql],ql==qr?0:a[qr-1],variation,ql==qr};
    EQ(view(seg.prod(ql,qr)),expected);
   }
  }
 }
}
""",
    "assign_maxsub": r"""
void test(){
 auto view=[](S s){return vector<long long>{s.sum,s.prefix,s.suffix,s.best,s.len};};
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<long long>a(n);vector<S>v;
  for(auto&x:a){x=draw(21)-10;long long p=max(0LL,x);v.push_back(S{x,p,p,p,1});}
  atcoder::lazy_segtree<S,op,e,F,mapping,composition,id>seg(v);
  for(step=0;step<80;++step){
   int l=draw(n+1),r=draw(n+1);if(l>r)swap(l,r);F f=step%4==0?F{}:F{draw(11)-5};
   action_laws(v,f,F{0},op,e,mapping,composition,id,view);
   action_laws(v,f,F{-3},op,e,mapping,composition,id,view);
   seg.apply(l,r,f);for(int i=l;i<r;++i)if(f)a[i]=*f;
   for(int ql=0;ql<=n;++ql)for(int qr=ql;qr<=n;++qr){
    long long sum=0,prefix=0,suffix=0,best=0,cur=0;
    for(int i=ql;i<qr;++i){sum+=a[i];prefix=max(prefix,sum);}
    for(int i=qr;i-->ql;){cur+=a[i];suffix=max(suffix,cur);}
    for(int i=ql;i<qr;++i){cur=0;for(int j=i;j<qr;++j){cur+=a[j];best=max(best,cur);}}
    vector<long long>expected{sum,prefix,suffix,best,qr-ql};EQ(view(seg.prod(ql,qr)),expected);
   }
  }
 }
}
""",
    "minplus": r"""
void test(){
 auto sample=[](){S s{};for(auto&row:s)for(auto&x:row)x=draw(4)==0?INF:draw(17)-7;return s;};
 for(trial=0;trial<40;++trial){
  int n=trial%10;vector<S>v;for(int i=0;i<n;++i)v.push_back(sample());
  atcoder::segtree<S,op,e>seg(v);monoid_laws(v,op,e,[](S s){return s;});
  for(step=0;step<30;++step){
   if(n){int p=draw(n);v[p]=sample();seg.set(p,v[p]);}
   for(int l=0;l<=n;++l)for(int r=l;r<=n;++r){
    S actual=seg.prod(l,r);
    // Enumerate the two-state layered DP independently of matrix multiplication.
    for(int start=0;start<2;++start){
     array<long long,2>dp{INF,INF};dp[start]=0;
     for(int i=l;i<r;++i){
      array<long long,2>next{INF,INF};
      for(int from=0;from<2;++from)for(int to=0;to<2;++to)
       if(dp[from]!=INF&&v[i][to][from]!=INF)next[to]=min(next[to],dp[from]+v[i][to][from]);
      dp=next;
     }
     for(int to=0;to<2;++to)EQ(actual[to][start],dp[to]);
    }
   }
  }
 }
}
""",
})

PRELUDE = r"""
#include <bits/stdc++.h>
#include <atcoder/segtree>
#include <atcoder/lazysegtree>
#include <atcoder/modint>
using namespace std;
constexpr unsigned seed = 20260930;
mt19937 rng(seed);
string recipe;
int trial = 0, step = 0;
int draw(int n) { return int(rng() % n); }
template<class T> void dump(const T& x) {
    if constexpr (requires { x.val(); }) cerr << x.val();
    else if constexpr (requires { x.begin(); x.end(); }) {
        cerr << '['; for (const auto& y : x) { dump(y); cerr << ','; } cerr << ']';
    } else cerr << x;
}
template<class A, class B> void equal_to_or_die(const A& a, const B& b, int line) {
    if (a == b) return;
    cerr << "recipe=" << recipe << " seed=" << seed << " trial=" << trial
         << " step=" << step << " line=" << line << " actual=";
    dump(a); cerr << " expected="; dump(b); cerr << '\n'; abort();
}
#define EQ(a,b) equal_to_or_die((a),(b),__LINE__)
template<class S, class Op, class E, class View>
void monoid_laws(const vector<S>& v, Op op, E e, View view) {
    S x=e(), y=e(), z=e();
    if (!v.empty()) x=v[0];
    if (v.size()>1) y=v[1];
    if (v.size()>2) z=v[2];
    for (auto s : {e(),x,y,op(x,y),op(op(x,y),z)}) {
        EQ(view(op(s,e())),view(s)); EQ(view(op(e(),s)),view(s));
    }
    EQ(view(op(op(x,y),z)),view(op(x,op(y,z))));
}
template<class S, class F, class Op, class E, class Map, class Comp, class Id, class View>
void action_laws(const vector<S>& v, F f, F g, Op op, E e, Map mapping,
                 Comp composition, Id id, View view) {
    monoid_laws(v,op,e,view);
    S x=e(), y=e();
    if(!v.empty()) x=v[0];
    if(v.size()>1) y=v[1];
    for(auto s : {e(),x,op(x,y)}) {
        EQ(view(mapping(id(),s)),view(s));
        EQ(view(mapping(composition(f,g),s)),view(mapping(f,mapping(g,s))));
        EQ(view(mapping(composition(f,id()),s)),view(mapping(f,s)));
        EQ(view(mapping(composition(id(),f),s)),view(mapping(f,s)));
    }
    EQ(view(mapping(f,e())),view(e()));
    EQ(view(mapping(f,op(x,y))),view(op(mapping(f,x),mapping(f,y))));
}
"""


class MonoidSnippetTest(unittest.TestCase):
    def test_exact_catalog_code_and_random_oracles(self):
        recipes = yaml.safe_load(DATA.read_text(encoding="utf-8"))
        snippets = {r["id"]: r for r in recipes if r.get("code")}
        self.assertEqual(set(snippets), set(TESTS), "Every published snippet needs an oracle")
        compiler = os.environ.get("CXX", "g++")
        flags = [compiler, "-std=" + os.environ.get("CXX_STANDARD", "gnu++20"),
                 "-O2", "-Wall", "-Wextra", "-Werror", "-D_GLIBCXX_ASSERTIONS",
                 *shlex.split(os.environ.get("CPPFLAGS", "")), "-I", str(ROOT / ".deps/ac-library")]
        with tempfile.TemporaryDirectory(prefix="monoid-snippets-") as temporary:
            temporary = Path(temporary)
            source = PRELUDE
            isolated = []
            for name, entry in snippets.items():
                code = entry["code"]
                # Compile each block BEFORE including ACL to catch missing standard
                # includes which the combined test's prelude could otherwise hide.
                unit = temporary / (name + ".cpp")
                tree = ("atcoder::lazy_segtree<S,op,e,F,mapping,composition,id>"
                        if entry["kind"] == "lazy" else "atcoder::segtree<S,op,e>")
                unit.write_text(code + '\n#include <atcoder/segtree>\n#include <atcoder/lazysegtree>\n'
                                + f"int main() {{ {tree} seg(0); (void)seg; }}\n", encoding="utf-8")
                isolated.append(str(unit))
                # Leaf/result convenience functions are deliberately absent from
                # the reader's S/F block. They only adapt test inputs and outputs.
                helpers = entry.get("test_helpers", "")
                source += f"\nnamespace recipe_{name} {{\n{code}\n{helpers}\n{TESTS[name]}\n}}\n"
            subprocess.run([*flags, "-fsyntax-only", *isolated], check=True, timeout=120)
            source += '\nint main() {\n'
            for name in snippets:
                source += f'  recipe="{name}"; recipe_{name}::test();\n'
            source += '}\n'
            program = temporary / "all.cpp"
            program.write_text(source, encoding="utf-8")
            executable = temporary / "all"
            subprocess.run([*flags, str(program), "-o", str(executable)], check=True, timeout=120)
            subprocess.run([str(executable)], check=True, timeout=30)


if __name__ == "__main__":
    unittest.main()

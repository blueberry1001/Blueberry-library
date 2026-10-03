#include <algorithm>
#include <bit>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <utility>
#include <vector>
#include <atcoder/modint>
#include "blueberry/graph/static-top-tree.hpp"
using Mint=atcoder::modint998244353;
struct Affine{Mint a,b;};
int calls=0;
Affine vertex(Affine v,Mint light){++calls;return {v.a,v.a*light+v.b};}
Mint edge(Affine p){++calls;return p.b;}
Affine compress(Affine x,Affine y){++calls;return {x.a*y.a,x.a*y.b+x.b};}
Mint rake(Mint x,Mint y){++calls;return x+y;}
Mint identity(){return 0;}
using Graph=std::vector<std::vector<int>>;
using Tree=blueberry::StaticTopTree<Affine,Affine,Mint,vertex,edge,compress,rake,identity>;
unsigned seed;
int trial=-1,step=-1;

void require(bool ok,const char* message){
 if(!ok){std::cerr<<"seed="<<seed<<" trial="<<trial<<" step="<<step<<" "<<message<<'\n';std::abort();}
}

// Ordinary rooted DP: no heavy paths, affine compression, or rake expressions.
Mint brute(const Graph& g,const std::vector<Affine>& a,int root){
 auto dfs=[&](auto&& self,int v,int parent)->Mint{
  Mint sum=0;
  for(int w:g[v])if(w!=parent)sum+=self(self,w,v);
  return a[v].a*sum+a[v].b;
 };
 return dfs(dfs,root,-1);
}

void check(const Tree& t,const Graph& g,const std::vector<Affine>& a,int root){
 require(t.size()==int(a.size()),"size mismatch");
 for(int v=0;v<t.size();++v){
  Affine value=t.get(v);
  require(value.a==a[v].a && value.b==a[v].b,"get mismatch");
 }
 Mint expected=brute(g,a,root),actual=t.all_prod().b;
 if(actual!=expected){
  std::cerr<<"root="<<root<<" expected="<<expected.val()<<" actual="<<actual.val()<<'\n';
  for(int v=0;v<int(g.size());++v){
   std::cerr<<v<<": a="<<a[v].a.val()<<" b="<<a[v].b.val()<<" neighbors=";
   for(int w:g[v])std::cerr<<w<<',';
   std::cerr<<'\n';
  }
  require(false,"rooted DP mismatch");
 }
}

void update(Tree& t,const Graph& g,std::vector<Affine>& a,int root,int v,Affine value){
 calls=0;t.set(v,value);
 require(calls<=20*int(std::bit_width(unsigned(t.size())))+10,"too many update callbacks");
 a[v]=value;check(t,g,a,root);
}

void boundary_cases(){
 Graph one(1);std::vector<Affine> single{{7,9}};Tree singleton(one,single);
 check(singleton,one,single,0);
 update(singleton,one,single,0,0,{3,4});
 update(singleton,one,single,0,0,{3,4});
 update(singleton,one,single,0,0,{0,0});
 update(singleton,one,single,0,0,{0,11});

 // Compression must follow the root-to-leaf direction, even when root != 0.
 Graph chain{{1},{0,2},{1}};std::vector<Affine> a{{2,1},{3,4},{5,6}};
 Tree t(chain,a,2);check(t,chain,a,2);require(t.all_prod().b==41,"chain initial value");
 update(t,chain,a,2,0,{7,2});require(t.all_prod().b==56,"chain leaf update");
 update(t,chain,a,2,1,{11,3});require(t.all_prod().b==131,"chain internal update");
 update(t,chain,a,2,2,{13,5});require(t.all_prod().b==330,"chain root update");
 update(t,chain,a,2,2,t.get(2));

 // Both public accessors return independent values.
 Affine vertex_copy=t.get(1),path_copy=t.all_prod();
 vertex_copy.b+=1;path_copy.b+=1;
 require(vertex_copy.b!=t.get(1).b && path_copy.b!=t.all_prod().b,"accessor aliasing");
 check(t,chain,a,2);
}

void ownership_cases(){
 const Graph g{{1,2},{0,3},{0},{1}};
 std::vector<Affine> a{{2,1},{3,4},{5,6},{7,8}};
 // Constructor inputs are changed and destroyed before subsequent operations.
 Tree t=[&]{
  Graph input_graph=g;auto input_values=a;
  Tree result(input_graph,input_values,2);
  input_graph.assign(1,{});input_values.assign(1,Affine{0,0});
  check(result,g,a,2);
  return result;
 }();
 check(t,g,a,2);
 update(t,g,a,2,3,{7,10});

 Tree copied(t);auto copied_values=a;
 update(copied,g,copied_values,2,0,{11,12});
 check(t,g,a,2);
 update(t,g,a,2,2,{0,13});
 check(copied,g,copied_values,2);

 // Assignment also replaces a previous, differently sized topology.
 const Graph one(1);const std::vector<Affine> single{{1,9}};
 Tree assigned(one,single);assigned=t;auto assigned_values=a;
 check(assigned,g,assigned_values,2);
 update(assigned,g,assigned_values,2,1,{0,0});
 check(t,g,a,2);

 Tree moved(std::move(copied));check(moved,g,copied_values,2);
 update(moved,g,copied_values,2,3,{0,17});
 check(t,g,a,2);check(assigned,g,assigned_values,2);
 Tree move_assigned(one,single);move_assigned=std::move(moved);
 check(move_assigned,g,copied_values,2);
 update(move_assigned,g,copied_values,2,2,{2,19});
 check(t,g,a,2);
 // A moved-from object is only destroyed or assigned before reuse.
 copied=t;check(copied,g,a,2);
 moved=Tree(one,single);check(moved,one,single,0);
 assigned=Tree(one,single);check(assigned,one,single,0);
 check(move_assigned,g,copied_values,2);
}

int main(int argc,char**argv){
 seed=argc>1?std::strtoul(argv[1],nullptr,10):20260919;std::mt19937 rng(seed);
 boundary_cases();ownership_cases();
 for(trial=0;trial<150;++trial){
  int n=1+rng()%150;Graph g(n);std::vector<int> label(n);
  std::iota(label.begin(),label.end(),0);std::shuffle(label.begin(),label.end(),rng);
  for(int v=1;v<n;++v){
   int p=trial%4==0?0:trial%4==1?v-1:trial%4==2?(v-1)/2:rng()%v;
   g[label[v]].push_back(label[p]);g[label[p]].push_back(label[v]);
  }
  for(auto& neighbors:g)std::shuffle(neighbors.begin(),neighbors.end(),rng);
  int root=rng()%n,leaf=root,internal=root;
  for(int v=0;v<n;++v)if(v!=root){
   if(g[v].size()==1)leaf=v;
   else internal=v;
  }
  std::vector<Affine>a(n);for(auto&x:a)x={rng()%17,rng()%23};
  Tree t(g,a,root);step=-1;check(t,g,a,root);
  for(step=0;step<300;++step){
   int v=step==0?root:step==1?leaf:step==2?internal:rng()%n;
   Affine value{rng()%17,rng()%23};
   if(step%17==3)value=a[v];
   if(step%17==4)value={0,0};
   if(step%17==5)value.a=0;
   update(t,g,a,root,v,value);
  }
 }
 for(int shape=0;shape<3;++shape){
  trial=150+shape;step=-1;int n=100000;Graph g(n);
  for(int v=1;v<n;++v){int p=shape==0?0:shape==1?v-1:(v-1)/2;g[v].push_back(p);g[p].push_back(v);}
  std::vector<Affine>a(n,Affine{1,1});Tree t(g,a);
  require(t.size()==n && t.all_prod().b==n,"large initial value");
  for(int v:{0,n/2,n-1}){
   step=v;calls=0;t.set(v,{1,2});
   require(calls<=20*int(std::bit_width(unsigned(n)))+10,"large update callbacks");
   require(t.get(v).a==1 && t.get(v).b==2,"large get after update");
  }
  require(t.all_prod().b==n+3,"large final value");
 }
}

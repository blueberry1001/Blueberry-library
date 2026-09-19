#include "blueberry/data-structure/slope-trick.hpp"
#include "blueberry/data-structure/splay-slope-trick.hpp"
#include "blueberry/math/fraction.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <utility>
#include <vector>
using ll=long long;
unsigned long long seed;int step,round_id;
void check(bool ok,const char*what){if(!ok){std::cerr<<"seed="<<seed<<" round="<<round_id<<" step="<<step<<" operation="<<what<<'\n';std::exit(1);}}
struct Brute{int l;std::vector<ll>v;int r()const{return l+static_cast<int>(v.size())-1;}};
using Splay=blueberry::SplaySlopeTrick<ll>;
void compare(Splay& f,const Brute& b){
 check(f.empty()==b.v.empty(),"empty");if(b.v.empty()){check(!f.min()&&!f.argmin()&&!f.domain()&&!f.eval(0),"empty queries");return;}
 auto d=f.domain();check(d&&d->first==b.l&&d->second==b.r(),"domain");
 ll minimum=*std::min_element(b.v.begin(),b.v.end());int lo=-1,hi=-1;
 for(int x=b.l;x<=b.r();++x){auto actual=f.eval(x);ll expected=b.v[x-b.l];if(!actual||*actual!=expected){std::cerr<<"x="<<x<<" expected="<<expected<<" actual="<<(actual?*actual:999999999999LL)<<" input=";for(ll y:b.v)std::cerr<<y<<',';std::cerr<<'\n';check(false,"eval");}if(expected==minimum){if(lo==-1)lo=x-b.l;hi=x-b.l;}}
 check(f.min()==minimum,"minimum");auto a=f.argmin();check(a&&a->first==b.l+lo&&a->second==b.l+hi,"argmin plateau");check(!f.eval(b.l-1)&&!f.eval(b.r()+1),"outside");
}
Brute convolution(const Brute&a,const Brute&b){Brute c{a.l+b.l,{}};if(a.v.empty()||b.v.empty())return c;c.v.assign(a.v.size()+b.v.size()-1,std::numeric_limits<ll>::max());for(std::size_t i=0;i<a.v.size();++i)for(std::size_t j=0;j<b.v.size();++j)c.v[i+j]=std::min(c.v[i+j],a.v[i]+b.v[j]);return c;}
int main(int argc,char**argv){seed=argc>1?std::strtoull(argv[1],nullptr,10):1;std::mt19937_64 rng(seed);
 for(round_id=0;round_id<8;++round_id){blueberry::SlopeTrick<ll> f;constexpr int limit=700;constexpr ll inf=1000000000000000LL;std::vector<ll>v(2*limit+1,0);
 for(step=0;step<120;++step){int a=static_cast<int>(rng()%41)-20;switch(rng()%8){
 case 0:f.add_abs(a);for(int x=-limit;x<=limit;++x)v[x+limit]+=std::abs(x-a);break;
 case 1:f.add_x_minus_a(a);for(int x=-limit;x<=limit;++x)v[x+limit]+=std::max(0,x-a);break;
 case 2:f.add_a_minus_x(a);for(int x=-limit;x<=limit;++x)v[x+limit]+=std::max(0,a-x);break;
 case 3:f.add_const(a);for(auto&y:v)y+=a;break;
 case 4:{int l=static_cast<int>(rng()%5)-2,r=l+rng()%4;f.shift(l,r);std::vector<ll>w(v.size(),inf);for(int x=-limit;x<=limit;++x)for(int y=x-r;y<=x-l;++y)if(-limit<=y&&y<=limit)w[x+limit]=std::min(w[x+limit],v[y+limit]);v=std::move(w);break;}
 case 5:f.prefix_min();for(std::size_t i=1;i<v.size();++i)v[i]=std::min(v[i],v[i-1]);break;
 case 6:f.suffix_min();for(std::size_t i=v.size()-1;i>0;--i)v[i-1]=std::min(v[i-1],v[i]);break;
 default:{blueberry::SlopeTrick<ll>g;g.add_abs(a);g.add_const(3);f.merge(g);check(g.size()==0&&g.eval(0)==0,"pq consumed merge");for(int x=-limit;x<=limit;++x)v[x+limit]+=std::abs(x-a)+3;}
 }
 for(int x=-40;x<=40;++x){check(f.eval(x)==v[x+limit],"pq eval");}check(f.min()==*std::min_element(v.begin(),v.end()),"pq minimum");auto [l,r]=f.argmin();if(l)check(f.eval(*l)==f.min(),"pq left argmin");if(r)check(f.eval(*r)==f.min(),"pq right argmin");
 }
 auto copy=f;auto moved=std::move(copy);check(copy.size()==0&&copy.min()==0,"pq move");check(moved.eval(0)==f.eval(0),"pq copy");f.clear();check(!f.argmin().first&&!f.argmin().second&&f.min()==0,"pq clear");}
 for(round_id=0;round_id<35;++round_id){Splay f(-5,5);Brute b{-5,std::vector<ll>(11,0)};
 for(step=0;step<80;++step){if(b.v.empty()){f=Splay(-3,3);b={-3,std::vector<ll>(7,0)};}int a=static_cast<int>(rng()%31)-15;ll w=rng()%1000001;switch(rng()%9){
 case 0:f.add_abs(a,w);for(int x=b.l;x<=b.r();++x)b.v[x-b.l]+=w*std::abs(x-a);break;
 case 1:f.add_x_minus_a(a,w);for(int x=b.l;x<=b.r();++x)b.v[x-b.l]+=w*std::max(0,x-a);break;
 case 2:f.add_a_minus_x(a,w);for(int x=b.l;x<=b.r();++x)b.v[x-b.l]+=w*std::max(0,a-x);break;
 case 3:{int k=static_cast<int>(rng()%9)-4;f.add_linear(k,a);for(int x=b.l;x<=b.r();++x)b.v[x-b.l]+=k*x+a;break;}
 case 4:{int dx=static_cast<int>(rng()%5)-2;f.shift(dx,a);b.l+=dx;for(auto&y:b.v)y+=a;break;}
 case 5:{int l=b.l-1+rng()%(b.v.size()+3),r=l+static_cast<int>(rng()%8);f.restrict(l,r);std::vector<ll>next;int start=std::max(l,b.l);for(int x=start;x<=std::min(r,b.r());++x)next.push_back(b.v[x-b.l]);b={start,next};break;}
 case 6:{int l=static_cast<int>(rng()%5)-2,r=l+rng()%5,k=static_cast<int>(rng()%9)-4;f.convolve_segment(l,r,k,a);Brute g{l,{}};for(int x=l;x<=r;++x)g.v.push_back(k*x+a);b=convolution(b,g);break;}
 case 7:{Splay g(b.l,b.r());g.add_abs(a,3);f.merge(g);check(g.empty(),"splay merge consumed");for(int x=b.l;x<=b.r();++x)b.v[x-b.l]+=3*std::abs(x-a);break;}
 default:{std::vector<std::pair<ll,ll>>points{{-2,3},{-1,1},{0,1},{1,4},{2,8}};auto g=Splay::from_points(points);f.convolve(g);check(g.empty(),"splay convolve consumed");b=convolution(b,Brute{-2,{3,1,1,4,8}});}
 }compare(f,b);}
 auto moved=std::move(f);check(f.empty(),"splay move source");compare(moved,b);f=std::move(moved);check(moved.empty(),"splay move assignment");compare(f,b);}
 using Q=blueberry::Fraction;using Rational=blueberry::SplaySlopeTrick<Q>;auto rational=Rational::from_points({{0,0},{2,1},{4,3}});rational.add_linear(Q(-3,4),0);check(rational.min()==Q(-1,2),"fractional minimum");rational.add_abs(Q(3,2),Q(1,4));auto arg=rational.argmin();check(arg&&arg->first==Q(3,2)&&arg->second==Q(2),"fractional plateau");check(rational.min()==Q(-3,8),"fractional weighted minimum");
 auto fractional_convolution = Rational::from_points({{-1,Q(1,2)},{0,0},{1,Q(1,2)}});
 fractional_convolution.convolve_segment(0,1,Q(1,4),0);
 check(fractional_convolution.eval(Q(1,4))==Q(1,16),"fractional segment convolution interior");
 check(fractional_convolution.eval(2)==Q(3,4),"fractional segment convolution endpoint");
 check(fractional_convolution.argmin()==std::pair<Q,Q>{0,0},"fractional segment convolution minimum");
 Splay empty;auto one=Splay::from_points({{4,9}});empty.convolve(one);check(empty.empty()&&one.empty(),"empty convolution");
}


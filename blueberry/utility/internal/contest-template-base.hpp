#pragma once
#include <bits/stdc++.h>
#include <atcoder/all>
#include "blueberry/utility/internal/contest-helpers-base.hpp"
using namespace std;
using namespace atcoder;
using ll=long long;
using ld=long double;
using ull=unsigned long long;
using uint=unsigned int;
using str=string;
using mint=modint998244353;
using mint1=modint1000000007;
template<class T> using pqg=priority_queue<T,vector<T>,greater<T>>;
using blueberry::bin_search;
using blueberry::chmin;
using blueberry::chmax;
using blueberry::Fill;
using blueberry::len;
using blueberry::sum;
using blueberry::min_of;
using blueberry::max_of;
using blueberry::my_distance;
using blueberry::parse128;
using blueberry::to_string128;
#define INF ((1LL<<62)-(1LL<<31))
inline ll dx[8]={0,1,0,-1,-1,-1,1,1},dy[8]={1,0,-1,0,-1,1,-1,1};
inline constexpr ld CPS=CLOCKS_PER_SEC;
#define rep1(a) for(ll i=0;i<static_cast<ll>(a);++i)
#define rep2(i,a) for(ll i=0;i<static_cast<ll>(a);++i)
#define rep3(i,a,b) for(ll i=(a);i<static_cast<ll>(b);++i)
#define rep4(i,a,b,c) for(ll i=(a);i<static_cast<ll>(b);i+=(c))
#define overload4(a,b,c,d,e,...) e
#define BLUEBERRY_REP1 rep1
#define BLUEBERRY_REP2 rep2
#define BLUEBERRY_REP3 rep3
#define BLUEBERRY_REP4 rep4
#define BLUEBERRY_REP_SELECT overload4
#define rep(...) overload4(__VA_ARGS__,rep4,rep3,rep2,rep1)(__VA_ARGS__)
#define ALL(x) std::begin(x),std::end(x)
#define rALL(x) std::rbegin(x),std::rend(x)
#define bit(x,i) (((x)>>(i))&1)
#define fi first
#define se second
#define pb push_back
#define eb emplace_back
#define Endl std::endl
#define spa " "
#define popc(x) std::popcount(static_cast<ull>(x))
#define crmp(x,l,r) ((l)<=(x)&&(x)<=(r))
#define O ::print
#define LL(...) ll __VA_ARGS__;scan(__VA_ARGS__)
#define STR(...) str __VA_ARGS__;scan(__VA_ARGS__)
#ifdef BLUEBERRY
#define deb O
#else
#define deb(...)
#endif
inline ostream& operator<<(ostream& out,__int128 value){return out<<to_string128(value);}
inline istream& operator>>(istream& in,__int128& value){string text;if(in>>text){auto parsed=parse128(text);if(parsed)value=*parsed;else in.setstate(ios::failbit);}return in;}
template<class T,class A> istream& operator>>(istream&,vector<T,A>&);
template<class T,class A> ostream& operator<<(ostream&,const vector<T,A>&);
template<class T,class A> istream& operator>>(istream&,deque<T,A>&);
template<class T,class A> ostream& operator<<(ostream&,const deque<T,A>&);
template<class T,class U> istream& operator>>(istream&,pair<T,U>&);
template<class T,class U> ostream& operator<<(ostream&,const pair<T,U>&);
template<class T,class A> istream& operator>>(istream& in,vector<T,A>& v){
 for(size_t i=0;i<v.size();++i){if constexpr(is_same_v<T,bool>){bool x;if(!(in>>x))break;v[i]=x;}else if(!(in>>v[i]))break;}return in;
}
template<class T,class A> ostream& operator<<(ostream& out,const vector<T,A>& v){bool first=true;for(const auto& x:v){if(!first)out<<' ';first=false;out<<x;}return out;}
template<class T,class A> istream& operator>>(istream& in,deque<T,A>& v){for(auto& x:v)if(!(in>>x))break;return in;}
template<class T,class A> ostream& operator<<(ostream& out,const deque<T,A>& v){bool first=true;for(const auto& x:v){if(!first)out<<' ';first=false;out<<x;}return out;}
template<class T,class U> istream& operator>>(istream& in,pair<T,U>& v){return in>>v.first>>v.second;}
template<class T,class U> ostream& operator<<(ostream& out,const pair<T,U>& v){return out<<v.first<<' '<<v.second;}
template<class... T> requires(sizeof...(T)>0&&requires{typename common_type_t<T...>;})
constexpr auto min(T... a){using C=common_type_t<T...>;return std::min(initializer_list<C>{static_cast<C>(a)...});}
template<class... T> requires(sizeof...(T)>0&&requires{typename common_type_t<T...>;})
constexpr auto max(T... a){using C=common_type_t<T...>;return std::max(initializer_list<C>{static_cast<C>(a)...});}
template<class T,class U> requires(!is_invocable_v<U&,const T&,const T&>&&requires{typename common_type_t<T,U>;})
constexpr auto min(const T& a,const T& b,U c){return ::min<T,T,U>(a,b,c);}
template<class T,class U> requires(!is_invocable_v<U&,const T&,const T&>&&requires{typename common_type_t<T,U>;})
constexpr auto max(const T& a,const T& b,U c){return ::max<T,T,U>(a,b,c);}
template<class T,class A> T min(const vector<T,A>& a){return *min_element(ALL(a));}
template<class T,class A> T max(const vector<T,A>& a){return *max_element(ALL(a));}
inline ll divide(ll a,ll b){return blueberry::floor_div(a,b);}
inline bool iskaibun(string_view s){return blueberry::is_palindrome(s);}
inline __int128 ran(ll a,ll b){return blueberry::range_sum(a,b);}
inline int zaatu(vector<ll>& a){return blueberry::compress(a);}
template<class Index> bool isbipartite_graph(const vector<vector<Index>>& g){return blueberry::is_bipartite(g);}
inline __int128 parsetoint128(string_view s){__int128 v=0;for(char c:s)if('0'<=c&&c<='9')v=v*10+(c-'0');return v;}
inline vector<ll> enumdiv(ll n){vector<ll> v;for(ll i=1;i<=n/i;++i)if(n%i==0){v.push_back(i);if(i!=n/i)v.push_back(n/i);}return v;}
inline vector<pair<ll,ll>> soinsu_bunkai(ll n){
 vector<pair<ll,ll>> v;auto take=[&](ll p){if(n%p)return;ll k=0;do{n/=p;++k;}while(n%p==0);v.emplace_back(p,k);};
 take(2);take(3);for(ll p=5;p<=n/p;p+=6){take(p);take(p+2);}if(n>1)v.emplace_back(n,1);return v;
}
inline vector<ll> topo_sort(const vector<vector<ll>>& g,vector<ll>& indeg,ll n){
 vector<ll> v;pqg<ll> q;for(ll i=0;i<n;++i)if(!indeg[i])q.push(i);
 while(!q.empty()){ll u=q.top();q.pop();for(ll w:g[u])if(--indeg[w]==0)q.push(w);v.push_back(u);}return v;
}
inline vector<ll> fac,finv,invv;
inline bool COMINIT=false;
inline void COMinit(ll n,ll mod){
 if(COMINIT){return;}fac.assign(n,1);finv.assign(n,1);invv.assign(n,0);if(n>1)invv[1]=1;
 for(ll i=2;i<n;++i){fac[i]=static_cast<ll>(__int128(fac[i-1])*i%mod);invv[i]=mod-static_cast<ll>(__int128(invv[mod%i])*(mod/i)%mod);finv[i]=static_cast<ll>(__int128(finv[i-1])*invv[i]%mod);}COMINIT=true;
}
inline ll COM(int n,int k,ll mod){if(n<0||k<0||n<k)return 0;if(!k)return 1;return static_cast<ll>(__int128(fac[n])*finv[k]%mod*finv[n-k]%mod);}
inline ll nPr(int n,int k,ll mod){if(n<0||k<0||n<k)return 0;if(!k)return 1;return static_cast<ll>(__int128(fac[n])*finv[n-k]%mod);}
inline vector<bool> isprime;
inline vector<int> Era(int n){
 isprime.assign(n,true);if(n>0)isprime[0]=false;if(n>1)isprime[1]=false;vector<int> v;
 for(int i=2;i<n;++i)if(isprime[i]){v.push_back(i);if(i<=(n-1)/i)for(ll j=ll(i)*i;j<n;j+=i)isprime[j]=false;}
 return v;
}
class UnionFind{
 dsu uf;
public:
 UnionFind()=default;
 explicit UnionFind(size_t n):uf(static_cast<int>(n)){}
 int find(int i){return uf.leader(i);}
 void merge(int a,int b){uf.merge(a,b);}
 bool connected(int a,int b){return uf.same(a,b);}
 int size(int i){return uf.size(i);}
};
struct Rande{
 mt19937 mt;
 Rande():Rande(static_cast<uint32_t>(chrono::steady_clock::now().time_since_epoch().count())){}
 explicit Rande(uint32_t seed):mt(seed){}
 int operator()(int a,int b){return uniform_int_distribution<int>(a,b-1)(mt);}
 int operator()(int b){return (*this)(0,b);}
};

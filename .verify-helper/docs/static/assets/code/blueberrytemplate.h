#include<bits/stdc++.h>
using namespace std;
void _main();int main(){cin.tie(0);ios::sync_with_stdio(false);cout<<fixed<<setprecision(30);_main();quick_exit(0);return 0;}
typedef long long ll;typedef long double ld;
typedef unsigned long long ull;
typedef unsigned int uint;
typedef string str;
#define rep1(a)          for(ll i = 0; i < (ll)(a); i++)
#define rep2(i, a)       for(ll i = 0; i < (ll)(a); i++)
#define rep3(i, a, b)    for(ll i = (a); i < (ll)(b); i++)
#define rep4(i, a, b, c) for(ll i = (a); i < (ll)(b); i += (c))
#define overload4(a, b, c, d, e, ...) e
#define rep(...) overload4(__VA_ARGS__, rep4, rep3, rep2, rep1)(__VA_ARGS__)
#define ALL(x) std::begin(x),std::end(x)
#define rALL(x) std::rbegin(x),std::rend(x)
#define INF ((1LL<<62)-(1LL<<31))
// #define inf ((1<<30)-(1<<15))
#define bit(x,i) (((x)>>(i))&1)
#define fi first
#define se second
#define pb push_back
#define eb emplace_back
#define Endl endl
#define spa " "
#define YesNo(x) cout<<(x?"Yes":"No")<<endl;
#define YESNO(x) cout<<(x?"YES":"NO")<<endl;

// #define eps (1e-8)
#define popc(x) __builtin_popcount(x)
#define crmp(x,l,r) ((l<=x)&&(x<=r))

//！？！？
#define O print
//可変長引数で入力を受け取りつつ変数を宣言
inline void scan(){}
template<class Head,class... Tail>
inline void scan(Head&head,Tail&... tail){std::cin>>head;scan(tail...);}
#define LL(...) ll __VA_ARGS__;scan(__VA_ARGS__)
#define STR(...) string __VA_ARGS__;scan(__VA_ARGS__)
//vectorのcin
template<typename T>
std::istream &operator>>(std::istream&is,std::vector<T>&v){for(T &in:v){is>>in;}return is;}
//vectorのcout
template<typename T>
std::ostream &operator<<(std::ostream&os,const std::vector<T>&v){for(auto it=std::begin(v);it!=std::end(v);){os<<*it<<((++it)!=std::end(v)?" ":"");}return os;}
//dequeのcin
template<typename T>
std::istream &operator>>(std::istream&is,std::deque<T>&v){for(T &in:v){is>>in;}return is;}
//dequeのcout
template<typename T>
std::ostream &operator<<(std::ostream&os,const std::deque<T>&v){for(auto it=std::begin(v);it!=std::end(v);){os<<*it<<((++it)!=std::end(v)?" ":"");}return os;}
//pairのcin,cout
template<typename T,typename U>
std::ostream &operator<<(std::ostream&os,const std::pair<T,U>&p){os<<p.first<<" "<<p.second;return os;}
template<typename T,typename U>
std::istream &operator>>(std::istream&is,std::pair<T,U>&p){is>>p.first>>p.second;return is;}
//x,y,x,yを渡すとldで距離を返す
long double my_distance(long double xi,long double yi,long double xj,long double yj){return hypot(abs(xi-xj),abs(yi-yj));}
//可変長引数のprint関数
#pragma GCC diagnostic ignored "-Wunused-value"
void print(){cout << '\n';}
template<class T, class... Ts>
void print(const T& a, const Ts&... b){cout << a;(std::cout << ... << (cout << ' ', b));cout << '\n';}
#pragma GCC diagnostic warning "-Wunused-value"
//可変長引数のmin
template<class... T>
constexpr auto min(T... a){return min(initializer_list<common_type_t<T...>>{a...});}
//可変長引数のmax
template<class... T>
constexpr auto max(T... a){return max(initializer_list<common_type_t<T...>>{a...});}
template<typename T,typename U>inline bool chmax(T&a,U b){if(a<b){a=b;return 1;}return 0;}
template<typename T,typename U>inline bool chmin(T&a,U b){if(a>b){a=b;return 1;}return 0;}
template<typename T> inline T sum(vector<T>&a){T ret{};for(auto&i:a)ret+=i;return ret;}
template<typename T> inline T min(vector<T>&a){T ret=a[0];for(auto&i:a)chmin(ret,i);return ret;}
template<typename T> inline T max(vector<T>&a){T ret=a[0];for(auto&i:a)chmax(ret,i);return ret;}
template<typename T> inline int len(vector<T>&a){return a.size();}
inline int len(string&a){return a.size();}
// n次元配列の初期化。第２引数の型のサイズごとに初期化していく。
template<typename A, size_t N, typename T>
void Fill(A (&array)[N], const T &val){std::fill( (T*)array, (T*)(array+N), val );}
//こめんとを付け外ししてMODを切り替える
//ll MOD = INF;
// ll MOD = 1000000007;
// ll MOD = 998244353;

//ax+by = 1 であるようなx,yを返す
// pair<long long, long long> extgcd(long long a, long long b) {
//   if (b == 0) return make_pair(1, 0);
//   long long x, y;
//   tie(y, x) = extgcd(b, a % b);
//   y -= a / b * x;
//   return make_pair(x, y);
// }

struct Rande {mt19937 mt;Rande(): mt(chrono::steady_clock::now().time_since_epoch().count()){}int operator()(int a, int b) {uniform_int_distribution< int > dist(a, b - 1);return dist(mt);}int operator()(int b){return (*this)(0, b);}};
//from:https://kenkoooo.hatenablog.com/entry/2016/11/30/163533 int128
std::ostream &operator<<(std::ostream &dest, __int128_t value) {std::ostream::sentry s(dest);if (s){__uint128_t tmp = value < 0 ? -value : value;char buffer[128];char *d = std::end(buffer);do{--d;*d = "0123456789"[tmp % 10];tmp /= 10;} while (tmp != 0);if (value < 0) {--d;*d = '-';}int len = std::end(buffer) - d;if (dest.rdbuf()->sputn(d, len) != len) {dest.setstate(std::ios_base::badbit);}}return dest;}
__int128 parsetoint128(string &s) {__int128 ret = 0;for (int i = 0; i < (int)s.length(); i++)if ('0' <= s[i] && s[i] <= '9')ret=10*ret+(__int128_t)(s[i]-'0');return ret;}

ll divide(ll a, ll b){if(b < 0) a *= -1, b *= -1;if(a >= 0) return a/b;else return -(((-a)+(b-1))/b);}
//回文判定 
bool iskaibun(string s){ll k = s.size();rep(i,0,k/2){if(s[i]!=s[k-1-i]){return false;}}return true;}

//二部グラフ判定 重みなしグラフを引数に取り、boolを返す
bool isbipartite_graph(vector<vector<ll>>&g){ll v = g.size();vector<ll>col(v,-1);vector<bool>used(v,false);bool ret = true;rep(i,v){if(used[i])continue;col[i]=0;[DFS([&](auto&&f,ll pos,ll pr)->void{if(used[pos])return;used[pos]=true;for(auto to:g[pos]){if(to==pr)continue;if(used[to]&&col[pos]==col[to]){ret = false;return;}if(used[to])continue;col[to]=col[pos]^1;f(f,to,pos);}}),&i]{DFS(DFS,i,-1);}();}return ret;}
//a~bの和 a<b
ll ran(ll a,ll b){return ((a+b)*(b-a+1))/2;}
//座圧する
ll zaatu(vector<ll>&A){map<ll,ll>m;for(auto&&x:A)m[x]=0;ll ret = 0;for(auto&&[key,val]:m)val=ret++;for(auto&&x:A)x=m[x];return ret;}
//約数列挙　引数に取った整数の約数のvectorを返す
vector<ll>enumdiv(ll n){vector<ll>s;for(ll i = 1;i*i<=n;i++){if(n%i==0){s.push_back(i);if(i*i!=n)s.push_back(n/i);}}return s;}
//トポロジカルソート グラフ、入次数カウント、頂点数を引数で渡すと、トポロジカルソートされた頂点列を返す
vector<ll> topo_sort(vector<vector<ll>>&G,vector<ll>&nyu_cnt,ll v){vector<ll>ret;priority_queue<ll,vector<ll>,greater<ll>>pq;rep(i,0,v){if(nyu_cnt[i]==0)pq.push(i);}while(!pq.empty()){ll pos = pq.top();pq.pop();for(ll i:G[pos]){nyu_cnt[i]--;if(nyu_cnt[i]==0)pq.push(i);}ret.push_back(pos);}return ret;}
//素因数分解 pair<素数、指数>のvectorを返す
vector<pair<ll, ll>> soinsu_bunkai(ll x){vector<pair<ll, ll>> ret;ret.reserve(1<<8);ll i = 2;for(i = 2;i<4;i++)if(x%i== 0){ll cnt{};while (x % i == 0){x /= i;cnt++;}ret.push_back({i, cnt});}for(i = 1;i*i<=x;i+=2){if(i>1)if(x%i==0){ll cnt{};while (x % i == 0){x /= i;cnt++;}ret.push_back({i, cnt});}i += 4;if(x%i==0){ll cnt{};while (x % i == 0){x /= i;cnt++;}ret.push_back({i, cnt});}}if (x != 1)ret.push_back({x, 1});return ret;}
//二項係数MOD MODは上の方で設定、MAXまでのnCrをCOM(n,r)でとれる
vector<ll> fac, finv, invv;bool COMINIT=false;
void COMinit(ll MAX,ll MOD){if(COMINIT)return;fac.resize(MAX);finv.resize(MAX);invv.resize(MAX);COMINIT=true;fac[0]=fac[1]=finv[0]=finv[1]=invv[1]=1;for(int i=2;i<MAX;i++){fac[i]=fac[i-1]*i%MOD;invv[i]=MOD-invv[MOD%i]*(MOD/i)%MOD;finv[i]=finv[i-1]*invv[i]%MOD;}}
ll COM(int n,int k,ll MOD){if(n<k)return 0;if(n<0||k<0)return 0;if(k==0)return 1;return fac[n]*(finv[k]*finv[n-k]%MOD)%MOD;}
ll nPr(int n,int k,ll MOD){if(n<k)return 0;if(n<0||k<0)return 0;if(k==0)return 1;return fac[n]*(finv[n-k])%MOD;}
//エラトステネスの篩　isprimeには素数かどうかが入っている
vector<bool> isprime;vector<int> Era(int n) {isprime.resize(n, true);vector<int> res;isprime[0] = false; isprime[1] = false;for (int i = 2; i < n; ++i) isprime[i] = true;for (int i = 2; i < n; ++i){if (isprime[i]) {res.push_back(i);for (int j = i*2; j < n; j += i) isprime[j] = false;}}return res;}
//Union-Find from https://zenn.dev/reputeless/books/standard-cpp-for-competitive-programming/viewer/union-find
class UnionFind{public:UnionFind()=default;explicit UnionFind(size_t n):m_parentsOrSize(n, -1){}int find(int i){if(m_parentsOrSize[i]<0){return i;}return(m_parentsOrSize[i]=find(m_parentsOrSize[i]));}void merge(int a,int b){a=find(a);b=find(b);if(a!=b){if(-m_parentsOrSize[a]<-m_parentsOrSize[b]){std::swap(a,b);}m_parentsOrSize[a]+=m_parentsOrSize[b];m_parentsOrSize[b]=a;}}bool connected(int a,int b){return (find(a)==find(b));}int size(int i){return -m_parentsOrSize[find(i)];}private:std::vector<int>m_parentsOrSize;};
template <class T> using pqg = priority_queue<T, vector<T>, greater<T>>;
template <class F> ll bin_search(ll ok,ll ng,const F&f){while(abs(ok-ng)>1){long long mid=(ok+ng)>>1;(f(mid)?ok:ng)=mid;}return ok;}
//グリッドの8近傍 4まで回せば4近傍
ll dx[8] = {0,1,0,-1,-1,-1,1,1},dy[8]={1,0,-1,0,-1,1,-1,1};
constexpr ld CPS = CLOCKS_PER_SEC;
#include<atcoder/all>
using namespace atcoder;
// using mint = modint;
using mint = modint998244353;
using mint1 = modint1000000007;
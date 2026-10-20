#include TEMPLATE_HEADER
#ifndef INF
#error INF must be a macro
#endif
#if INF!=((1LL<<62)-(1LL<<31))
#error INF has a different value
#endif
template<class T> struct Alloc:allocator<T>{using value_type=T;template<class U>struct rebind{using other=Alloc<U>;};Alloc()=default;template<class U>Alloc(const Alloc<U>&){}};
struct Score{int value;bool operator<(const Score& x)const{return value<x.value;}};
struct Callable{operator int()const{return 0;}bool operator()(int a,int b)const{return a>b;}};
template<class T>void compare_case(const T& actual,const T& expected,int round,const char* operation){if(actual!=expected){cerr<<"seed=71549 round="<<round<<" operation="<<operation<<" expected="<<expected<<" actual="<<actual<<'\n';abort();}}
void randomized(){
mt19937_64 rng(71549);
for(int round=0;round<200;++round){
int count=rng()%9;UnionFind uf(count);vector<int> labels(count);iota(ALL(labels),0);
for(int step=0;step<30&&count;++step){int a=rng()%count,b=rng()%count;if(rng()%2){int old=labels[b],next=labels[a];for(auto& label:labels)if(label==old)label=next;uf.merge(a,b);}compare_case(uf.connected(a,b),labels[a]==labels[b],round,"UnionFind.connected");compare_case(uf.size(a),int(std::count(ALL(labels),labels[a])),round,"UnionFind.size");}
vector<vector<ll>> graph(count);vector<ll> degree(count);
for(int a=0;a<count;++a)for(int b=0;b<count;++b)if(rng()%5==0){graph[a].pb(b);++degree[b];}
auto original_degree=degree;vector<bool> removed(count);vector<ll> expected;
for(;;){int next=-1;for(int i=0;i<count;++i)if(!removed[i]&&!degree[i]){next=i;break;}if(next<0)break;removed[next]=true;expected.pb(next);for(ll to:graph[next])--degree[to];}
auto actual_degree=original_degree;compare_case(topo_sort(graph,actual_degree,count),expected,round,"topo_sort");compare_case(actual_degree,degree,round,"topo_sort.indegree");
int limit=2+rng()%100;vector<int> primes;vector<bool> flags(limit);
for(int value=2;value<limit;++value){bool prime=true;for(int d=2;d*d<=value;++d)if(value%d==0)prime=false;if(prime){primes.pb(value);flags[value]=true;}}
compare_case(Era(limit),primes,round,"Era");compare_case(isprime,flags,round,"Era.isprime");
ll value=1+rng()%10000,remaining=value;vector<pair<ll,ll>> factors;vector<ll> divisors;
for(ll d=2;d*d<=remaining;++d)if(remaining%d==0){ll exponent=0;while(remaining%d==0){remaining/=d;++exponent;}factors.eb(d,exponent);}
if(remaining>1)factors.eb(remaining,1);
for(ll d=1;d<=value;++d)if(value%d==0)divisors.pb(d);
auto actual_divisors=enumdiv(value);sort(ALL(actual_divisors));compare_case(actual_divisors,divisors,round,"enumdiv");compare_case(soinsu_bunkai(value),factors,round,"soinsu_bunkai");
}
}
int main(){
init_io(2);
static_assert(is_same_v<decltype(max(1,2LL)),ll>);
static_assert(is_same_v<decltype(min(1,2LL,3.0L)),ld>);
assert(max(1,7LL,-3,4.5)==7&&min(1,7LL,-3,4.5)==-3);
assert(max(7)==7&&min(7)==7&&max(1,2,3)==3&&min(1,2,3)==1&&max(1,2,3,4)==4&&min(1,2,3,4)==1);
assert(max(1,2LL,3.0)==3&&min(1,2LL,3.0)==1&&max(__int128(-3),2LL)==2);
assert(max(str("a"),str("c"),str("b"))=="c"&&min(str("a"),str("c"),str("b"))=="a");
assert(max(Score{1},Score{9},Score{3}).value==9&&min(Score{1},Score{9},Score{3}).value==1);
int a=1,b=2;static_assert(is_same_v<decltype(max(a,b)),const int&>);
assert(&max(a,b)==&b&&max(1,2,greater<int>{})==1&&min(1,2,greater<int>{})==2);
assert(max(1,2,Callable{})==1&&min(1,2,Callable{})==2);
assert(max({1,2,3})==3&&min({1,2,3},greater<int>{})==3);
vector<ll> values{3,1,3};assert(min(values)==1&&max(values)==3&&sum(values)==7&&len(values)==3);
const vector<int,Alloc<int>> custom_extrema{3,1,2};const vector<bool> bool_extrema{true,false};assert(max(custom_extrema)==3&&min(custom_extrema)==1&&max(bool_extrema)&&!min(bool_extrema));
assert(zaatu(values)==2&&(values==vector<ll>{1,0,1}));
int total=0;rep1(3)total+=i;rep2(j,2)total+=j;rep3(j,2,4)total+=j;rep4(j,0,4,2)total+=j;assert(total==11);
total=0;rep(3)total+=i;rep(j,2)total+=j;rep(j,2,4)total+=j;rep(j,0,4,2)total+=j;assert(total==11);
vector<int> numbers;numbers.pb(3);numbers.eb(4);sort(rALL(numbers));assert(numbers.front()==4);
pair<int,int> p{3,4};assert(p.fi==3&&p.se==4&&bit(8,3)&&popc(1ULL<<40)==1&&crmp(2,1,3));
pqg<int> queue;queue.push(3);queue.push(1);assert(queue.top()==1);
int array[2][3];Fill(array,-5);assert(array[1][2]==-5);
assert(divide(-7,3)==-3&&divide(7,-3)==-3&&ran(1,3)==6&&my_distance(0,0,3,4)==5);
assert(iskaibun("")&&iskaibun("abba")&&!iskaibun("ab"));
str text="-a1b23";assert(parsetoint128(text)==123);
assert(parsetoint128("")==0&&parsetoint128("--abc")==0);
str maximum="170141183460469231731687303715884105727";assert(parsetoint128(maximum)==numeric_limits<__int128>::max());
assert(parse128("-123").value()==-123&&to_string128(-123)=="-123");
assert((enumdiv(12)==vector<ll>{1,12,2,6,3,4})&&enumdiv(1)==vector<ll>{1});
assert((soinsu_bunkai(360)==vector<pair<ll,ll>>{{2,3},{3,2},{5,1}})&&soinsu_bunkai(1).empty());
for(ll value=1;value<=300;++value){
auto actual=enumdiv(value);sort(ALL(actual));vector<ll> expected_divisors;
for(ll d=1;d<=value;++d)if(value%d==0)expected_divisors.pb(d);
assert(actual==expected_divisors);ll product=1,previous=1;
for(auto [prime,exponent]:soinsu_bunkai(value)){assert(prime>previous&&exponent>0);for(ll d=2;d*d<=prime;++d)assert(prime%d!=0);while(exponent--)product*=prime;previous=prime;}
assert(product==value);
}
vector<vector<ll>> graph{{2},{2},{}};vector<ll> indegree{0,0,2};
assert((topo_sort(graph,indegree,3)==vector<ll>{0,1,2})&&(indegree==vector<ll>{0,0,0}));
graph={{1},{0},{}};indegree={1,1,0};assert((topo_sort(graph,indegree,3)==vector<ll>{2}));
assert(isbipartite_graph(graph));graph={{0}};assert(!isbipartite_graph(graph));
assert(!COMINIT);COMinit(16,998244353);assert(COMINIT&&fac.size()==16&&finv.size()==16&&invv.size()==16);
assert(fac[5]==120&&fac[5]*finv[5]%998244353==1&&invv[2]*2%998244353==1);
assert(COM(5,2,998244353)==10&&nPr(5,2,998244353)==20&&COM(2,3,998244353)==0&&nPr(-1,0,998244353)==0);
ll pascal[16][16]{};for(int i=0;i<16;++i){pascal[i][0]=1;for(int k=1;k<=i;++k)pascal[i][k]=pascal[i-1][k-1]+pascal[i-1][k];for(int k=0;k<=i;++k){assert(COM(i,k,998244353)==pascal[i][k]);ll permutation=1;for(int j=0;j<k;++j)permutation=permutation*(i-j)%998244353;assert(nPr(i,k,998244353)==permutation);}}
assert((Era(20)==vector<int>{2,3,5,7,11,13,17,19})&&isprime.size()==20&&!isprime[0]&&!isprime[1]&&isprime[19]);
assert((Era(3)==vector<int>{2})&&isprime.size()==3);
assert(Era(0).empty()&&isprime.empty());assert(Era(1).empty()&&isprime.size()==1&&!isprime[0]);assert(Era(2).empty()&&isprime.size()==2&&!isprime[1]);
COMinit(3,7);assert(fac.size()==16&&COM(5,2,998244353)==10);
COMINIT=false;COMinit(0,7);assert(fac.empty()&&finv.empty()&&invv.empty()&&COM(0,0,7)==1);
COMINIT=false;COMinit(1,7);assert(fac.size()==1&&fac[0]==1&&finv[0]==1&&invv[0]==0&&nPr(0,0,7)==1);
COMINIT=false;COMinit(7,7);assert(COM(6,3,7)==6&&nPr(6,3,7)==1);
UnionFind empty;UnionFind uf(4);uf.merge(0,1);uf.merge(1,2);uf.merge(0,2);
assert(uf.connected(0,2)&&!uf.connected(0,3)&&uf.find(0)==uf.find(2)&&uf.size(1)==3&&uf.size(3)==1);
Rande random;random.mt.seed(42);mt19937 expected(42);assert(random.mt()==expected());
for(int i=0;i<100;++i){int x=random(-3,2);assert(-3<=x&&x<2);assert(random(1)==0);}
ll* horizontal=dx;ll* vertical=dy;assert(horizontal[0]==0&&vertical[0]==1&&CPS==CLOCKS_PER_SEC);
dx[0]=9;assert(dx[0]==9);dx[0]=0;
mint m=3;mint1 n=4;dsu acl(2);acl.merge(0,1);assert(m.val()==3&&n.val()==4&&acl.same(0,1));
randomized();
vector<vector<pair<int,int>>> matrix(1,vector<pair<int,int>>(2));
vector<int,Alloc<int>> custom(2);deque<int,Alloc<int>> custom_deque(2);vector<bool,Alloc<bool>> bits(3);
pair<deque<vector<int>>,int> nested{{vector<int>(2)},0};vector<__int128> wide(1);
istringstream input("1 2 3 4 5 6 7 8 1 0 1 9 10 11 -170141183460469231731687303715884105728");
auto* saved=cin.rdbuf(input.rdbuf());cin>>matrix>>custom>>custom_deque>>bits>>nested>>wide;
assert(cin&&matrix[0][1].second==4&&custom[1]==6&&custom_deque[1]==8&&bits[0]&&!bits[1]&&bits[2]&&nested.second==11&&wide[0]==numeric_limits<__int128>::min());
cin.rdbuf(saved);cin.clear();ostringstream output;output<<matrix<<' '<<custom<<' '<<custom_deque<<' '<<bits<<' '<<nested<<' '<<wide;
assert(output.str()=="1 2 3 4 5 6 7 8 1 0 1 9 10 11 -170141183460469231731687303715884105728");
vector<bool> failed{false,false,true};istringstream bad("1 x 0");bad>>failed;assert(bad.fail()&&failed[0]&&!failed[1]&&failed[2]);
istringstream short_input("8");vector<int> partial{4,5};short_input>>partial;assert(short_input.fail()&&(partial==vector<int>{8,5}));
LL(first,second);STR(word);vector<pair<int,int>> scanned(2);scan(scanned);scan();
assert(first==7&&second==5&&word=="word"&&scanned[1].second==4);
int effects=0;deb(++effects,vector<int>{4,5});
#ifdef BLUEBERRY
assert(effects==1);
#else
assert(effects==0);deb(a_symbol_that_must_not_be_compiled);
#endif
O(first+second,word,scanned);print(1.25);YesNo(true);YESNO(false);
#ifdef FAST_FACADE
assert(flush());
#endif
}

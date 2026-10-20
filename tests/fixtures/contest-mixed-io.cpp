#include TEMPLATE_HEADER
static_assert(is_same_v<decltype(cin),istream>);
static_assert(is_same_v<decltype(cout),ostream>);
struct Custom{ll value;string name;};
istream&operator>>(istream&in,Custom&value){return in>>value.value>>value.name;}
#ifdef FAST_FACADE
template<class T>struct Alloc:allocator<T>{template<class U>struct rebind{using other=Alloc<U>;};};
static_assert(is_same_v<decltype(fin>>declval<ll&>()),decltype(fin)&>);
static_assert(!is_convertible_v<decltype(fin),bool>);
#endif
int finish(){
#ifdef FAST_FACADE
 return flush()?0:1;
#else
 cout.flush();return cout?0:1;
#endif
}
int main(int argc,char**argv){
 init_io();
 string mode=argc>1?argv[1]:"reproducer";
 if(mode=="reproducer"){
  LL(n,m);
  vector<ll>a(n);cin>>a;
  O(a);
  return finish();
 }
#ifdef FAST_FACADE
 if(mode=="fin-reproducer"){
  ll n=0,m=0;fin>>n>>m;vector<ll>a(n);fin>>a;O(a);return finish();
 }
 if(mode=="fin-types"){
  init_io(3);ll n=0,m=0;fin>>n>>m;vector<ll,Alloc<ll>>a(n);vector<bool>flags(4);
  deque<ll>d(2);array<ll,2>r{};pair<ll,str>p;char c=0;double value=0;__int128 wide=0;
  vector<vector<__int128>>nested(2,vector<__int128>(2));
  fin>>a>>flags>>d>>r>>p>>c>>value>>wide>>nested;
  O(bool(fin),fin.fail(),n,m,a,flags,d,r,p,c,value,wide,nested);return finish();
 }
 if(mode=="fin-alternate"){
  ll n=0;fin>>n;ull hash=0;
  for(ll i=0;i<n;++i){ll value=0;if(i%4==0)fin>>value;else if(i%4==1)scan(value);else if(i%4==2)std::cin>>value;else{LL(parsed);value=parsed;}hash=hash*1000003+static_cast<ull>(value);}
  O(hash,bool(fin),fin.fail());return finish();
 }
 if(mode=="fin-fail"){
  ll a=0,b=8,c=9;auto&chain=fin>>a>>b>>c;
  O(&chain==&fin,bool(fin),!fin,fin.fail(),a,b,c);
  init_io(3);O(bool(fin),fin.fail());fin.clear();fin>>c;O(bool(fin),fin.fail(),c);
  ll d=0;scan(d);O(d);fin>>d;O(bool(fin),!fin,fin.fail(),d);return finish();
 }
 if(mode=="fin-scan-fail"){
  ll value=11;bool ok=scan(value);O(ok,bool(fin),fin.fail(),value);
  fin>>value;O(bool(fin),fin.fail(),value);return finish();
 }
 if(mode=="fin-precision"){
  init_io(2);double a=0,b=0;fin>>a;init_io(4);fin>>b;
  O(a,b);cout<<a<<' '<<b<<'\n';return finish();
 }
#endif
 if(mode=="alternate"){
  LL(n);ull hash=0;
  for(ll i=0;i<n;++i){ll value=0;if(i%3==0)scan(value);else if(i%3==1)cin>>value;else std::cin>>value;hash=hash*1000003+static_cast<ull>(value);}
  O(hash);cout<<hash<<'\n';return finish();
 }
 if(mode=="nested"){
  LL(first);
  vector<vector<pair<__int128,__int128>>>a(2,vector<pair<__int128,__int128>>(2));
  cin>>a;
  vector<bool>flags(4);cin>>flags;
  __int128 tail=0;scan(tail);
  O(first,a,flags,tail);
  cout<<a<<'\n';return finish();
 }
 if(mode=="custom"){
  LL(first);Custom value{};istream&in=std::cin;in>>value;LL(last);
  O(first,value.value,value.name,last);return finish();
 }
 if(mode=="cin-first"){
  ll n=0,tail=0;cin>>n;vector<ll>a(n);scan(a);cin>>tail;
  O(a,tail);return finish();
 }
 if(mode=="line"){
  LL(n);cin>>ws;
  int first=cin.peek();char c=0;cin.get(c);cin.unget();
  string line;getline(cin,line);
  LL(tail);O(n,first,c,line,tail);return finish();
 }
 if(mode=="chars"){
  LL(n);cin>>ws;ull hash=0;
  for(ll i=0;i<n;++i){char c=0;cin.get(c);if(i%31==0){cin.unget();char again=0;cin.get(again);if(again!=c)return 2;}hash=hash*1000003+static_cast<unsigned char>(c);}
  LL(tail);O(hash,tail);return finish();
 }
 if(mode=="state"){
  bool flag=false;cin>>flag;cout<<flag<<' ';
  int value=777;cin>>value;cout<<cin.fail()<<' ';
  cin.clear();string token;cin>>token;
  scan(value);O(token,value);
  cin>>value;cout<<cin.eof()<<' '<<cin.fail()<<'\n';
  init_io(3);O(cin.eof(),cin.fail());
  cin.clear();cout<<cin.good()<<'\n';return finish();
 }
 if(mode=="bool"){
  bool a=false,b=false;cin>>boolalpha>>a>>b;cout<<noboolalpha<<a<<' '<<b<<' ';
  int value=0;scan(value);O(value);return finish();
 }
 if(mode=="order"){
  cout<<"a";O("b");cout<<"c"<<endl;O("d");return finish();
 }
 if(mode=="print-lookup"){
  string label="word";int value=7;vector<ll>values{2,3};const string fixed="fixed";const int frozen=9;
  struct Bits{unsigned value:4;};Bits bits{5};
  print(label,value);print("literal",value);print(label,values);print(fixed,frozen);
  ::print<string,int>(label,value);print();O(bits.value);O(label,+bits.value);return finish();
 }
 if(mode=="output-large"){
  LL(n);for(ll i=0;i<n;++i){cout<<i<<' ';O(-i);}return finish();
 }
 if(mode=="precision"){
  LL(n);init_io(3);vector<ll>a(n);cin>>a;O(a,1.25);cout<<2.5<<'\n';
  init_io(1);LL(tail);O(tail,3.76);cout<<4.26<<'\n';return finish();
 }
 if(mode=="flush"){
  O("pending");cout<<endl;LL(value);O(value);return finish();
 }
 return 3;
}

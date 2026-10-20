#define PROBLEM "https://judge.yosupo.jp/problem/many_aplusb_128bit"
#include "blueberry/utility/contest-fast-io.hpp"
int main(){
 init_io();int t=0;if(!scan(t))return 1;
 while(t--){__int128 a=0,b=0;if(!(cin>>a)||!(fin>>b)||!print(a+b))return 1;}
 return flush()?0:1;
}

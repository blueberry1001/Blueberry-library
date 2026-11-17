#include <chrono>
#include <cstdint>
#include <cstdio>
#include "queue.hpp"
struct F { uint64_t a, b; };
F op(F x,F y){return {x.a*y.a,x.b*y.a+y.b};}
F e(){return {1,0};}
int main(int argc,char**argv){
 int mode=argv[1][0]-'0'; uint64_t sum=0;
 auto start=std::chrono::steady_clock::now();
 if(mode==0) for(int r=0;r<70;r++){blueberry::AggregateQueue<F,op,e> q; for(int i=0;i<100000;i++)q.push({uint64_t(i+1),uint64_t(i)}); while(!q.empty()){sum+=q.prod().b;q.pop();}}
 if(mode==1){blueberry::AggregateQueue<F,op,e> q; for(int r=1;r<=5000;r++){for(int i=0;i<r;i++)q.push({uint64_t(i+1),uint64_t(i)});for(int i=0;i<r;i++){sum+=q.prod().b;q.pop();}}}
 if(mode==2){blueberry::AggregateQueue<F,op,e> q; for(int r=0;r<1000000;r++){for(int i=0;i<8;i++)q.push({uint64_t(i+1),uint64_t(i)});for(int i=0;i<8;i++){sum+=q.prod().b;q.pop();}}}
 auto end=std::chrono::steady_clock::now(); printf("%.3f %llu\n",std::chrono::duration<double,std::milli>(end-start).count(),(unsigned long long)sum);
}

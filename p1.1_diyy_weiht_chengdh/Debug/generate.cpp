#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <random>
#include <chrono>
#include "src/utils.h"
int main(){
    int i=0;
    FILE* inp=fopen("test.in","w");
    std::mt19937 rnd(std::chrono::system_clock::now().time_since_epoch().count());
    for(i=0;i<50;i++)
    InstOutput(&inp, rnd());
}
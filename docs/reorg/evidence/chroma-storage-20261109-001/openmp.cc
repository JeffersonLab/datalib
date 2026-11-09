#include <omp.h>
#include <iostream>
int main(){int threads=0;
#pragma omp parallel
{
#pragma omp single
threads=omp_get_num_threads();}
std::cout<<threads<<"\n";}

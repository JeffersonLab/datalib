#include <array>
#include <iostream>
std::array<std::size_t,7> reader();std::array<std::size_t,7> writer();int main(){auto a=reader(),b=writer();for(int i=0;i<7;++i)std::cout<<a[i]<<" "<<b[i]<<"\n";return a==b?0:1;}

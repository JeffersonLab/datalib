#include <array>
#include <iostream>
std::array<std::size_t,7> reader(),writer(),direct();int main(){auto a=reader(),b=writer(),c=direct();for(int i=0;i<7;++i)std::cout<<a[i]<<" "<<b[i]<<" "<<c[i]<<"\n";return a==b&&a==c?0:1;}

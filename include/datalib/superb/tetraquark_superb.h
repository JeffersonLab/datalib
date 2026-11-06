#ifndef ADAT_TETRAQUARK_SUPERB_H
#define ADAT_TETRAQUARK_SUPERB_H
#include "datalib/superb/types.h"
#include "datalib/superb/tetraquark_elem_type.h"
#include "datalib/storage/filedb.h"
#include "ConfDataStoreMultipleDB.h"
#include "datalib/superb/superb_aux.h"
#include <limits>
#include <set>
#include <cmath>
namespace Datalib::TensorStorage {
// Both formats expose the same typed keys. Tensor storage v1 uses i-fastest
// ijkltmc axes and explicit momentum/color dictionaries; missing blocks fail.
class StorageTetraQuark {
 using K=Hadron::KeyTetraQuarkElementalOperator_t;
 using V=Hadron::ValTetraQuarkElementalOperator_t;
 using T=std::complex<double>;
 FILEDB::ConfDataStoreMultipleDB<SerialDBKey<K>,SerialDBData<V>> legacy;
#ifdef USE_SUPERBBLAS
 static constexpr int N=7;
 struct Store {superbblas::Storage_handle handle;std::string order;std::map<char,int> dims;std::vector<std::array<int,3>> moms;std::vector<std::string> colors;};
 std::vector<Store> stores;
#endif
public:
 StorageTetraQuark()=default;
 StorageTetraQuark(const StorageTetraQuark&)=delete;
 ~StorageTetraQuark(){close();}
 void setCacheSize(unsigned int n){legacy.setCacheSize(n);}
 int close(){
#ifdef USE_SUPERBBLAS
  for(auto&s:stores)superbblas::close_storage<N,T>(s.handle);stores.clear();
#endif
  return legacy.close();
 }
 int open(const std::vector<std::string>& files){
  close();std::vector<std::string> old;
  try{for(const auto&file:files){
#ifdef USE_SUPERBBLAS
   std::vector<char> meta;std::vector<superbblas::IndexType> dims;superbblas::values_datatype dtype;
   bool tensor=false;try{superbblas::read_storage_header(file.c_str(),superbblas::FastToSlow,dtype,meta,dims);tensor=true;}catch(const std::exception&){}
   if(tensor){
    if(dims.size()!=N || dtype!=superbblas::detail::get_values_datatype<T>())throw std::runtime_error("Tetraquark tensor requires rank 7 and complex<double>");
    auto opt=SB::getOptionsFromXML(std::string(meta.begin(),meta.end()));
    if(SB::getOption<std::string>(*opt,"DBMetaData/id")!="tetraQuarkElemOp" || SB::getOption<int>(*opt,"DBMetaData/storage_version")!=1)throw std::runtime_error("Unsupported tetraquark tensor schema");
    auto order=SB::getOption<std::string>(*opt,"DBMetaData/tensorOrder");
    if(order.size()!=N || std::set<char>(order.begin(),order.end())!=std::set<char>{'i','j','k','l','t','m','c'})throw std::runtime_error("Invalid tetraquark tensor axes");
    auto kv=detail::to_kvdim(order,dims);
    for(char c:std::string("ijkltmc"))if(kv.at(c)<=0)throw std::runtime_error("Empty tetraquark tensor dimension");
    for(char c:std::string("jkl"))if(kv.at(c)!=kv.at('i'))throw std::runtime_error("Unequal tetraquark vector dimensions");
    auto moms=SB::getOption<std::vector<std::array<int,3>>>(*opt,"DBMetaData/moms");auto colors=SB::getOption<std::vector<std::string>>(*opt,"DBMetaData/colors");
    if(moms.size()!=kv.at('m') || colors.size()!=kv.at('c') || std::set<std::array<int,3>>(moms.begin(),moms.end()).size()!=moms.size() || std::set<std::string>(colors.begin(),colors.end()).size()!=colors.size())throw std::runtime_error("Invalid tetraquark dictionaries");
    superbblas::Storage_handle h;superbblas::open_storage<N,T>(file.c_str(),false,&h);stores.push_back({h,order,kv,moms,colors});continue;
   }
#endif
   old.push_back(file);
  }return old.empty()?0:legacy.open(old);}catch(...){close();throw;}
 }
 int get(const SerialDBKey<K>& key,SerialDBData<V>& val){return get(key.key(),val.data());}
 int get(const K&key,V&val){
#ifdef USE_SUPERBBLAS
  for(const auto&s:stores){
   auto m=std::find(s.moms.begin(),s.moms.end(),detail::toCoor(key.mom));auto c=std::find(s.colors.begin(),s.colors.end(),key.color);
   if(m==s.moms.end()||c==s.colors.end()||key.t_slice<0||key.t_slice>=s.dims.at('t'))continue;
   std::vector<superbblas::PartitionItem<N>> blocks;
   superbblas::get_blocks<N,N,T>(s.handle,s.order.c_str(),s.order.c_str(),superbblas::Coor<N>{},detail::kvcoors<N>(s.order,s.dims),blocks,superbblas::FastToSlow);
   bool complete=false;
   for(const auto& b:blocks){auto lo=detail::to_kvdim(s.order,b[0]);auto sz=detail::to_kvdim(s.order,b[1]);bool covers=true;
    for(char a:std::string("ijkl"))covers=covers&&lo.at(a)==0&&sz.at(a)==s.dims.at(a);
    for(auto v:std::map<char,int>{{'t',key.t_slice},{'m',int(m-s.moms.begin())},{'c',int(c-s.colors.begin())}})covers=covers&&v.second>=lo.at(v.first)&&v.second<lo.at(v.first)+sz.at(v.first);
    complete=complete||covers;
   }
   if(!complete)continue;
   int n=s.dims.at('i');size_t count=size_t(n)*n*n*n;
   std::vector<T> data(count,T(std::numeric_limits<double>::quiet_NaN(),0));T*ptr=data.data();
   superbblas::Coor<4> zero{},size{{n,n,n,n}};superbblas::PartitionItem<4> part{zero,size};superbblas::Context cpu=superbblas::createCpuContext();
   auto from=detail::kvcoors<N>(s.order,{{'t',key.t_slice},{'m',int(m-s.moms.begin())},{'c',int(c-s.colors.begin())}});
   auto extent=detail::kvcoors<N>(s.order,{{'i',n},{'j',n},{'k',n},{'l',n}},1);
#if SUPERBBLAS_VERSION < 2
   superbblas::load<N,4,T,T>(T(1),s.handle,s.order.c_str(),from,extent,&part,1,"ijkl",zero,&ptr,&cpu,superbblas::FastToSlow,superbblas::Copy);
#else
   superbblas::load<N,4,T,T>(T(1),s.handle,s.order.c_str(),from,extent,&part,1,"ijkl",zero,size,&ptr,&cpu,superbblas::FastToSlow,superbblas::Copy);
#endif
   // A partially present tensor block is not a complete elemental.
   for(auto z:data)if(!std::isfinite(z.real())||!std::isfinite(z.imag()))return -1;
   val.op.resize(n,n,n,n);
   for(int l=0;l<n;++l)for(int k=0;k<n;++k)for(int j=0;j<n;++j)for(int i=0;i<n;++i)val.op(i,j,k,l)=data[i+n*(j+n*(k+n*l))];return 0;
  }
#endif
  SerialDBKey<K> k(key);SerialDBData<V> v;int rc=legacy.get(k,v);if(!rc)val=v.data();return rc;
 }
};
}
#endif

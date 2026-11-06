#ifndef ADAT_GLUEBALL_SUPERB_H
#define ADAT_GLUEBALL_SUPERB_H
#include "datalib/superb/types.h"
#include "datalib/superb/glueball_elem_type.h"
#include "datalib/storage/filedb.h"
#include "ConfDataStoreMultipleDB.h"
#include "datalib/superb/superb_aux.h"
#include <limits>
#include <set>
#include <cmath>
namespace Datalib::TensorStorage {
// Diagonal glue elementals. Schema v1 uses vector-fastest ntdm axes;
// left/right are zero, with explicit normalized displacement/momentum lists.
class StorageGlue {
 using K=Hadron::KeyGlueElementalOperator_t;using V=Hadron::ValGlueElementalOperator_t;using T=std::complex<double>;
 FILEDB::ConfDataStoreMultipleDB<SerialDBKey<K>,SerialDBData<V>> legacy;
#ifdef USE_SUPERBBLAS
 static constexpr int N=4;
 struct Store{superbblas::Storage_handle handle;std::string order;std::map<char,int> dims;std::vector<std::array<int,3>> moms;std::vector<std::vector<int>> disps;std::set<std::array<int,3>> present;};
 std::vector<Store> stores;
#endif
public:
 StorageGlue()=default;StorageGlue(const StorageGlue&)=delete;~StorageGlue(){close();}
 void setCacheSize(unsigned int n){legacy.setCacheSize(n);}void setCacheSizeMB(unsigned int n){legacy.setCacheSizeMB(n);}
 int close(){
#ifdef USE_SUPERBBLAS
  for(auto&s:stores)superbblas::close_storage<N,T>(s.handle);stores.clear();
#endif
  return legacy.close();
 }
 int open(const std::vector<std::string>&files){close();std::vector<std::string> old;
  try{for(const auto&file:files){
#ifdef USE_SUPERBBLAS
   std::vector<char>meta;std::vector<superbblas::IndexType>dims;superbblas::values_datatype dtype;bool tensor=false;
   try{superbblas::read_storage_header(file.c_str(),superbblas::FastToSlow,dtype,meta,dims);tensor=true;}catch(const std::exception&){}
   if(tensor){
    if(dims.size()!=N||dtype!=superbblas::detail::get_values_datatype<T>())throw std::runtime_error("Glue tensor requires rank 4 and complex<double>");
    auto opt=SB::getOptionsFromXML(std::string(meta.begin(),meta.end()));
    if(SB::getOption<std::string>(*opt,"DBMetaData/id")!="glueElemOp"||SB::getOption<int>(*opt,"DBMetaData/storage_version")!=1)throw std::runtime_error("Unsupported glue tensor schema");
    auto order=SB::getOption<std::string>(*opt,"DBMetaData/tensorOrder");
    if(order.size()!=N||std::set<char>(order.begin(),order.end())!=std::set<char>{'n','t','d','m'})throw std::runtime_error("Invalid glue tensor axes");
    auto kv=detail::to_kvdim(order,dims);for(auto c:std::string("ntdm"))if(kv.at(c)<=0)throw std::runtime_error("Empty glue tensor dimension");
    auto moms=SB::getOption<std::vector<std::array<int,3>>>(*opt,"DBMetaData/moms");auto disps=SB::getOption<std::vector<std::vector<int>>>(*opt,"DBMetaData/displacements");
    if(moms.size()!=kv.at('m')||disps.size()!=kv.at('d')||std::set<std::array<int,3>>(moms.begin(),moms.end()).size()!=moms.size()||std::set<std::vector<int>>(disps.begin(),disps.end()).size()!=disps.size())throw std::runtime_error("Invalid glue dictionaries");
    auto convention=SB::getOption<std::string>(*opt,"DBMetaData/operator_convention");
    if(convention!="GLUEBALL_MATELEM_COLORVEC"&&convention!="GLUEBALL_DIAG_MATELEM_COLORVEC")throw std::runtime_error("Unsupported glue operator convention");
    superbblas::Storage_handle h;superbblas::open_storage<N,T>(file.c_str(),false,&h);Store store{h,order,kv,moms,disps,{}};
    std::vector<superbblas::PartitionItem<N>>blocks;
    superbblas::get_blocks<N,N,T>(h,order.c_str(),order.c_str(),superbblas::Coor<N>{},detail::kvcoors<N>(order,kv),blocks,superbblas::FastToSlow);
    for(const auto&b:blocks){auto lo=detail::to_kvdim(order,b[0]);auto sz=detail::to_kvdim(order,b[1]);if(lo.at('n')!=0||sz.at('n')!=kv.at('n'))continue;
     for(int t=lo.at('t');t<lo.at('t')+sz.at('t');++t)for(int d=lo.at('d');d<lo.at('d')+sz.at('d');++d)for(int m=lo.at('m');m<lo.at('m')+sz.at('m');++m)store.present.insert({t,d,m});}
    stores.push_back(std::move(store));continue;
   }
#endif
   old.push_back(file);
  }return old.empty()?0:legacy.open(old);}catch(...){close();throw;}
 }
 int get(const SerialDBKey<K>&k,SerialDBData<V>&v){return get(k.key(),v.data());}
 int get(const K&key,V&val){
#ifdef USE_SUPERBBLAS
  for(const auto&s:stores){
   if(key.left!=0||key.right!=0||key.t_slice<0||key.t_slice>=s.dims.at('t'))continue;
   auto m=std::find(s.moms.begin(),s.moms.end(),detail::toCoor(key.mom));auto d=std::find(s.disps.begin(),s.disps.end(),key.displacement);
   if(m==s.moms.end()||d==s.disps.end())continue;int n=s.dims.at('n');
   if(!s.present.count({key.t_slice,int(d-s.disps.begin()),int(m-s.moms.begin())}))continue;
   std::vector<T>data(n,T(std::numeric_limits<double>::quiet_NaN(),0));T*ptr=data.data();superbblas::Coor<1>zero{},size{{n}};superbblas::PartitionItem<1>part{zero,size};auto cpu=superbblas::createCpuContext();
   auto from=detail::kvcoors<N>(s.order,{{'t',key.t_slice},{'m',int(m-s.moms.begin())},{'d',int(d-s.disps.begin())}});auto extent=detail::kvcoors<N>(s.order,{{'n',n}},1);
#if SUPERBBLAS_VERSION < 2
   superbblas::load<N,1,T,T>(T(1),s.handle,s.order.c_str(),from,extent,&part,1,"n",zero,&ptr,&cpu,superbblas::FastToSlow,superbblas::Copy);
#else
   superbblas::load<N,1,T,T>(T(1),s.handle,s.order.c_str(),from,extent,&part,1,"n",zero,size,&ptr,&cpu,superbblas::FastToSlow,superbblas::Copy);
#endif
   for(auto z:data)if(!std::isfinite(z.real())||!std::isfinite(z.imag()))return -1;val.op.resize(n);for(int i=0;i<n;++i)val.op[i]=data[i];return 0;
  }
#endif
  SerialDBKey<K>k(key);SerialDBData<V>v;int rc=legacy.get(k,v);if(!rc)val=v.data();return rc;
 }
};
}
#endif

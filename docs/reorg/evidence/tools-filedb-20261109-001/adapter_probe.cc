#include "util/ferm/key_val_db.h"
#include <stdexcept>
#include <string>
#include <fcntl.h>
static void require(bool ok,const char* message){if(!ok){QDP::QDPIO::cerr<<message<<std::endl;QDP::QDP_abort(1);}}
int main(int argc,char**argv){QDP::QDP_initialize(&argc,&argv);if(argc!=2){QDP::QDP_finalize();return 2;}QDP::multi1d<int> dims(4);dims[0]=2;dims[1]=2;dims[2]=2;dims[3]=4;QDP::Layout::setLattSize(dims);QDP::Layout::create();
 const std::string base=argv[1];
 // Every rank independently writes and reads its own serial database.
 {Chroma::LocalBinaryStoreDB<Chroma::LocalSerialDBKey<int>,Chroma::LocalSerialDBData<int>> db(true);db.setMaxUserInfoLen(64);db.open(base+"/local-"+std::to_string(QDP::Layout::nodeNumber())+".sdb",O_RDWR|O_CREAT|O_EXCL,0600);db.insertUserdata("local-metadata");Chroma::LocalSerialDBKey<int> key(7);Chroma::LocalSerialDBData<int> value(100+QDP::Layout::nodeNumber());db.insert(key,value);Chroma::LocalSerialDBData<int> copy;require(db.get(key,copy)==0&&copy.data()==100+QDP::Layout::nodeNumber(),"local value mismatch");std::string meta;db.getUserdata(meta);require(meta=="local-metadata","local metadata mismatch");db.close();}
 // The collective wrapper writes only on the primary node and broadcasts metadata.
 {Chroma::BinaryStoreDB<Chroma::SerialDBKey<int>,Chroma::SerialDBData<int>> db;db.setMaxUserInfoLen(64);db.open(base+"/collective.sdb",O_RDWR|O_CREAT|O_EXCL,0600);db.insertUserdata("collective-metadata");Chroma::SerialDBKey<int> key(11);Chroma::SerialDBData<int> value(29);db.insert(key,value);std::string meta;db.getUserdata(meta);require(meta=="collective-metadata","collective metadata mismatch");if(QDP::Layout::numNodes()==1){Chroma::SerialDBData<int> copy;require(db.get(key,copy)==0&&copy.data()==29,"collective one-rank read mismatch");std::vector<Chroma::SerialDBKey<int>> keys;db.keys(keys);require(keys.size()==1&&keys[0].key()==11,"collective keys mismatch");}db.close();}
 QDP::QDPIO::cout<<"PASS local per-rank read/write; collective insert/metadata; nodes="<<QDP::Layout::numNodes()<<std::endl;QDP::QDP_finalize();return 0;}

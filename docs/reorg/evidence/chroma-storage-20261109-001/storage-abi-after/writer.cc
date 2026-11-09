#include "util/ferm/storage_io.h"
#include <array>
#include <datalib/superb/meson_superb.h>
#include <datalib/superb/baryon_superb.h>
#include <datalib/superb/prop_superb.h>
#include <datalib/superb/genprop4.h>
#include <datalib/superb/disco_superb.h>
#include <datalib/superb/glueball_superb.h>
#include <datalib/superb/tetraquark_superb.h>
std::array<std::size_t,7> writer(){return {sizeof(Datalib::TensorStorage::StorageMeson),sizeof(Datalib::TensorStorage::StorageBaryon),sizeof(Datalib::TensorStorage::StorageProp4),sizeof(Datalib::TensorStorage::StorageGenprop4),sizeof(Datalib::TensorStorage::StorageDisco),sizeof(Datalib::TensorStorage::StorageGlue),sizeof(Datalib::TensorStorage::StorageTetraQuark)};}

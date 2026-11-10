#include <complex>
#include <vector>
/*! \file
 * \brief Read a correlator edb
 */

#include <AllConfStoreDB.h>

#include <datalib/legacy/hadron_sun_npart_npt_corr.h>
#include <datalib/model/map_obj.h>
#include <datalib/storage/filedb.h>

//! Get corrs
Datalib::Model::MapObject<Hadron::KeyHadronSUNNPartNPtCorr_t,
                          std::vector<std::vector<std::complex<double>>>>
getCorrs(const std::string &dbase) {
  typedef Hadron::KeyHadronSUNNPartNPtCorr_t K;
  typedef std::vector<std::complex<double>> V;

  // Open DB
  FILEDB::AllConfStoreDB<Datalib::Storage::SerialDBKey<K>,
                         Datalib::Storage::SerialDBData<V>>
      database;
  if (database.open(dbase, O_RDONLY, 0400) != 0) {
    std::cerr << __func__ << ": error opening dbase= " << dbase << std::endl;
    exit(1);
  }

  // Read all the keys and values
  std::vector<Datalib::Storage::SerialDBKey<K>> keys;
  std::vector<std::vector<Datalib::Storage::SerialDBData<V>>> vals;

  database.keysAndData(keys, vals);

  // Put into something more useful
  Datalib::Model::MapObject<K, std::vector<V>> dest;

  for (int i = 0; i < keys.size(); ++i) {
    std::vector<V> v(vals[i].size());

    for (int n = 0; n < v.size(); ++n) {
      v[n] = vals[i][n].data();
    }

    dest.insert(keys[i].key(), v);
  }

  return dest;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "barf" << std::endl;
    return -1;
  }

  // Read all the correlators from an edb file
  std::cout << "read file = " << argv[1] << std::endl;
  auto corrs = getCorrs(argv[1]);

  for (auto p = corrs.begin(); p != corrs.end(); ++p) {
    std::cout << p->first << "\n";
  }

  return 0;
}

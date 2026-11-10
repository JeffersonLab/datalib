/*! \file
 * \brief Extract diag t0's from a set of props
 */

#include <datalib/storage/filedb.h>
#include <datalib/superb/prop_elem_type.h>

#include "ConfDataStoreDB.h"
#include "ConfDataStoreMultipleDB.h"
#include "DBString.h"

#include <datalib/storage/dbtype.h>

#include <map>

using namespace std;
using namespace Hadron;
using namespace FILEDB;

namespace {
  int usage(int argc, char **argv) {
    cerr << "Usage: " << argv[0]
         << " <new dbasename> <first db> [<second db> <third db> ...]" << endl;
    return -1;
  }
} // namespace

//! Extract keys matching a mass
void doTheWork(int argc, char **argv) {
  typedef KeyPropElementalOperator_t K;
  typedef ValPropElementalOperator_t V;

  // The unique-ified keys
  std::map<std::string, int> uniq_mass;

  // Grab the metadata from the first db
  std::string meta_str;

  // Snarf keys
  for (int n = 2; n < argc; ++n) {
    ConfDataStoreDB<Datalib::Storage::SerialDBKey<K>,
                    Datalib::Storage::SerialDBData<V>>
        dbin;
    if (dbin.open(argv[n], O_RDONLY, 0400) != 0) {
      std::cerr << argv[0] << ": error opening db = " << argv[n] << std::endl;
      exit(1);
    }

    // Grab the metadata from the first db
    if (n == 2) {
      dbin.getUserdata(meta_str);
    }

    // First pass - read all keys and find the uniq mass labels
    std::vector<Datalib::Storage::SerialDBKey<K>> keys;
    dbin.keys(keys);

    for (int i = 0; i < keys.size(); ++i) {
      uniq_mass[keys[i].key().mass_label] = 1;
    }
  }

  // All the dbs treated as one
  ConfDataStoreMultipleDB<Datalib::Storage::SerialDBKey<K>,
                          Datalib::Storage::SerialDBData<V>>
      dbin;
  {
    dbin.setCacheSize(1 * 1024 * 1024 * 1024);

    std::vector<std::string> dbnames;
    for (int n = 2; n < argc; ++n)
      dbnames.push_back(argv[n]);

    if (dbin.open(dbnames) != 0) {
      std::cerr << argv[0] << ": error opening dbs\n";
      exit(1);
    }
  }

  // Get the layout and time extent
  Hadron::Layout::Layout_t layout(Hadron::getDBLayoutFromMetaData(meta_str));
  int Nt = layout.latt_size[layout.decay_dir];

  // Open output DB
  ConfDataStoreDB<Datalib::Storage::SerialDBKey<K>,
                  Datalib::Storage::SerialDBData<V>>
      dbout;
  dbin.setCacheSize(1024 * 1024 * 1024);
  dbout.setMaxUserInfoLen(meta_str.size());
  dbout.open(argv[1], O_RDWR | O_CREAT | O_TRUNC, 0644);
  dbout.insertUserdata(meta_str);

  // Second pass - use unique keys to force all possible t_slices and spins
  for (std::map<std::string, int>::const_iterator pp = uniq_mass.begin();
       pp != uniq_mass.end(); ++pp) {
    for (int t = 0; t < Nt; ++t) {
      for (int l = 0; l < Datalib::TensorStorage::spin_count; ++l) {
        for (int r = 0; r < Datalib::TensorStorage::spin_count; ++r) {
          Datalib::Storage::SerialDBKey<K> key;
          Datalib::Storage::SerialDBData<V> val;

          key.key().t_slice = t;
          key.key().t_source = t;
          key.key().spin_l = l;
          key.key().spin_r = r;
          key.key().mass_label = pp->first;

          int ret;
          if ((ret = dbin.get(key, val)) != 0) {
            //	    std::cerr << __func__ << ": key not found\n" << key;
            std::cerr << __func__ << ": key not found for t= " << t
                      << "  mass= " << key.key().mass_label << "\n";
            exit(1);
          }

          dbout.insert(key, val);
        }
      }
    }
  }
}

int main(int argc, char **argv) {
  // Parse the arguments
  if (argc < 3) {
    return usage(argc, argv);
  }

  // Read the dbtype from the metadata
  string dbtype = getDBType(argv[2]);

  // Extract the diag t0's
  doTheWork(argc, argv);

  return 0;
}

#include "dbtools_support.h"
#include <complex>
#include <vector>
/*! \file
 * \brief Add together several DB's into one DB
 */

#include <stdlib.h>

#include <datalib/io/serial_xml.h>
#include <datalib/storage/filedb.h>

#include "AllConfStoreDB.h"
#include "DBString.h"
#include <datalib/storage/dbtype.h>

#include <datalib/legacy/hadron_1pt_corr.h>
#include <datalib/legacy/hadron_2pt_corr.h>
#include <datalib/legacy/hadron_3pt_corr.h>

#include <datalib/legacy/hadron_npart_npt_corr.h>
#include <datalib/legacy/hadron_sun_npart_npt_corr.h>

using namespace std;
using namespace Hadron;
using namespace FILEDB;
using namespace FF;
using namespace Hadron;

namespace {
  int usage(int argc, char **argv) {
    cerr << "Usage: " << argv[0]
         << " <new dbasename> <first db>  [<second db>  ...]" << endl;
    return -1;
  }
} // namespace

//! Get a key/value
template <typename K, typename SV>
std::vector<Datalib::Storage::SerialDBData<SV>>
printKeyValue(AllConfStoreDB<Datalib::Storage::SerialDBKey<K>,
                             Datalib::Storage::SerialDBData<SV>> &database,
              Datalib::Storage::SerialDBKey<K> &key) {
  std::vector<Datalib::Storage::SerialDBData<SV>> vals;
  int ret;
  if ((ret = database.get(key, vals)) != 0) {
    std::cerr << __func__ << ": key not found\n" << key.key();
    exit(1);
  }

  return vals;
}

//! Get a key/value
template <typename K, typename SV>
void insertKeyValue(
    AllConfStoreDB<Datalib::Storage::SerialDBKey<K>,
                   Datalib::Storage::SerialDBData<SV>> &database,
    Datalib::Storage::SerialDBKey<K> &key,
    std::vector<Datalib::Storage::SerialDBData<SV>> &vals) {
  int ret;
  if ((ret = database.insert(key, vals)) != 0) {
    std::cerr << __func__ << ": error inserting key\n" << key.key();
    exit(1);
  }
}

//! Combine the DB-s
template <typename K> void keyHandler(int argc, char **argv) {
  using SV = std::vector<std::complex<double>>;

  // Hold all DBs
  std::vector<AllConfStoreDB<Datalib::Storage::SerialDBKey<K>,
                             Datalib::Storage::SerialDBData<SV>>>
      database(argc);
  int Ncors = (argc - 2);
  double weight = 1.0 / Ncors;
  std::cout << "Number to average: " << Ncors << std::endl;
  std::cout << "Normalization: " << weight << std::endl;
  // Loop over each DB
  for (int n = 2; n < argc; n += 1) {
    // Open DB
    database[n].open(argv[n], O_RDONLY, 0400);
  }

  // Grab the metadata from the first db
  std::string meta;
  database[2].getUserdata(meta);

  // Open DB
  int nbins = database[2].size();
  std::vector<int> cfgs(nbins);

  for (int i = 0; i < nbins; ++i) {
    cfgs[i] = i;
  }

  AllConfStoreDB<Datalib::Storage::SerialDBKey<K>,
                 Datalib::Storage::SerialDBData<SV>>
      dbnew(cfgs);

  // Setup for the new DB
  dbnew.setMaxUserInfoLen(meta.size());

  // Open it
  if (dbnew.open(argv[1], O_RDWR | O_TRUNC | O_CREAT, 0664) != 0) {
    std::cerr << __func__ << ": error opening dbase= " << argv[1] << std::endl;
    exit(1);
  }

  // Insert new DB-meta
  dbnew.insertUserdata(meta);

  // The keys all come from the first db
  std::vector<Datalib::Storage::SerialDBKey<K>> keys;
  database[2].keys(keys);

  // Loop over each key
  for (int i = 0; i < keys.size(); ++i) {
    // The first val determines the size, etc.
    std::vector<Datalib::Storage::SerialDBData<SV>> val_avg =
        printKeyValue<K, SV>(database[2], keys[i]);
    for (int k = 0; k < val_avg.size(); ++k)
      Datalib::Tools::scale(val_avg[k].data(), weight);

    // Loop over each additional DB
    for (int n = 3; n < argc; n += 1) {
      // Read this term
      std::vector<Datalib::Storage::SerialDBData<SV>> val =
          printKeyValue<K, SV>(database[n], keys[i]);

      // Accumulate
      for (int k = 0; k < val_avg.size(); ++k)
        Datalib::Tools::addScaled(val_avg[k].data(), val[k].data(), weight);
    }

    // Insert
    insertKeyValue(dbnew, keys[i], val_avg);
  }
}

int main(int argc, char **argv) {
  // Register all the handlers

  // Parse the arguments
  if (argc < 3) {
    return usage(argc, argv);
  }

  // Read the dbtype from the metadata
  string dbtype = getDBType(argv[2]);

  // Call the appropriate handler
  try {
    // Call the approriate function
    Datalib::Tools::dispatch(dbtype,
                             [&]<class K>() { keyHandler<K>(argc, argv); });
  } catch (const string &e) {
    std::cerr << argv[0] << ": error message: " << e << endl;
    exit(1);
  }

  return 0;
}

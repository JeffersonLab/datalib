#include "dbtools_support.h"
#include <complex>
#include <vector>
/*! \file
 * \brief Bin a DB
 */

#include <algorithm>
#include <filesystem>
#include <stdexcept>
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

namespace {
  int usage(int argc, char **argv) {
    cerr << "Usage: " << argv[0] << " <new dbasename> <first db> <bin size>"
         << endl;
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

//! Bin configurations in a database
template <typename K>
void keyHandler(const string &dbout_name, const string &dbin_name, int nb) {
  using SV = std::vector<std::complex<double>>;
  using Scalar_t = double;

  // Hold all DBs
  AllConfStoreDB<Datalib::Storage::SerialDBKey<K>,
                 Datalib::Storage::SerialDBData<SV>>
      dbin;

  // Open DB
  if (dbin.open(dbin_name, O_RDONLY, 0400) != 0) {
    std::cerr << __func__ << ": error opening dbase= " << dbin_name
              << std::endl;
    exit(1);
  }

  // Grab the metadata from the first db
  std::string meta;
  dbin.getUserdata(meta);

  // Get size info
  int num = dbin.size();
  if (num <= 0)
    throw std::runtime_error("Input database contains no configurations");
  int num_nb = num / nb;
  if (num % nb != 0)
    ++num_nb;

  std::vector<int> cfgs(num_nb);

  for (int i = 0; i < num_nb; ++i) {
    cfgs[i] = i;
  }

  AllConfStoreDB<Datalib::Storage::SerialDBKey<K>,
                 Datalib::Storage::SerialDBData<SV>>
      dbout(cfgs);

  // Setup for the new DB
  dbout.setMaxUserInfoLen(meta.size());

  // Open it
  if (dbout.open(dbout_name, O_RDWR | O_TRUNC | O_CREAT, 0664) != 0) {
    std::cerr << __func__ << ": error opening dbase= " << dbout_name
              << std::endl;
    exit(1);
  }

  // Insert new DB-meta
  dbout.insertUserdata(meta);

  // The keys all come from the first db
  std::vector<Datalib::Storage::SerialDBKey<K>> keys;
  dbin.keys(keys);

  // Bin configurations independently for every key. Retain a final partial bin.
  for (std::size_t key_index = 0; key_index < keys.size(); ++key_index) {
    const auto src = printKeyValue<K, SV>(dbin, keys[key_index]);
    if (src.size() != static_cast<std::size_t>(num))
      throw std::runtime_error("Key has an unexpected configuration count");
    const int nobs = src.front().data().size();
    for (const auto &value : src)
      if (value.data().size() != nobs)
        throw std::runtime_error("Key has inconsistent observable counts");

    std::vector<Datalib::Storage::SerialDBData<SV>> bins;
    bins.reserve(num_nb);
    for (int first = 0; first < num;) {
      const int count = std::min(nb, num - first);
      Datalib::Storage::SerialDBData<SV> average;
      average.data().resize(nobs);

      for (int offset = 0; offset < count; ++offset)
        Datalib::Tools::addScaled(average.data(), src[first + offset].data(),
                                  1.0);
      Datalib::Tools::divide(average.data(), Scalar_t(count));
      bins.push_back(average);
      first += count;
    }
    insertKeyValue(dbout, keys[key_index], bins);
  }
}

int main(int argc, char **argv) {
  // Parse the arguments
  if (argc != 4) {
    return usage(argc, argv);
  }

  // Read the dbtype from the metadata
  string dbout = argv[1];
  string dbin = argv[2];

  // Call the appropriate handler
  try {
    const std::string bin_argument = argv[3];
    std::size_t parsed = 0;
    const int bsize = std::stoi(bin_argument, &parsed);
    if (parsed != bin_argument.size() || bsize <= 0)
      throw std::runtime_error("Bin size must be a positive integer");
    if (std::filesystem::exists(dbout) &&
        std::filesystem::equivalent(dbout, dbin))
      throw std::runtime_error(
          "Input and output databases must be different files");
    const std::string dbtype = getDBType(dbin);
    Datalib::Tools::dispatch(
        dbtype, [&]<class K>() { keyHandler<K>(dbout, dbin, bsize); });
  } catch (const string &e) {
    std::cerr << argv[0] << ": error message: " << e << endl;
    return 1;
  } catch (const std::exception &e) {
    std::cerr << argv[0] << ": error message: " << e.what() << endl;
    return 1;
  }

  return 0;
}

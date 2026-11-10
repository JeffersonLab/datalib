#include <complex>
#include <vector>
/*! \file
 * \brief Average over source location from several dbs
 */

#include <cstdlib>
#include <functional>
#include <unordered_map>

#include <datalib/io/serial_xml.h>
#include <datalib/storage/corr_reader.h>
#include <datalib/storage/filedb.h>

#include "AllConfStoreDB.h"
#include <datalib/storage/dbtype.h>

#include <datalib/graph/hadron_sun_class_npart_npt_corr.h>
#include <datalib/legacy/hadron_npart_npt_corr.h>
#include <datalib/legacy/hadron_sun_npart_npt_corr.h>

#include <datalib/graph/filenames.h>

using namespace std;
using namespace Hadron;
using namespace FILEDB;

using namespace Hadron;

using Filenames = std::vector<std::string>;

template <typename K> K get_avg_key(const K &key) {
  K k = key;
  k.npoint[k.npoint.size()].t_slice = -k.npoint.size();
  return k;
}

template <typename K> std::string get_string_key(const K &key) {
  return ensemFileName(key);
}

template <typename K>
void keyHandler(const Filenames &filenames, const std::string &new_filename) {
  // Loop over each DB
  bool is_first = true; ///< whether processing the first file
  int number_of_configurations =
      0;            ///< expected number of configurations in each file
  std::string meta; ///< metadata for the new database
  int NtCorr = 0;   ///< number of time-slices
  using Value = std::vector<std::vector<std::complex<double>>>;
  std::unordered_map<std::string, std::pair<Value, unsigned int>> avg_values(
      1024);                                         ///< output values
  std::unordered_map<std::string, K> avg_keys(1024); ///< output keys
  for (const auto &filename : filenames) {
    // Open DB
    Datalib::Storage::StorageCorrFunc<K> database;
    database.open_for_reading(filename);

    // Check that all given files have the same number of configurations inside
    // and grab the metadata
    if (is_first) {
      meta = database.metadata;
      NtCorr = database.NtCorr;
      number_of_configurations = database.num_configs;
    } else if (database.num_configs != number_of_configurations) {
      throw std::runtime_error(
          std::string("Error: the following file has a different number of "
                      "configurations, ") +
          filename);
    }

    // Get all keys and values from the DB
    const auto &[keys, values] = database.keys_and_data();

    // Insert the values
    for (std::size_t i = 0; i < keys.size(); ++i) {
      const auto &val = values[i];
      auto avg_key = get_avg_key(keys[i]);
      auto str_avg_key = get_string_key(avg_key);
      auto it = avg_values.find(str_avg_key);
      if (it == avg_values.end()) {
        avg_keys.emplace(str_avg_key, avg_key);
        avg_values.emplace(str_avg_key,
                           std::make_pair(val, static_cast<unsigned int>(1)));
      } else {
        auto &avg_value = it->second.first;
        for (int k = 0; k < number_of_configurations; ++k)
          for (int t = 0; t < NtCorr; ++t)
            avg_value[k][t] += val[k][t];

        // Increase the number of average keys
        it->second.second++;
      }
    }

    is_first = false;
  }

  // Check that all keys have the same number of statistics and make the
  // normalization
  is_first = true;
  unsigned int number_of_stats = 0;
  for (auto &it : avg_values) {
    if (is_first)
      number_of_stats = it.second.second;
    else if (number_of_stats != it.second.second)
      throw std::runtime_error("some key has different statistics");

    auto &avg_value = it.second.first;
    for (int k = 0; k < number_of_configurations; ++k)
      for (int t = 0; t < NtCorr; ++t)
        avg_value[k][t] *= 1.0 / number_of_stats;

    is_first = false;
  }

  // Open the new DB
  Datalib::Storage::StorageCorrFunc<K> dbnew;
  dbnew.open_for_writing(new_filename, number_of_configurations, meta, NtCorr);

  // Insert the keys
  std::vector<K> new_keys;
  new_keys.reserve(avg_keys.size());
  for (const auto &it : avg_keys)
    new_keys.push_back(it.second);
  dbnew.insert_keys(new_keys);

  // Insert the keys and values
  for (const auto &it : avg_values)
    if (dbnew.insert(avg_keys.at(it.first), it.second.first) != 0)
      throw std::runtime_error("Error inserting keys in the new database");
}

int main(int argc, char **argv) {
  // Show usage if the number of arguments is not enough
  if (argc < 3) {
    std::cout << "Usage: " << argv[0]
              << " <new dbasename> <first db> [<second db> ...]" << endl;
    return -1;
  }

  // Process commandline arguments
  std::string new_filename(argv[1]);
  Filenames filenames;
  for (int n = 2; n < argc; n++) {
    filenames.push_back(std::string(argv[n]));
  }

  // Read the dbtype from the metadata
  string dbtype = getDBType(filenames.at(0));

  std::unordered_map<std::string, std::function<void(Filenames, std::string)>>
      handlers{{"hadronNPartNPtCorr", keyHandler<KeyHadronNPartNPtCorr_t>},
               {"hadronSUNNPartNPtCorr",
                keyHandler<KeyHadronSUNClassNPartNPtCorr_t>},
               {"hadronSUNClassNPartNPtCorr",
                keyHandler<KeyHadronSUNClassNPartNPtCorr_t>}};

  // Call the appropriate handler
  try {
    if (handlers.count(dbtype) == 0)
      throw std::runtime_error(std::string("Unsupported type: ") + dbtype);
    handlers.at(dbtype)(filenames, new_filename);
  } catch (const string &e) {
    std::cerr << argv[0] << ": error message: " << e << endl;
    exit(1);
  } catch (const std::exception &e) {
    std::cerr << argv[0] << ": error message: " << e.what() << std::endl;
    exit(1);
  } catch (...) {
    std::cerr << argv[0] << ": caught generic exception\n";
    exit(1);
  }

  return 0;
}

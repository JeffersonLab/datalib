#include <complex>
#include <vector>
/*! \file
 * \brief Merge many single dbs into one db.
 */

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <datalib/legacy/hadron_1pt_corr.h>
#include <datalib/legacy/hadron_2pt_corr.h>
#include <datalib/legacy/hadron_3pt_corr.h>
#include <datalib/storage/dbtype.h>

#include <datalib/graph/hadron_sun_class_npart_npt_corr.h>
#include <datalib/graph/hadron_timeslice.h>
#include <datalib/legacy/hadron_npart_npt_corr.h>
#include <datalib/legacy/hadron_sun_npart_npt_corr.h>

#include <datalib/storage/corr_reader.h>

template <typename K>
void combineWork(const std::vector<std::string> &filenames,
                 const std::string new_filename) {
  // Loop over each DB
  std::string meta;    ///< metadata for the new database
  int NtCorr = 0;      ///< number of time-slices
  int num_configs = 0; ///< number of configurations
  using Value = std::vector<std::vector<std::complex<double>>>;
  std::unordered_map<std::string, Value> v(
      1024);              ///< values for a single configuration
  bool first_file = true; ///< whether processing the first file

  for (const auto &filename : filenames) {
    // Open DB
    Datalib::Storage::StorageCorrFunc<K> database;
    database.open_for_reading(filename);

    // Grab the metadata the first time
    if (first_file) {
      meta = database.metadata;
    }

    // Get the number of configurations and make sure it is consistent for all
    // files
    if (first_file) {
      num_configs = database.num_configs;
    } else if (num_configs != database.num_configs) {
      throw std::runtime_error(
          std::string("input file does not have the same number of "
                      "configurations as other files: ") +
          filename);
    }

    // Get all keys and values from the DB
    const auto &[keys, values] = database.string_keys_and_data();

    // Accumulate the values for the same configuration
    for (std::size_t i = 0; i < keys.size(); ++i) {
      if (values[i].size() != 1) {
        throw std::runtime_error(
            std::string("input file does not have a single configuration: ") +
            filename);
      }

      if (first_file && i == 0)
        NtCorr = values[i].at(0).size();
      else if (NtCorr != values[i].at(0).size())
        throw std::runtime_error(
            std::string("input file has keys with different NtCorr: ") +
            filename);

      if (v.find(keys[i]) != v.end()) {
        throw std::runtime_error("some key is going to be overwritten");
      }
      v.emplace(keys[i], values[i]);
    }

    first_file = false;
  }

  // Create the database
  Datalib::Storage::StorageCorrFunc<K> dbnew;
  dbnew.open_for_writing(new_filename, num_configs, meta, NtCorr);

  // Insert the keys
  std::vector<std::string> new_keys;
  new_keys.reserve(v.size());
  for (const auto &it : v)
    new_keys.push_back(it.first);
  dbnew.insert_string_keys(new_keys);

  // Insert the keys and values at the row `cnf`
  for (const auto &it : v)
    if (dbnew.insert_from_string_key(it.first, it.second) != 0)
      throw std::runtime_error("Error inserting keys in the new database");
}

int main(int argc, char **argv) {
  // Parse the argument
  if (argc < 3) {
    std::cout << "Usage: " << argv[0]
              << " <new dbasename> <first edb> [<second edb> ...]" << std::endl
              << "NOTE: both sdb and edb files are supported" << std::endl;
    return -1;
  }

  const std::string new_filename = argv[1];

  try {
    // construct configuration list and database list
    std::vector<std::string> filenames;
    for (int i = 2; i < argc; ++i)
      filenames.push_back(std::string(argv[i]));

    // Read the dbtype from the metadata of the first file
    std::string dbtype = Hadron::getDBType(filenames.at(0));

    // Call the proper handler
    std::unordered_map<
        std::string, std::function<void(std::vector<std::string>, std::string)>>
        handlers{{"hadron1Pt", combineWork<FF::KeyHadron1PtCorr_t>},
                 {"hadron2Pt", combineWork<FF::KeyHadron2PtCorr_t>},
                 {"hadron3Pt", combineWork<FF::KeyHadron3PtCorr_t>},
                 {"hadronNPartNPtCorr",
                  combineWork<Hadron::KeyHadronNPartNPtCorr_t>},
                 {"hadronSUNNPartNPtCorr",
                  combineWork<Hadron::KeyHadronSUNClassNPartNPtCorr_t>},
                 {"hadronSUNClassNPartNPtCorr",
                  combineWork<Hadron::KeyHadronSUNClassNPartNPtCorr_t>}};
    if (handlers.count(dbtype) == 0)
      throw std::runtime_error(std::string("Unsupported type: ") + dbtype);
    handlers.at(dbtype)(filenames, new_filename);
  } catch (const std::string &e) {
    std::cerr << argv[0] << ": error message: " << e << std::endl;
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

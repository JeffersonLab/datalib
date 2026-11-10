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

using Indices_and_filenames = std::vector<std::pair<unsigned int, std::string>>;

template <typename K>
void mergeWork(const Indices_and_filenames &indices_and_filenames,
               std::size_t num_configs, const std::string new_filename) {
  // Loop over each DB
  std::string meta; ///< metadata for the new database
  int NtCorr = 0;   ///< number of time-slices
  using Value = std::vector<std::vector<std::complex<double>>>;
  std::unordered_map<std::string, Value> v(
      1024); ///< values for a single configuration
  Datalib::Storage::StorageCorrFunc<K> dbnew; ///< new database

  for (std::size_t if_i = 0; if_i < indices_and_filenames.size(); ++if_i) {
    const auto &index_and_filename = indices_and_filenames.at(if_i);
    const auto cnf = index_and_filename.first;        ///< configuration index
    const auto &filename = index_and_filename.second; ///< filename

    // Open DB
    Datalib::Storage::StorageCorrFunc<K> database;
    database.open_for_reading(filename);

    // Grab the metadata
    if (if_i == 0) {
      meta = database.metadata;
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

      if (if_i == 0)
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

    if (if_i + 1 == indices_and_filenames.size() ||
        cnf != indices_and_filenames.at(if_i + 1).first) {
      if (cnf == 0) {
        // Create the database
        dbnew.open_for_writing(new_filename, num_configs, meta, NtCorr);

        // Insert the keys
        std::vector<std::string> new_keys;
        new_keys.reserve(v.size());
        for (const auto &it : v)
          new_keys.push_back(it.first);
        dbnew.insert_string_keys(new_keys);
      }

      // Insert the keys and values at the row `cnf`
      for (const auto &it : v)
        if (dbnew.insert_from_string_key(it.first, it.second, cnf) != 0)
          throw std::runtime_error("Error inserting keys in the new database");

      // Clear v
      v.clear();
    }
  }
}

int main(int argc, char **argv) {
  // Parse the argument
  // NOTE: accept a 4th argument although not being used for back compatibility
  if (argc != 3 && argc != 4) {
    std::cout
        << "Usage: " << argv[0] << " <dbasename> <list file>" << std::endl
        << "The list is a file containing two columns: confignum dbasename"
        << std::endl
        << "where confignum are configuration numbers, and " << std::endl
        << "dbasename are database name for the configuration number. Example:"
        << std::endl
        << "0 corr_1000.sdb" << std::endl
        << "1 corr_1010.sdb" << std::endl
        << std::endl;
    return -1;
  }

  std::string new_filename = argv[1];
  std::string lfile = argv[2];

  try {
    // construct configuration list and database list
    Indices_and_filenames indices_and_filenames;
    std::ifstream ifs(lfile);
    if (!ifs.is_open())
      throw std::runtime_error(std::string("Error opening the file ") + lfile);
    unsigned int index;
    std::string filename;
    unsigned int max_index = 0; ///< maximum configuration index
    std::unordered_set<unsigned int> indices;
    while (ifs >> index >> filename) {
      max_index = std::max(max_index, index);
      indices.insert(index);
      indices_and_filenames.push_back({index, filename});
    }

    if (indices.size() != max_index + 1)
      throw std::runtime_error(
          "invalid given indices, the smallest index should be zero and the "
          "largest should the number of different gauge configurations minus "
          "one");

    if (indices_and_filenames.size() == 0)
      throw std::runtime_error("the given list of configurations is empty");

    // Sort the files in ascending index
    std::sort(indices_and_filenames.begin(), indices_and_filenames.end(),
              [](const auto &a, const auto &b) { return a.first < b.first; });

    // Read the dbtype from the metadata of the first file
    std::string dbtype = Hadron::getDBType(indices_and_filenames.at(0).second);

    // Call the proper handler
    std::unordered_map<
        std::string,
        std::function<void(Indices_and_filenames, std::size_t, std::string)>>
        handlers{
            {"hadron1Pt", mergeWork<FF::KeyHadron1PtCorr_t>},
            {"hadron2Pt", mergeWork<FF::KeyHadron2PtCorr_t>},
            {"hadron3Pt", mergeWork<FF::KeyHadron3PtCorr_t>},
            {"hadronNPartNPtCorr", mergeWork<Hadron::KeyHadronNPartNPtCorr_t>},
            {"hadronSUNNPartNPtCorr",
             mergeWork<Hadron::KeyHadronSUNClassNPartNPtCorr_t>},
            {"hadronSUNClassNPartNPtCorr",
             mergeWork<Hadron::KeyHadronSUNClassNPartNPtCorr_t>}};
    if (handlers.count(dbtype) == 0)
      throw std::runtime_error(std::string("Unsupported type: ") + dbtype);
    handlers.at(dbtype)(indices_and_filenames, indices.size(), new_filename);
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

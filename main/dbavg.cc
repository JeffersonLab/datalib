#include <complex>
#include <vector>
/*! \file
 * \brief Add together several DB's into one DB
 */

#include <cstdlib>
#include <functional>
#include <unordered_map>

#include <datalib/io/serial_xml.h>
#include <datalib/storage/corr_reader.h>
#include <datalib/storage/filedb.h>

#include <datalib/storage/dbtype.h>

#include <datalib/legacy/hadron_1pt_corr.h>
#include <datalib/legacy/hadron_2pt_corr.h>
#include <datalib/legacy/hadron_3pt_corr.h>

#include <datalib/graph/hadron_sun_class_npart_npt_corr.h>
#include <datalib/legacy/hadron_npart_npt_corr.h>
#include <datalib/legacy/hadron_sun_npart_npt_corr.h>

using namespace std;
using namespace Hadron;
using namespace FILEDB;
using namespace FF;
using namespace Hadron;

using Filenames_and_weights = std::vector<std::pair<std::string, double>>;

template <typename K>
void keyHandler(const Filenames_and_weights &filenames_and_weights,
                const std::string &new_filename) {
  // Loop over each DB
  bool is_first = true; ///< whether processing the first file
  int number_of_configurations =
      0;            ///< expected number of configurations in each file
  std::string meta; ///< metadata for the new database
  int NtCorr = 0;   ///< number of time-slices
  using Value = std::vector<std::vector<std::complex<double>>>;
  std::unordered_map<std::string, Value> avg_values(1024); ///< output values
  for (const auto &filename_and_weight : filenames_and_weights) {
    // Open DB
    Datalib::Storage::StorageCorrFunc<K> database;
    database.open_for_reading(filename_and_weight.first);

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
          filename_and_weight.first);
    } else if (database.NtCorr != NtCorr) {
      throw std::runtime_error(
          std::string(
              "Error: the following file has a different number of NtCorr, ") +
          filename_and_weight.first);
    }

    // Get all keys and values from the DB
    const auto &[keys, values] = database.string_keys_and_data();
    if (!is_first && keys.size() != avg_values.size())
      throw std::runtime_error(
          std::string("Error: the following file is missing some keys, ") +
          filename_and_weight.first);

    // Insert the values
    const auto &weight = filename_and_weight.second;
    for (std::size_t i = 0; i < keys.size(); ++i) {
      if (is_first) {
        auto val = values[i];
        for (auto &it_cnf : val)
          for (auto &it_t : it_cnf)
            it_t *= weight;
        avg_values.emplace(keys[i], val);
      } else {
        auto it = avg_values.find(keys[i]);
        if (it == avg_values.end())
          throw std::runtime_error(
              std::string("Error: the following file is missing some keys, ") +
              filename_and_weight.first);
        auto &avg_value = it->second;
        const auto &val = values[i];
        for (int k = 0; k < number_of_configurations; ++k)
          for (int t = 0; t < NtCorr; ++t)
            avg_value[k][t] += val[k][t] * weight;
      }
    }

    is_first = false;
  }

  // Open the new DB
  Datalib::Storage::StorageCorrFunc<K> dbnew;
  dbnew.open_for_writing(new_filename, number_of_configurations, meta, NtCorr);

  // Insert the keys
  std::vector<std::string> new_keys;
  new_keys.reserve(avg_values.size());
  for (const auto &it : avg_values)
    new_keys.push_back(it.first);
  dbnew.insert_string_keys(new_keys);

  // Insert the keys and values
  for (const auto &it : avg_values)
    if (dbnew.insert_from_string_key(it.first, it.second) != 0)
      throw std::runtime_error("Error inserting keys in the new database");
}

int main(int argc, char **argv) {
  // Show usage if the number of arguments is not enough
  if (argc < 4 || (argc % 2) != 0) {
    std::cout << "Usage: " << argv[0]
              << " <new dbasename> <first db> <first weight> [<second db> "
                 "<second weight> ...]"
              << endl;
    return -1;
  }

  // Process commandline arguments
  std::string new_filename(argv[1]);
  Filenames_and_weights filenames_and_weights;
  for (int n = 2; n < argc; n += 2) {
    filenames_and_weights.push_back(
        std::make_pair(std::string(argv[n]), std::atof(argv[n + 1])));
  }

  // Read the dbtype from the metadata
  string dbtype = getDBType(filenames_and_weights.at(0).first);

  std::unordered_map<std::string,
                     std::function<void(Filenames_and_weights, std::string)>>
      handlers{{"hadron1Pt", keyHandler<KeyHadron1PtCorr_t>},
               {"hadron2Pt", keyHandler<FF::KeyHadron2PtCorr_t>},
               {"hadron3Pt", keyHandler<FF::KeyHadron3PtCorr_t>},
               {"hadronNPartNPtCorr", keyHandler<KeyHadronNPartNPtCorr_t>},
               {"hadronSUNNPartNPtCorr",
                keyHandler<KeyHadronSUNClassNPartNPtCorr_t>},
               {"hadronSUNClassNPartNPtCorr",
                keyHandler<KeyHadronSUNClassNPartNPtCorr_t>}};

  // Call the appropriate handler
  try {
    if (handlers.count(dbtype) == 0)
      throw std::runtime_error(std::string("Unsupported type: ") + dbtype);
    handlers.at(dbtype)(filenames_and_weights, new_filename);
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

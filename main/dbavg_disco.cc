/*! \file
 * \brief Average several disco DB into a single one
 */

#include <algorithm>
#include <array>
#include <complex>
#include <stdlib.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <datalib/io/serial_xml.h>
#include <datalib/superb/disco_superb.h>

#include <datalib/storage/dbtype.h>

using namespace Hadron;
using namespace FILEDB;
using namespace Hadron;

/// Print a vector `o` into a stream `s`

template <typename Ostream, typename T>
Ostream &operator<<(Ostream &s, const std::vector<T> &o) {
  for (const auto &i : o)
    s << i << " ";
  return s;
}

/// Print an array `o` into a stream `s`

template <typename Ostream, typename T, std::size_t N>
Ostream &operator<<(Ostream &s, const std::array<T, N> &o) {
  for (const auto &i : o)
    s << i << " ";
  return s;
}

/// Return a momentum or a path ordered ascendantly by the absolute value
/// Example: canonize({-1,0}) -> {0,1}

template <typename Container> Container canonize(const Container &v0) {
  Container v = v0;
  for (auto &it : v)
    it = std::abs(it);
  std::sort(v.begin(), v.end());
  return v;
}

template <typename Vector> struct HashVector {
  std::size_t operator()(const Vector &k) const {
    auto h = std::hash<int>{}(12345);
    for (const auto &it : k)
      h = (h << 1) ^ std::hash<int>{}(it);
    return h;
  }
};

//! Combine the DB-s for disco
void discoKeyMerger(std::vector<std::string> db_names,
                    const std::string &new_db_name, bool print_stats = false) {
  using K = KeyTwoQuarkDiscoOperator_t;
  using V = ValTwoQuarkDiscoOperator_t;

  // Return if no databases to average
  if (db_names.size() == 0)
    throw std::runtime_error("unexpected situation");

  // Hold all DBs
  std::vector<Datalib::TensorStorage::StorageDisco> database(db_names.size());
  std::vector<int> noise_vectors(db_names.size());
  std::vector<std::string> metas(db_names.size());

  std::vector<int> latt_size; // Total lattice size
  int decay_dir;              // The decay direction within the lattice
  int probing_distance;       // Probing schema parameter
  int probing_power;          // Probing schema parameter
  std::string mass_label;     // Mass label
  int num_colors = -1;        // Total number of colors for the probing schema
  std::vector<unsigned int>
      cover; // number of configurations that computed that color
  int total_noise_vectors =
      0; // collect total number of noise vectors across all databases
  std::vector<unsigned int> trace_index(
      db_names.size()); // bin to store the trace
  std::unordered_set<std::vector<int>, HashVector<std::vector<int>>> disps(
      16); // all displacements
  std::unordered_set<std::array<int, 3>, HashVector<std::array<int, 3>>> moms(
      16); // all momenta

  // Loop over each DB
  for (int n = 0; n < db_names.size(); ++n) {
    // Open DB
    database[n].open({db_names[n]});

    // Grab the metadata from the first db
    const auto &meta = database[n].meta.at(0);
    metas[n] = meta;
    std::istringstream xml_s(meta);
    Datalib::Serial::XMLReader xml_meta(xml_s);

    Layout::Layout_t layout;
    read(xml_meta, "/", layout);
    int this_probing_distance;
    read(xml_meta, "/DBMetaData/Params/probing_distance",
         this_probing_distance);
    int this_probing_power;
    read(xml_meta, "/DBMetaData/Params/probing_power", this_probing_power);
    std::string this_mass_label;
    read(xml_meta, "/DBMetaData/Params/mass_label", this_mass_label);
    read(xml_meta, "/DBMetaData/Params/noise_vectors", noise_vectors[n]);
    if (xml_meta.count("/DBMetaData/first_computed_color") > 0) {
      int first_computed_color;
      read(xml_meta, "/DBMetaData/first_computed_color", first_computed_color);
      int num_computed_colors;
      read(xml_meta, "/DBMetaData/num_computed_colors", num_computed_colors);
      int this_num_colors;
      read(xml_meta, "/DBMetaData/num_colors", this_num_colors);
      if (num_colors == -1) {
        if (this_num_colors <= 0) {
          std::cerr << "Error: invalid number of colors, " << db_names[n]
                    << std::endl;
          exit(1);
        }
        num_colors = this_num_colors;
        cover = std::vector<unsigned int>(num_colors, total_noise_vectors);
      } else if (num_colors != this_num_colors) {
        std::cerr << "Error: inconsistent number of colors across databases, "
                  << db_names[n] << std::endl;
        exit(1);
      }
      trace_index[n] = cover.at(first_computed_color);
      for (int i = 0; i < num_computed_colors; ++i)
        cover.at(first_computed_color + i) += noise_vectors[n];
      if (first_computed_color == 0)
        total_noise_vectors += noise_vectors[n];
    } else {
      trace_index[n] = (num_colors == -1 ? total_noise_vectors : cover.front());
      for (auto &it : cover)
        it += noise_vectors[n];
      total_noise_vectors += noise_vectors[n];
    }

    if (layout.dbtype != std::string("DiscoBlocks")) {
      std::cerr
          << "Error: the following database does not contain disco loops: "
          << db_names[n] << std::endl;
      exit(1);
    }

    if (n == 0) {
      latt_size = layout.latt_size;
      decay_dir = layout.decay_dir;
      probing_distance = this_probing_distance;
      probing_power = this_probing_power;
      mass_label = this_mass_label;
    } else if (latt_size != layout.latt_size || decay_dir != layout.decay_dir) {
      std::cerr << "Error: some database does not have the same layout as the "
                   "first one, "
                << db_names[n] << std::endl;
      exit(1);
    } else if (this_probing_distance != probing_distance ||
               this_probing_power != probing_power) {
      std::cerr << "Error: some database does not have the same probing schema "
                   "as the first one, "
                << db_names[n] << std::endl;
      exit(1);
    } else if (mass_label != this_mass_label) {
      std::cerr << "Error: some database does not have the same mass label as "
                   "the first one, "
                << db_names[n] << std::endl;
      exit(1);
    }
  }

  // Check that all colors schemas are complete
  if (cover.size() > 0) {
    for (const auto &it : cover) {
      if (it != cover.front()) {
        std::cerr << "Error: missing computed probing colors" << std::endl;
        exit(1);
      }
    }
  }

  // The keys all come from the first db
  std::vector<K> keys;
  database.front().keys(keys);

  // Find out all the displacements and momenta
  for (const auto &key : keys) {
    disps.insert(key.disp);
    moms.insert(std::array<int, 3>{key.mom[0], key.mom[1], key.mom[2]});
  }

  // Open new DB
  Datalib::TensorStorage::StorageDisco dbnew;
  if (dbnew.create(new_db_name, latt_size, decay_dir, probing_distance,
                   probing_power, total_noise_vectors, mass_label,
                   std::vector<std::vector<int>>(disps.begin(), disps.end()),
                   std::vector<std::array<int, 3>>(moms.begin(), moms.end())) !=
      0) {
    std::cerr << __func__ << ": error opening dbase= " << new_db_name
              << std::endl;
    exit(1);
  }

  // Store statistics about the traces
  struct MesonKey {
    std::vector<int> disp;  // Displacement dirs of quark (right)
    std::array<int, 3> mom; // D-1 momentum of this operator
  };

  struct HashMesonKey {
    std::size_t operator()(const MesonKey &k) const {
      auto h = std::hash<int>{}(12345);
      for (const auto &it : k.disp)
        h = (h << 1) ^ std::hash<int>{}(it);
      for (const auto &it : k.mom)
        h = (h << 1) ^ std::hash<int>{}(it);
      return h;
    }
  };

  struct EqualToMesonKey {
    bool operator()(const MesonKey &a, const MesonKey &b) const {
      return a.disp == b.disp && a.mom == b.mom;
    }
  };

  struct Stats {
    std::array<double, 16> abs_trace;
    std::array<double, 16> var_trace;
    int count;
  };
  using MesonMap =
      std::unordered_map<MesonKey, Stats, HashMesonKey, EqualToMesonKey>;
  MesonMap stats(16);

  std::unordered_map<unsigned int, std::array<std::complex<double>, 16>>
      trace_bins_base(database.size() * 2);
  for (const auto &this_trace_index : trace_index)
    trace_bins_base[this_trace_index] =
        std::array<std::complex<double>, 16>{{}};

  // Loop over each key
  V avg_val;

  for (const auto &key : keys) {
    for (int j = 0; j < avg_val.op.size(); ++j) {
      avg_val.op[j] = 0;
    }

    // Average all values with the given weights
    auto trace_bins = trace_bins_base;
    for (int n = 0; n < database.size(); ++n) {
      // Find the key
      V val;
      if (database[n].get(key, val) != 0) {
        std::cerr << __func__ << ": key not found: key=" << key << std::endl;
        exit(1);
      }

      for (int j = 0; j < avg_val.op.size() && j < 16; ++j) {
        avg_val.op[j] +=
            val.op[j] * (double)noise_vectors[n] / (double)total_noise_vectors;
        trace_bins[trace_index[n]][j] +=
            val.op[j] *
            std::sqrt((double)noise_vectors[n] / (double)total_noise_vectors);
      }
    }

    std::array<double, 16> trace_var{{}};
    for (const auto &it : trace_bins) {
      for (int j = 0; j < 16; ++j) {
        trace_var[j] += std::norm(it.second[j]);
      }
    }

    // Insert the new key
    if (dbnew.insert(key, avg_val) != 0) {
      std::cerr << __func__
                << ": problem inserting a new key in dbase=" << new_db_name
                << std::endl;
      exit(1);
    }

    // Compute the absolute value of the traces
    std::array<double, 16> trace_abs{{}};
    for (int j = 0; j < avg_val.op.size() && j < 16; ++j)
      trace_abs[j] = std::abs(avg_val.op[j]);

    // Compute the variance as E[conj(x)*x] - conj(E[x])*E[x]
    for (int j = 0; j < avg_val.op.size() && j < 16; ++j)
      trace_var[j] = std::max(0.0, trace_var[j] - std::norm(avg_val.op[j]));

    // Insert the stats
    std::array<int, 3> mom{key.mom[0], key.mom[1], key.mom[2]};
    MesonKey stats_key{canonize(key.disp), canonize(mom)};
    if (stats.count(stats_key) == 0) {
      stats[stats_key] = {trace_abs, trace_var, 1};
    } else {
      auto &it = stats[stats_key];
      for (std::size_t j = 0; j < trace_abs.size(); ++j) {
        it.abs_trace[j] += trace_abs[j];
        it.var_trace[j] += trace_var[j];
      }
      it.count++;
    }
  }
  dbnew.close();

  for (auto &db : database)
    db.close();

  // Order the keys to print by momentum and displacement
  std::vector<MesonKey> stat_keys;
  stat_keys.reserve(stats.size());
  for (const auto &it : stats)
    stat_keys.push_back(it.first);
  std::sort(stat_keys.begin(), stat_keys.end(),
            [=](const MesonKey &a, const MesonKey &b) {
              auto a_mom = std::vector<int>(a.mom.begin(), a.mom.end());
              auto b_mom = std::vector<int>(b.mom.begin(), b.mom.end());
              return (a_mom < b_mom || (a_mom == b_mom &&  // compare mom
                                        a.disp < b.disp)); // compare disp
            });

  // Show stats
  for (const auto &key : stat_keys) {
    const auto &val = stats.at(key);
    const int n = val.count;
    std::cout << "DISCO VARIANCE with " << total_noise_vectors
              << " noise vectors key: disp = " << key.disp
              << " mom = " << key.mom << "   val: " << std::endl;
    for (int i = 0; i < 16; i++)
      std::cout << "Gamma[ " << i << " ]: avg = " << val.abs_trace[i] / n //
                << "   var = " << val.var_trace[i] / n                    //
                << "   relative_error = "
                << sqrt(val.var_trace[i] / n) / (val.abs_trace[i] / n)
                << std::endl;
  }
}

int main(int argc, char **argv) {
  // Parse the arguments
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0]
              << " <new dbasename> <first db> [<second db> ...]" << std::endl;
    return -1;
  }

  // Call the appropriate handler
  try {
    std::vector<std::string> db_names;
    for (int i = 2; i < argc; ++i)
      db_names.push_back(std::string(argv[i]));
    discoKeyMerger(db_names, std::string(argv[1]));
  } catch (const std::string &e) {
    std::cerr << argv[0] << ": error message: " << e << std::endl;
    exit(1);
  } catch (const std::runtime_error &e) {
    std::cerr << argv[0] << ": error message: " << e.what() << std::endl;
    exit(1);
  }

  return 0;
}

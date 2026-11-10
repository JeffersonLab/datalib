#include <complex>
#include <vector>
/*! \file
 * \brief Make synthetic Gaussian replicas of a single-configuration correlator
 * DB.
 */
#include <datalib/graph/hadron_sun_class_npart_npt_corr.h>
#include <datalib/legacy/hadron_1pt_corr.h>
#include <datalib/legacy/hadron_2pt_corr.h>
#include <datalib/legacy/hadron_3pt_corr.h>
#include <datalib/legacy/hadron_npart_npt_corr.h>
#include <datalib/storage/corr_reader.h>
#include <datalib/storage/dbtype.h>

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace {
  struct Options {
    std::string output, input;
    int samples;
    double factor;
    std::uint64_t seed;
  };

  std::uint64_t unsigned_integer(const std::string &text) {
    if (text.empty() ||
        text.find_first_not_of("0123456789") != std::string::npos)
      throw std::runtime_error("Expected an unsigned integer: " + text);
    std::size_t used;
    const auto value = std::stoull(text, &used);
    if (used != text.size())
      throw std::runtime_error("Invalid integer: " + text);
    return value;
  }

  template <class Key> void replicate(const Options &opt) {
    using Row = std::vector<std::complex<double>>;
    Datalib::Storage::StorageCorrFunc<Key> input;
    input.open_for_reading(opt.input);
    if (input.num_configs != 1)
      throw std::runtime_error("Input must contain exactly one configuration");
    auto keys = input.keys();
    if (keys.empty())
      throw std::runtime_error("Input contains no correlators");
    // Stable traversal makes a fixed seed independent of database key iteration
    // order.
    std::sort(keys.begin(), keys.end(), [](const Key &a, const Key &b) {
      std::ostringstream x, y;
      x << a;
      y << b;
      return x.str() < y.str();
    });
    int nt = -1;
    auto read = [&](const Key &key) {
      std::vector<Row> rows;
      if (input.get(key, rows) != 0 || rows.size() != 1 || rows[0].empty())
        throw std::runtime_error("Invalid single-configuration correlator");
      if (rows[0].size() >
          static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::runtime_error(
            "Correlator time extent exceeds storage limit");
      if (nt < 0)
        nt = static_cast<int>(rows[0].size());
      if (rows[0].size() != static_cast<std::size_t>(nt))
        throw std::runtime_error("Correlators have different time extents");
      for (const auto &c : rows[0]) {
        if (!std::isfinite(c.real()) || !std::isfinite(c.imag()) ||
            !std::isfinite(opt.factor * std::abs(c)))
          throw std::runtime_error("Non-finite input or Gaussian width");
      }
      return rows[0];
    };
    // Validate every input before creating output. Hold only one correlator at
    // a time.
    for (const auto &key : keys)
      read(key);
    Datalib::Storage::StorageCorrFunc<Key> output;
    output.open_for_writing(opt.output, opt.samples, input.metadata, nt);
    output.insert_keys(keys);
    std::mt19937_64 rng(opt.seed);
    std::normal_distribution<double> gaussian(0.0, 1.0);
    for (const auto &key : keys) {
      const Row original = read(key);
      std::vector<Row> replicas(opt.samples, Row(nt));
      for (auto &row : replicas) {
        for (int t = 0; t < nt; ++t) {
          const double sigma = opt.factor * std::abs(original[t]);
          if (sigma == 0.0)
            row[t] = original[t];
          else {
            // Draw in an explicit order, independent of argument evaluation
            // order.
            const double re = gaussian(rng), im = gaussian(rng);
            row[t] = original[t] + std::complex<double>(sigma * re, sigma * im);
            if (!std::isfinite(row[t].real()) || !std::isfinite(row[t].imag()))
              throw std::runtime_error("Non-finite synthetic sample");
          }
        }
      }
      if (output.insert(key, replicas) != 0)
        throw std::runtime_error("Error writing synthetic correlator");
    }
    output.close();
    std::cout << "SYNTHETIC replicas: keys=" << keys.size()
              << " samples=" << opt.samples << " NtCorr=" << nt
              << " factor=" << opt.factor << " seed=" << opt.seed
              << "\nsigma(Re noise) = sigma(Im noise) = factor * abs(C(t)); "
                 "independent across samples, keys, times and components.\n";
  }
} // namespace

int main(int argc, char **argv) {
  if (argc != 5 && argc != 6) {
    std::cerr
        << "Usage: " << argv[0]
        << " <output db> <single-config input db> <samples> <factor> [seed]\n"
        << "factor: finite nonnegative relative Gaussian standard deviation, "
           "not a bound.\n"
        << "Default seed: 5489. Output uses the same storage format as "
           "dbmerge.\n"
        << "Synthetic replicas are for software tests, not independent gauge "
           "configurations.\n";
    return 1;
  }
  try {
    const auto n = unsigned_integer(argv[3]);
    if (n == 0 ||
        n > static_cast<std::uint64_t>(std::numeric_limits<int>::max()))
      throw std::runtime_error("Samples must be between 1 and INT_MAX");
    std::size_t used;
    const std::string width(argv[4]);
    const double factor = std::stod(width, &used);
    if (used != width.size() || !std::isfinite(factor) || factor < 0)
      throw std::runtime_error("Factor must be finite and nonnegative");
    Options opt{argv[1], argv[2], static_cast<int>(n), factor,
                argc == 6 ? unsigned_integer(argv[5]) : 5489};
    if (std::filesystem::exists(std::filesystem::symlink_status(opt.output)))
      throw std::runtime_error("Refusing to overwrite existing output: " +
                               opt.output);
    const auto type = Hadron::getDBType(opt.input);
    const std::unordered_map<std::string, std::function<void(const Options &)>>
        handlers{
            {"hadron1Pt", replicate<FF::KeyHadron1PtCorr_t>},
            {"hadron2Pt", replicate<FF::KeyHadron2PtCorr_t>},
            {"hadron3Pt", replicate<FF::KeyHadron3PtCorr_t>},
            {"hadronNPartNPtCorr", replicate<Hadron::KeyHadronNPartNPtCorr_t>},
            {"hadronSUNNPartNPtCorr",
             replicate<Hadron::KeyHadronSUNClassNPartNPtCorr_t>},
            {"hadronSUNClassNPartNPtCorr",
             replicate<Hadron::KeyHadronSUNClassNPartNPtCorr_t>}};
    if (handlers.count(type) == 0)
      throw std::runtime_error("Unsupported correlator type: " + type);
    handlers.at(type)(opt);
  } catch (const std::string &e) {
    std::cerr << argv[0] << ": " << e << '\n';
    return 1;
  } catch (const std::exception &e) {
    std::cerr << argv[0] << ": " << e.what() << '\n';
    return 1;
  }
  return 0;
}

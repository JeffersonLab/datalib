/*! \file
 * \brief Generate a gaussian distributed set of correlators into an ensembl
 */

#include "AllConfStoreDB.h"
#include "ConfDataStoreDB.h"
#include "DBString.h"
#include <datalib/graph/naming.h>
#include <datalib/legacy/hadron_sun_npart_npt_corr.h>
#include <datalib/storage/filedb.h>

using namespace Hadron;

#include <cmath>
#include <cstdlib>
#include <limits>

namespace {
  double generateGaussianNoise(double mu, double sigma) {
    static const double epsilon = std::numeric_limits<double>::min();
    static const double two_pi = 2.0 * 3.14159265358979323846;

    thread_local double z1;
    thread_local bool generate;
    generate = !generate;

    if (!generate)
      return z1 * sigma + mu;

    double u1, u2;
    do {
      u1 = rand() * (1.0 / RAND_MAX);
      u2 = rand() * (1.0 / RAND_MAX);
    } while (u1 <= epsilon);

    double z0;
    z0 = sqrt(-2.0 * log(u1)) * cos(two_pi * u2);
    z1 = sqrt(-2.0 * log(u1)) * sin(two_pi * u2);
    return z0 * sigma + mu;
  }
} // namespace

int main(int argc, char **argv) {
  // Parse the arguments
  if (argc != 5) {
    std::cerr << "Usage: " << argv[0]
              << " <new dbasename> <old db> <nbins> <# sigma for width>"
              << std::endl;
    return -1;
  }

  try {
    // Get bored typing this so much
    typedef Hadron::KeyHadronSUNNPartNPtCorr_t K;
    typedef std::vector<std::complex<double>> SV;

    // Old db
    FILEDB::ConfDataStoreDB<Datalib::Storage::SerialDBKey<K>,
                            Datalib::Storage::SerialDBData<SV>>
        dbold;
    if (dbold.open(argv[2], O_RDONLY, 0400) != 0) {
      std::cerr << __func__ << ": error opening dbase= " << argv[2]
                << std::endl;
      exit(1);
    }

    std::string meta;
    dbold.getUserdata(meta);
    std::cout << "meta= " << meta << std::endl;

    // Get bins and widths
    int nbins = std::stoi(argv[3]);
    if (nbins < 1) {
      std::cerr << __func__ << ": error: nbins= " << nbins << std::endl;
      exit(1);
    }
    std::vector<int> cfgs(nbins);

    for (int i = 0; i < nbins; ++i) {
      cfgs[i] = i;
    }

    // Width
    double width = std::stof(argv[4]);
    std::cout << "width= " << width << std::endl;

    // New db
    std::cout << "line= " << __LINE__ << std::endl;
    FILEDB::AllConfStoreDB<Datalib::Storage::SerialDBKey<K>,
                           Datalib::Storage::SerialDBData<SV>>
        dbnew(cfgs);
    std::cout << "line= " << __LINE__ << std::endl;

    // Setup for the new DB
    std::cout << "line= " << __LINE__ << std::endl;
    dbnew.setMaxUserInfoLen(meta.size());
    std::cout << "line= " << __LINE__ << std::endl;

    // Open it
    if (dbnew.open(argv[1], O_RDWR | O_TRUNC | O_CREAT, 0664) != 0) {
      std::cerr << __func__ << ": error opening dbase= " << argv[1]
                << std::endl;
      exit(1);
    }

    std::cout << "line= " << __LINE__ << std::endl;
    // Insert new DB-meta
    std::cout << "line= " << __LINE__ << std::endl;
    dbnew.insertUserdata(meta);
    std::cout << "line= " << __LINE__ << std::endl;

    // Get keys
    std::vector<Datalib::Storage::SerialDBKey<K>> keys;
    dbold.keys(keys);
    std::cout << "keys.num= " << keys.size() << std::endl;

    for (auto key = keys.begin(); key != keys.end(); ++key) {
      // Get the old value
      Datalib::Storage::SerialDBData<SV> orig_val;

      if (dbold.get(*key, orig_val) != 0) {
        std::cerr << __func__ << ": key not found\n";
        exit(1);
      }

      // New value
      int Lt = orig_val.data().size();
      std::cout << __func__ << ": Lt= " << Lt << "\nkey: " << key->key()
                << std::endl;
      std::vector<Datalib::Storage::SerialDBData<SV>> val(nbins);

      std::cout << "line= " << __LINE__ << std::endl;
      for (int n = 0; n < nbins; ++n) {
        std::cout << "line= " << __LINE__ << std::endl;
        val[n].data().resize(Lt);
        std::cout << "line= " << __LINE__ << std::endl;

        for (int t = 0; t < Lt; ++t) {
          std::cout << "line= " << __LINE__ << std::endl;
          double mu = std::real(orig_val.data()[t]);
          double rree = generateGaussianNoise(mu, width);
          val[n].data()[t] = std::complex<double>(rree, 0.0);
          std::cout << "line= " << __LINE__ << "  mu= " << mu
                    << "   re= " << rree << std::endl;
        }
      }

      // Insert the new key
      std::cout << "line= " << __LINE__ << "\nkey: " << key->key() << std::endl;
      if (dbnew.insert(*key, val) != 0) {
        std::cerr << __func__ << ": error inserting key\n";
        exit(1);
      }
      std::cout << "line= " << __LINE__ << std::endl;
    }

    // close it
    dbnew.close();
  } catch (const std::string &e) {
    std::cerr << argv[0] << ": error message: " << e << std::endl;
    exit(1);
  }

  return 0;
}

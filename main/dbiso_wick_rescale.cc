#include "dbtools_support.h"
#include <complex>
#include <vector>
/*! \file
 * \brief Rescale correlation functions, rescaling flavor weights and correcting
 * for normalization in Wick contractions
 */

#include "AllConfStoreDB.h"
#include "DBString.h"
#include <datalib/graph/naming.h>
#include <datalib/graph/operator_name.h>
#include <datalib/legacy/hadron_npart_npt_corr.h>
#include <datalib/legacy/hadron_sun_npart_npt_corr.h>
#include <datalib/storage/filedb.h>

using namespace Hadron;

int main(int argc, char **argv) {
  // Parse the arguments
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <new dbasename> <old db>"
              << std::endl;
    return -1;
  }

  try {
    // Get bored typing this so much
    typedef Hadron::KeyHadronNPartNPtCorr_t K;

    // Old db
    FILEDB::AllConfStoreDB<
        Datalib::Storage::SerialDBKey<K>,
        Datalib::Storage::SerialDBData<std::vector<std::complex<double>>>>
        dbold;
    if (dbold.open(argv[2], O_RDONLY, 0400) != 0) {
      std::cerr << __func__ << ": error opening dbase= " << argv[2]
                << std::endl;
      exit(1);
    }

    std::string meta;
    dbold.getUserdata(meta);

    // Open DB
    int nbins = dbold.size();
    std::vector<int> cfgs(nbins);

    for (int i = 0; i < nbins; ++i) {
      cfgs[i] = i;
    }

    // New db
    FILEDB::AllConfStoreDB<
        Datalib::Storage::SerialDBKey<K>,
        Datalib::Storage::SerialDBData<std::vector<std::complex<double>>>>
        dbnew(cfgs);

    // Setup for the new DB
    dbnew.setMaxUserInfoLen(meta.size());

    // Open it
    if (dbnew.open(argv[1], O_RDWR | O_TRUNC | O_CREAT, 0664) != 0) {
      std::cerr << __func__ << ": error opening dbase= " << argv[1]
                << std::endl;
      exit(1);
    }

    // Insert new DB-meta
    dbnew.insertUserdata(meta);

    // Get keys and rescale by a factor determined by the total number of
    // quarks.
    std::vector<Datalib::Storage::SerialDBKey<K>> keys;
    dbold.keys(keys);

    for (int i = 0; i < keys.size(); ++i) {
      // Get the old value
      std::vector<
          Datalib::Storage::SerialDBData<std::vector<std::complex<double>>>>
          val;
      int ret;

      if ((ret = dbold.get(keys[i], val)) != 0) {
        std::cerr << __func__ << ": key not found\n";
        exit(1);
      }

      // Loop over the ops and find the number of quarks and build up rephase
      // factors
      double rescale_factor = 1.0;
      double proj_op_factor = 1.0;
      int num_quarks = 0;

      for (std::vector<Hadron::KeyHadronNPartNPtCorr_t::NPoint_t>::iterator
               npt = keys[i].key().npoint.ref().begin();
           npt != keys[i].key().npoint.ref().end(); ++npt) {
        for (std::vector<KeyParticleOp_t>::iterator op =
                 npt->irrep.op.ops.ref().begin();
             op != npt->irrep.op.ops.ref().end(); ++op) {
          num_quarks += Hadron::getNumQuarks(op->name);
          rescale_factor *= Hadron::getRescalingFactor(op->name);
          proj_op_factor *= Hadron::getProjOpRescalingFactor(op->name);
        }
      }

      // The rescaling factor
      int num_quarks2 = num_quarks >> 1;
      int N2m1 = num_quarks2 - 1;
      double f = Datalib::Tools::wickDenominator(N2m1);
      double ff(proj_op_factor * rescale_factor / f);

      // std::cout << "ff= " << ff << "  proj_op_factor= " << proj_op_factor <<
      // "  rescale_factor= " << rescale_factor << "  num_quarks= " <<
      // num_quarks << "  f= " << f << "  key: " << keys[i].key() << "\n";

      // Rescale the correlators
      for (std::vector<Datalib::Storage::SerialDBData<
               std::vector<std::complex<double>>>>::iterator v = val.begin();
           v != val.end(); ++v) {
        Datalib::Tools::scale(v->data(), ff);
      }

      // Insert the new key
      if ((ret = dbnew.insert(Datalib::Storage::SerialDBKey<K>(keys[i].key()),
                              val)) != 0) {
        std::cerr << __func__ << ": error inserting key\n";
        exit(1);
      }
    }

    // close it
    dbnew.close();
  } catch (const std::string &e) {
    std::cerr << argv[0] << ": error message: " << e << std::endl;
    exit(1);
  }

  return 0;
}

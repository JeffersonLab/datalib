#include <complex>
#include <vector>
/*! \file
 * \brief Rephase correlation functions
 */
#include <datalib/storage/corr_reader.h>

#include "AllConfStoreDB.h"
#include "DBString.h"
#include <datalib/graph/naming.h>
#include <datalib/graph/operator_name.h>
#include <datalib/legacy/hadron_sun_npart_npt_corr.h>
#include <datalib/storage/filedb.h>
#include <unordered_map>

using namespace Hadron;

namespace {

  void insertPhase(
      std::unordered_map<std::string, std::complex<double>> &the_phase_map,
      std::string left, std::string right, std::string target,
      std::complex<double> val) {
    the_phase_map.insert(
        {{std::string(left + "x" + right + "->" + target), val}});
  }

  void insertPhaseBothOrders(
      std::unordered_map<std::string, std::complex<double>> &the_phase_map,
      std::string left, std::string right, std::string target,
      std::complex<double> val) {
    insertPhase(the_phase_map, left, right, target, val);
    insertPhase(the_phase_map, right, left, target, val);
  }

  void insertOpPhase(
      std::unordered_map<std::string, std::complex<double>> &the_phase_map,
      std::string opname, std::complex<double> val) {
    the_phase_map.insert({{opname, val}});
  }

  //! Get a key/value
  template <typename K>
  std::vector<std::vector<std::complex<double>>>
  get_value(const K &key, Datalib::Storage::StorageCorrFunc<K> &database) {
    std::vector<std::vector<std::complex<double>>> val;
    if (database.get(key, val) != 0) {
      std::stringstream ss;
      ss << "key not found: " << key;
      throw std::runtime_error(ss.str());
    }

    return val;
  }

} // namespace

int main(int argc, char **argv) {
  // Parse the arguments
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <new dbasename> <old dbasename>"
              << std::endl;
    return -1;
  }

  try {
    // Setup the phase map
    std::unordered_map<std::string, std::complex<double>> phase_map;
    std::unordered_map<std::string, std::complex<double>> op_phase_map;
    std::complex<double> complex_0(0.0, 0.0);
    std::complex<double> complex_1(1.0, 0.0);
    std::complex<double> complex_i(0.0, 1.0);

    // Phases for CGs
    // N.B. This should be the same order as in filetest_redstar
    // insertPhaseBothOrders("D4A1","D2A1","D4A1,1",complex_1);
    // insertPhase(phase_map,"D4E2","D4A2","D2A1,1",complex_i);
    // insertPhase(phase_map,"D4E2","D4A2","D2A2,1",complex_i);
    // insertPhase(phase_map,"D4E2","D2A2","D2A2,1",complex_i);
    // insertPhase(phase_map,"D4E2","D4A2","D2B1,1",complex_i);
    // insertPhase(phase_map,"D4B2","D4A2","D2B1,1",complex_i);
    // insertPhase(phase_map,"D4E2","D4A2","D2B2,1",complex_i);
    // insertPhase(phase_map,"D4B1","D4A2","D2B2,1",complex_i);

    // Phases for projected ops
    // insertOpPhase(op_phase_map,"chice_proj1_p100_H1D4E2",complex_i);

    // Get bored typing this so much
    typedef Hadron::KeyHadronSUNClassNPartNPtCorr_t K;

    // Open DB
    Datalib::Storage::StorageCorrFunc<K> db_in;
    db_in.open_for_reading(argv[2]);

    std::string meta = db_in.metadata;
    meta += "\n<!-- Rephased using db_rephase_corrs -->\n";

    // New db
    Datalib::Storage::StorageCorrFunc<K> db_out;
    db_out.open_for_writing(argv[1], db_in.num_configs, db_in.metadata,
                            db_in.NtCorr, true /* use new format */);

    const auto &keys = db_in.keys();
    db_out.insert_keys(keys);
    for (const auto &key : keys) {
      auto val = get_value(key, db_in);

      // The rephasing factor
      std::complex<double> ff(1.0);

      // Loop over the ops
      std::cout << "key= " << std::endl;
      for (auto npt = key.npoint.ref().begin(); npt != key.npoint.ref().end();
           ++npt) {
        std::cout << "   ";
        int num_CGs = npt->irrep.op.CGs.size();
        std::cout << "creation_op= " << npt->irrep.creation_op
                  << ", num_CGs= " << num_CGs << ": " << std::endl;

        // Phases for projected operators
        for (int iop = 1; iop <= npt->irrep.op.ops.size(); iop++) {
          std::unordered_map<std::string, std::complex<double>>::const_iterator
              map_itt = op_phase_map.find(npt->irrep.op.ops[iop].name);
          if (map_itt != op_phase_map.end()) {
            std::complex<double> opphase = map_itt->second;
            if (npt->irrep.creation_op) {
              std::cout << "      rephasing op= " << npt->irrep.op.ops[iop].name
                        << " by phase= " << opphase << std::endl;
            } else {
              opphase = conj(opphase);
              std::cout << "      rephasing op= " << npt->irrep.op.ops[iop].name
                        << " by conj(phase)= " << opphase << std::endl;
            }
            ff = ff * opphase;
          }
        }

        // Phases for CGs
        if (num_CGs == 1) {
          std::string rep1 = Hadron::removeIrrepGParity(Hadron::removeHelicity(
              Hadron::getSingleHadronOpIrrep(npt->irrep.op.ops[1].name)));
          std::string rep2 = Hadron::removeIrrepGParity(Hadron::removeHelicity(
              Hadron::getSingleHadronOpIrrep(npt->irrep.op.ops[2].name)));
          std::string target = npt->irrep.op.CGs[1].target.irrep;
          std::size_t pos = target.find(",");
          std::string rept = Hadron::removeIrrepGParity(target.substr(0, pos));
          std::string embed = target.substr(pos + 1);
          std::complex<double> phase(1.0, 0.0);
          std::string CGCstring =
              std::string(rep1 + "x" + rep2 + "->" + rept + "," + embed);
          std::cout << "      " << CGCstring;

          std::unordered_map<std::string, std::complex<double>>::const_iterator
              map_itt = phase_map.find(CGCstring);
          if (map_itt != phase_map.end()) {
            phase = map_itt->second;
            if (npt->irrep.creation_op)
              std::cout << "  rephasing by phase= " << phase << std::endl;
            else
              std::cout << "  rephasing by conj(phase)= " << conj(phase)
                        << std::endl;
          } else {
            std::cout << "  no rephasing phase" << std::endl;
          }
          if (!npt->irrep.creation_op)
            phase = conj(phase);
          ff = ff * phase;
        }
      }

      // Rephase the correlators
      for (std::vector<std::vector<std::complex<double>>>::iterator v =
               val.begin();
           v != val.end(); ++v) {
        for (std::vector<std::complex<double>>::iterator vv = v->begin();
             vv != v->end(); ++vv) {
          *vv = *vv * ff;
        }
      }

      db_out.insert(key, val);
    }

    // close it
    db_out.close();
  } catch (const std::string &e) {
    std::cerr << argv[0] << ": error message: " << e << std::endl;
    exit(1);
  }

  return 0;
}

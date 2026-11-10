#include "dbtools_support.h"
#include <complex>
#include <vector>
/*! \file
 * \brief Transform matrix of correlation functions by taking linear
 * combinations of operators
 */

#include "AllConfStoreDB.h"
#include "DBString.h"
#include <datalib/graph/naming.h>
#include <datalib/graph/operator_name.h>
#include <datalib/legacy/hadron_sun_npart_npt_corr.h>
#include <datalib/storage/filedb.h>
#include <unordered_map>

using namespace Hadron;

namespace {

  struct lin_combin_t {
    lin_combin_t() : combin(""), val(0.0) {}
    lin_combin_t(std::string combin_, std::complex<double> val_)
        : combin(combin_), val(val_) {}

    std::string combin;
    std::complex<double> val;
  };

  typedef std::unordered_map<std::string, std::vector<lin_combin_t>>
      lin_combin_map_t;

  // Functions to insert into map for convenience
  void insertLinCombin2(lin_combin_map_t &the_lin_combin_map,
                        std::string target, std::string first_str,
                        std::complex<double> first_val, std::string second_str,
                        std::complex<double> second_val) {
    std::vector<lin_combin_t> lin_combin;
    lin_combin.push_back(lin_combin_t(first_str, first_val));
    lin_combin.push_back(lin_combin_t(second_str, second_val));
    the_lin_combin_map.insert({{target, lin_combin}});
  }

  void insertLinCombin3(lin_combin_map_t &the_lin_combin_map,
                        std::string target, std::string first_str,
                        std::complex<double> first_val, std::string second_str,
                        std::complex<double> second_val, std::string third_str,
                        std::complex<double> third_val) {
    std::vector<lin_combin_t> lin_combin;
    lin_combin.push_back(lin_combin_t(first_str, first_val));
    lin_combin.push_back(lin_combin_t(second_str, second_val));
    lin_combin.push_back(lin_combin_t(third_str, third_val));
    the_lin_combin_map.insert({{target, lin_combin}});
  }

  // Get a key/value
  template <typename K, typename SV>
  std::vector<Datalib::Storage::SerialDBData<SV>> printKeyValueVec(
      const K &ky,
      FILEDB::AllConfStoreDB<Datalib::Storage::SerialDBKey<K>,
                             Datalib::Storage::SerialDBData<SV>> &database) {
    Datalib::Storage::SerialDBKey<K> key;
    key.key() = ky;

    std::vector<Datalib::Storage::SerialDBData<SV>> vals;
    int ret;
    if ((ret = database.get(key, vals)) != 0) {
      std::cerr << __func__ << ": key not found\n" << ky;
      exit(1);
    }

    return vals;
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
    // Some definitions for convenience
    std::complex<double> complex_0(0.0, 0.0);
    std::complex<double> complex_1(1.0, 0.0);
    std::complex<double> complex_i(0.0, 1.0);
    typedef std::complex<double> cplx;

    // Get bored typing this so much
    typedef Hadron::KeyHadronSUNNPartNPtCorr_t K;
    typedef Datalib::Storage::SerialDBData<std::vector<std::complex<double>>>
        SV;

    // Setup the map
    lin_combin_map_t lin_combin_map;

    // Linear combinations of ops
    // (coefficients are for creation ops, they are conjugated for annihilation
    // ops)

    // insertLinCombin2(lin_combin_map, "J1o2_H1o2D2ExJ0_H0D2A2->Hg,1",
    //		     "Hg,1", complex_1, "Hg,2", complex_0);
    // insertLinCombin2(lin_combin_map, "J1o2_H1o2D2ExJ0_H0D2A2->Hg,2",
    //		     "Hg,1", complex_0, "Hg,2", complex_1);

    // Nucleon meson

    insertLinCombin2(lin_combin_map, "J1o2_H1o2D2ExJ0_H0D2A2->Hg,1", "Hg,1",
                     cplx(-1.98848, -4.80061), "Hg,2", cplx(4.39523, -5.06774));
    insertLinCombin2(lin_combin_map, "J1o2_H1o2D2ExJ0_H0D2A2->Hg,2", "Hg,1",
                     cplx(-5.93172, -1.17344), "Hg,2", cplx(2.37666, 3.20763));

    insertLinCombin2(lin_combin_map, "J1o2_H1o2D2ExJ0_H0D2A2->Hu,1", "Hu,1",
                     cplx(1.98848, -4.80061), "Hu,2", cplx(4.39523, 5.06774));
    insertLinCombin2(lin_combin_map, "J1o2_H1o2D2ExJ0_H0D2A2->Hu,2", "Hu,1",
                     cplx(5.93172, -1.17344), "Hu,2", cplx(2.37666, 3.20763));

    // Nucleon nucleon

    insertLinCombin2(lin_combin_map, "J1o2_H1o2D4E1xJ1o2_H1o2D4E1->T1p,1",
                     "T1p,1", 0.0, "T1p,2", 4.0);
    insertLinCombin2(lin_combin_map, "J1o2_H1o2D4E1xJ1o2_H1o2D4E1->T1p,2",
                     "T1p,1", -8.0, "T1p,2", 0.0);

    insertLinCombin3(lin_combin_map, "J1o2_H1o2D2ExJ1o2_H1o2D2E->T1p,1",
                     "T1p,1", cplx(8.0, 0.0), "T1p,2", cplx(0.0, 5.65685),
                     "T1p,3", cplx(0.0, -5.19615));
    insertLinCombin3(lin_combin_map, "J1o2_H1o2D2ExJ1o2_H1o2D2E->T1p,2",
                     "T1p,1", cplx(0.0, 5.65685), "T1p,2", cplx(4.0, 0.0),
                     "T1p,3", cplx(-3.67423, 0.0));
    insertLinCombin3(lin_combin_map, "J1o2_H1o2D2ExJ1o2_H1o2D2E->T1p,3",
                     "T1p,1", cplx(0.0, 0.0), "T1p,2", cplx(0.0, -8.0), "T1p,3",
                     cplx(0.0, -4.89898));

    insertLinCombin2(lin_combin_map, "J1o2_H1o2D3E1xJ1o2_H1o2D3E1->T1p,1",
                     "T1p,1", cplx(0.0, 0.0), "T1p,2", cplx(6.53168, 6.53168));
    insertLinCombin2(lin_combin_map, "J1o2_H1o2D3E1xJ1o2_H1o2D3E1->T1p,2",
                     "T1p,1", cplx(13.0651, 13.0651), "T1p,2", cplx(0.0, 0.0));

#if 0
    // Old coefficients considering LS basis

    // Nucleon meson
    
    insertLinCombin2(lin_combin_map, "J1o2_H1o2D2ExJ0_H0D2A2->Hg,1",
    		     "Hg,1", cplx(-2.81214, -6.7891), "Hg,2", cplx(6.21579, -7.16687) );
    insertLinCombin2(lin_combin_map, "J1o2_H1o2D2ExJ0_H0D2A2->Hg,2",
    		     "Hg,1", cplx(-11.8635, -2.34689), "Hg,2", cplx(4.75334, 6.41528) );

    insertLinCombin2(lin_combin_map, "J1o2_H1o2D2ExJ0_H0D2A2->Hu,1",
    		     "Hu,1", cplx(-2.81214, 6.7891), "Hu,2", cplx(-6.21579, -7.16687) );
    insertLinCombin2(lin_combin_map, "J1o2_H1o2D2ExJ0_H0D2A2->Hu,2",
    		     "Hu,1", cplx(-7.50312, -1.48431), "Hu,2", cplx(-0.582413, 8.19517) );


    // Nucleon nucleon

    insertLinCombin2(lin_combin_map, "J1o2_H1o2D4E1xJ1o2_H1o2D4E1->T1p,1",
    		     "T1p,1", -8.00001, "T1p,2", 2.82843);
    insertLinCombin2(lin_combin_map, "J1o2_H1o2D4E1xJ1o2_H1o2D4E1->T1p,2",
    		     "T1p,1", -5.65684, "T1p,2", -3.99999);

    insertLinCombin3(lin_combin_map, "J1o2_H1o2D2ExJ1o2_H1o2D2E->T1p,1",
    		     "T1p,1", cplx(5.65685, 5.65685), "T1p,2", cplx(4., 4.), "T1p,3", cplx(0.0, -7.99999) );
    insertLinCombin3(lin_combin_map, "J1o2_H1o2D2ExJ1o2_H1o2D2E->T1p,2",
    		     "T1p,1", cplx(-8., 4.), "T1p,2", cplx(2.82843, -5.65685), "T1p,3", cplx(0.0, -5.65685) );
    insertLinCombin3(lin_combin_map, "J1o2_H1o2D2ExJ1o2_H1o2D2E->T1p,3",
    		     "T1p,1", cplx(2.44949, -7.34847), "T1p,2", cplx(-5.19616, 1.73205), "T1p,3", cplx(0.0, -6.92821) );

    insertLinCombin2(lin_combin_map, "J1o2_H1o2D3E1xJ1o2_H1o2D3E1->T1p,1",
    		     "T1p,1", cplx(13.0639, 13.0639), "T1p,2", cplx(4.61881, 4.61881) );
    insertLinCombin2(lin_combin_map, "J1o2_H1o2D3E1xJ1o2_H1o2D3E1->T1p,2",
    		     "T1p,1", cplx(9.23761, 9.23761), "T1p,2", cplx(-6.53197, -6.53197) );
#endif

    // Old db
    FILEDB::AllConfStoreDB<Datalib::Storage::SerialDBKey<K>, SV> dbold;
    if (dbold.open(argv[2], O_RDONLY, 0400) != 0) {
      std::cerr << __func__ << ": error opening dbase= " << argv[2]
                << std::endl;
      exit(1);
    }

    std::string meta;
    dbold.getUserdata(meta);
    meta += "\n<!-- Transformed matrix of correlation functions using "
            "db_transform_corrs -->\n";

    int nbins = dbold.size();
    std::vector<int> cfgs(nbins);

    for (int i = 0; i < nbins; ++i) {
      cfgs[i] = i;
    }

    // New db
    FILEDB::AllConfStoreDB<Datalib::Storage::SerialDBKey<K>, SV> dbnew(cfgs);

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

    // Get keys and loop over
    std::vector<Datalib::Storage::SerialDBKey<K>> keys;
    dbold.keys(keys);

    for (int i = 0; i < keys.size(); ++i) {
      // Get the old value
      K original_key = keys[i].key();
      std::vector<SV> val =
          printKeyValueVec<K, std::vector<std::complex<double>>>(original_key,
                                                                 dbold);

      // Loop over the ops
      std::cout << "key= " << i << " (npts=" << original_key.npoint.size()
                << "): " << std::endl;

      if (original_key.npoint.size() != 2) // Only support 2pts
      {
        std::cerr << "ERROR: only support 2-point correlation functions"
                  << std::endl;
        exit(1);
      }

      std::vector<bool> lin_combin_needed;
      std::vector<std::string> CGCstrings;

      for (std::vector<K::NPoint_t>::iterator npt =
               original_key.npoint.ref().begin();
           npt != original_key.npoint.ref().end(); ++npt) {
        std::cout << "   ";
        int num_CGs = npt->irrep.op.CGs.size();
        std::cout << "npt["
                  << (std::distance(original_key.npoint.ref().begin(), npt) + 1)
                  << "]: "
                  << "creation_op= " << npt->irrep.creation_op
                  << ", num_CGs= " << num_CGs << ": " << std::endl;

        // For now only implemented for two-hadron ops
        if (num_CGs == 1) {
          std::string rep1 = Hadron::removeHelicity(
              Hadron::getSingleHadronOpSpinIrrep(npt->irrep.op.ops[1].name));
          std::string rep2 = Hadron::removeHelicity(
              Hadron::getSingleHadronOpSpinIrrep(npt->irrep.op.ops[2].name));
          std::string target = npt->irrep.op.CGs[1].target.irrep;
          std::size_t pos = target.find(",");
          std::string rept = target.substr(0, pos);
          std::string embed = target.substr(pos + 1);
          std::complex<double> phase(1.0, 0.0);
          std::string CGCstring =
              std::string(rep1 + "x" + rep2 + "->" + rept + "," + embed);
          std::cout << "      (" << npt->irrep.op.ops[1].name << " x "
                    << npt->irrep.op.ops[2].name << ") " << CGCstring;

          lin_combin_map_t::const_iterator map_itt =
              lin_combin_map.find(CGCstring);
          if (map_itt != lin_combin_map.end()) {
            std::cout << "  LIN COMBIN NEEDED" << std::endl;
            lin_combin_needed.push_back(true);
            CGCstrings.push_back(CGCstring);
          } else {
            // std::cout << "  no lin combin needed";
            std::cout << std::endl;
            lin_combin_needed.push_back(false);
            CGCstrings.push_back("");
          }

        } else {
          // num_CGs != 1
          lin_combin_needed.push_back(false);
          CGCstrings.push_back("");
        }

      } // loop over npts

      // Sanity checks
      if (lin_combin_needed.size() != 2) {
        std::cerr << "ERROR: lin_combin_needed.size()= "
                  << lin_combin_needed.size() << " but it should be 2"
                  << std::endl;
        exit(1);
      }
      if (CGCstrings.size() != 2) {
        std::cerr << "ERROR: CGCstrings.size()= " << CGCstrings.size()
                  << " but it should be 2" << std::endl;
        exit(1);
      }

      // Take linear combination if needed
      std::vector<SV> new_val;
      if (lin_combin_needed[0] &&
          lin_combin_needed[1]) // Need linear combination for npt[1] and npt[2]
      {
        std::cout << "   Taking linear combination for npt[1] and npt[2]:"
                  << std::endl;
        lin_combin_map_t::const_iterator map_itt1 =
            lin_combin_map.find(CGCstrings[0]);
        lin_combin_map_t::const_iterator map_itt2 =
            lin_combin_map.find(CGCstrings[1]);
        std::vector<lin_combin_t> lin_combin_vec1 = map_itt1->second;
        std::vector<lin_combin_t> lin_combin_vec2 = map_itt2->second;

        for (std::vector<lin_combin_t>::iterator lin_combin_itt1 =
                 lin_combin_vec1.begin();
             lin_combin_itt1 != lin_combin_vec1.end(); ++lin_combin_itt1) {
          K new_key = original_key;
          std::complex<double> coeff1 = (*lin_combin_itt1).val;
          if (!original_key.npoint[1].irrep.creation_op)
            coeff1 = std::conj(coeff1); // Take conj if annihilation op
          new_key.npoint[1].irrep.op.CGs[1].target.irrep =
              (*lin_combin_itt1).combin;

          for (std::vector<lin_combin_t>::iterator lin_combin_itt2 =
                   lin_combin_vec2.begin();
               lin_combin_itt2 != lin_combin_vec2.end(); ++lin_combin_itt2) {
            std::complex<double> coeff2 = (*lin_combin_itt2).val;
            if (!original_key.npoint[2].irrep.creation_op)
              coeff2 = std::conj(coeff2); // Take conj if annihilation op
            new_key.npoint[2].irrep.op.CGs[1].target.irrep =
                (*lin_combin_itt2).combin;

            std::cout << "      npt[1] " << (*lin_combin_itt1).combin
                      << " coeff= " << coeff1 << ", npt[2] "
                      << (*lin_combin_itt2).combin << " coeff= " << coeff2
                      << ", overall coeff = " << (coeff1 * coeff2) << std::endl;

            // Build up the linear combination with appropriate coefficient
            if ((lin_combin_itt1 == lin_combin_vec1.begin()) &&
                (lin_combin_itt2 == lin_combin_vec2.begin())) {
              std::vector<SV> temp_val =
                  printKeyValueVec<K, std::vector<std::complex<double>>>(
                      new_key, dbold);
              for (int i = 0; i < temp_val.size(); i++) {
                SV temp_sv = temp_val[i];
                Datalib::Tools::scale(temp_sv.data(), coeff1 * coeff2);
                new_val.push_back(temp_sv);
              }
            } else {
              std::vector<SV> temp_val =
                  printKeyValueVec<K, std::vector<std::complex<double>>>(
                      new_key, dbold);
              for (int i = 0; i < temp_val.size(); i++) {
                SV temp_sv = temp_val[i];
                Datalib::Tools::scale(temp_sv.data(), coeff1 * coeff2);
                Datalib::Tools::addScaled(new_val[i].data(), temp_sv.data(),
                                          1.0);
              }
            }
          } // lin_combin_itt2
        } // lin_combin_itt1

      } else if (lin_combin_needed[0]) // Need linear combination for npt[1]
      {
        std::cout << "   Taking linear combination for npt[1]:" << std::endl;
        lin_combin_map_t::const_iterator map_itt1 =
            lin_combin_map.find(CGCstrings[0]);
        std::vector<lin_combin_t> lin_combin_vec1 = map_itt1->second;

        for (std::vector<lin_combin_t>::iterator lin_combin_itt1 =
                 lin_combin_vec1.begin();
             lin_combin_itt1 != lin_combin_vec1.end(); ++lin_combin_itt1) {
          K new_key = original_key;
          std::complex<double> coeff1 = (*lin_combin_itt1).val;
          if (!original_key.npoint[1].irrep.creation_op)
            coeff1 = std::conj(coeff1); // Take conj if annihilation op
          new_key.npoint[1].irrep.op.CGs[1].target.irrep =
              (*lin_combin_itt1).combin;

          std::cout << "      npt[1] " << (*lin_combin_itt1).combin
                    << " coeff= " << coeff1 << std::endl;

          // Build up the linear combination with appropriate coefficient
          if (lin_combin_itt1 == lin_combin_vec1.begin()) {
            std::vector<SV> temp_val =
                printKeyValueVec<K, std::vector<std::complex<double>>>(new_key,
                                                                       dbold);
            for (int i = 0; i < temp_val.size(); i++) {
              SV temp_sv = temp_val[i];
              Datalib::Tools::scale(temp_sv.data(), coeff1);
              new_val.push_back(temp_sv);
            }
          } else {
            std::vector<SV> temp_val =
                printKeyValueVec<K, std::vector<std::complex<double>>>(new_key,
                                                                       dbold);
            for (int i = 0; i < temp_val.size(); i++) {
              SV temp_sv = temp_val[i];
              Datalib::Tools::scale(temp_sv.data(), coeff1);
              Datalib::Tools::addScaled(new_val[i].data(), temp_sv.data(), 1.0);
            }
          }
        } // lin_combin_itt1

      } else if (lin_combin_needed[1]) // Need linear combination for npt[2]
      {
        std::cout << "   Taking linear combination for npt[2]:" << std::endl;
        lin_combin_map_t::const_iterator map_itt2 =
            lin_combin_map.find(CGCstrings[1]);
        std::vector<lin_combin_t> lin_combin_vec2 = map_itt2->second;
        K new_key = original_key;

        for (std::vector<lin_combin_t>::iterator lin_combin_itt2 =
                 lin_combin_vec2.begin();
             lin_combin_itt2 != lin_combin_vec2.end(); ++lin_combin_itt2) {
          std::complex<double> coeff2 = (*lin_combin_itt2).val;
          if (!original_key.npoint[2].irrep.creation_op)
            coeff2 = std::conj(coeff2); // Take conj if annihilation op
          new_key.npoint[2].irrep.op.CGs[1].target.irrep =
              (*lin_combin_itt2).combin;

          std::cout << "      npt[2] " << (*lin_combin_itt2).combin
                    << " coeff= " << coeff2 << std::endl;

          // Build up the linear combination with appropriate coefficient
          if (lin_combin_itt2 == lin_combin_vec2.begin()) {
            std::vector<SV> temp_val =
                printKeyValueVec<K, std::vector<std::complex<double>>>(new_key,
                                                                       dbold);
            for (int i = 0; i < temp_val.size(); i++) {
              SV temp_sv = temp_val[i];
              Datalib::Tools::scale(temp_sv.data(), coeff2);
              new_val.push_back(temp_sv);
            }
          } else {
            std::vector<SV> temp_val =
                printKeyValueVec<K, std::vector<std::complex<double>>>(new_key,
                                                                       dbold);
            for (int i = 0; i < temp_val.size(); i++) {
              SV temp_sv = temp_val[i];
              Datalib::Tools::scale(temp_sv.data(), coeff2);
              Datalib::Tools::addScaled(new_val[i].data(), temp_sv.data(), 1.0);
            }
          }
        } // lin_combin_itt2

      } else // Do not need linear combination
      {
        std::cout << "   No linear combination needed" << std::endl;
        new_val = val;
      }

      // std::cout << "key: " << original_key << "\n";

      // Insert the new key
      int ret;
      if ((ret = dbnew.insert(Datalib::Storage::SerialDBKey<K>(original_key),
                              new_val)) != 0) {
        std::cerr << __func__ << ": error inserting key\n";
        exit(1);
      }

    } // loop over keys

    // close the databases
    dbnew.close();
    dbold.close();
  } catch (const std::string &e) {
    std::cerr << argv[0] << ": error message: " << e << std::endl;
    exit(1);
  }

  return 0;
}

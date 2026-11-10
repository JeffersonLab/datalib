#include <datalib/legacy/hadron_convert_corr_util.h>
// -*- C++ -*-
/*! \file
 *  \brief Convert correlators from SU(2) specific keys to SU(N) specific keys
 */

#include <datalib/graph/hadron_sun_class_npart_npt_corr.h>
#include <datalib/graph/naming.h>
#include <datalib/graph/operator_name.h>
#include <datalib/legacy/hadron_npart_npt_corr.h>

namespace Hadron {
  //----------------------------------------------------------------------------
  //! Convert a correlator key
  KeyHadronSUNClassNPartIrrepOp_t
  convertHadronNPart(const KeyHadronNPartIrrepOp_t &irrep) {
    KeyHadronSUNClassNPartIrrepOp_t dest;

    // dest.ops = irrep.ops;
    for (const auto &old_op : irrep.ops.ref()) {
      KeyParticleClassOp_t new_op;
      new_op.name = old_op.name;
      new_op.smear = old_op.smear;
      new_op.mom_class = old_op.mom_type;
      dest.ops.ref().push_back(new_op);
    }

    for (auto old = irrep.CGs.ref().begin(); old != irrep.CGs.ref().end();
         ++old) {
      KeyHadronSUNClassNPartIrrepOp_t::CGPair_t cg;

      cg.left = old->left;
      cg.right = old->right;

      cg.target.slot = old->target.slot;
      cg.target.mom_class = old->target.mom_type;

      if (1) {
        std::ostringstream os;
        os << old->target.irrep << "," << old->target.embed;
        cg.target.irrep = os.str();
      }

      if (1) {
        std::ostringstream os;
        os << (old->target.twoI + 1)
           << ",1"; // All SU(2) constructions are in the first embedding
        cg.target.F = os.str();
      }

      dest.CGs.push_back(cg);
    }

    return dest;
  }

  //----------------------------------------------------------------------------
  //! Convert a correlator key
  KeyHadronSUNClassNPartIrrep_t
  convertHadronNPart(const KeyHadronNPartIrrep_t &orig) {
    KeyHadronSUNClassNPartIrrep_t dest;

    dest.creation_op = orig.creation_op;
    dest.smearedP = orig.smearedP;

    dest.flavor = KeyCGCSU3_t(0, 0, orig.twoI_z);
    dest.irrep_mom = KeyCGCIrrepMom_t(orig.row, orig.mom);

    dest.op = convertHadronNPart(orig.op);

    // Set the target flavor components depending on the operator constructions
    dest.flavor.twoI_z = orig.twoI_z;

    if (dest.op.ops.size() == 0) {
      std::cerr << __func__ << ": error - must have some operators\n";
      exit(1);
    }
    if (dest.op.ops.size() == 1) {
      // Single particle - flavor components determined by the operator
      dest.flavor.twoI = getTwoIsospin(dest.op.ops[1].name);
      dest.flavor.threeY = getThreeY(dest.op.ops[1].name);
      dest.flavor.twoI_z = orig.twoI_z;
    } else {
      int max_cg = dest.op.CGs.size();
      dest.flavor.twoI = orig.op.CGs[max_cg].target.twoI;

      dest.flavor.threeY = 0;
      for (auto op = dest.op.ops.ref().begin(); op != dest.op.ops.ref().end();
           ++op)
        dest.flavor.threeY += getThreeY(op->name);
    }

    return dest;
  }

  //----------------------------------------------------------------------------
  //! Convert a correlator key
  KeyHadronSUNClassNPartNPtCorr_t
  convertHadronNPart(const KeyHadronNPartNPtCorr_t &corr) {
    KeyHadronSUNClassNPartNPtCorr_t dest;

    for (auto old = corr.npoint.ref().begin(); old != corr.npoint.ref().end();
         ++old) {
      KeyHadronSUNClassNPartNPtCorr_t::NPoint_t npt;

      npt.t_slice = old->t_slice;
      npt.irrep = convertHadronNPart(old->irrep);

      dest.npoint.push_back(npt);
    }

    return dest;
  }

} // end namespace Hadron

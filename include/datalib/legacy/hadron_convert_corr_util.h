// -*- C++ -*-
/*! \file
 *  \brief Convert correlators from SU(2) specific keys to SU(N) specific keys
 */

#ifndef __hadron_convert_corr_util_h__
#define __hadron_convert_corr_util_h__

#include <datalib/graph/hadron_sun_class_npart_npt_corr.h>
#include <datalib/legacy/hadron_npart_npt_corr.h>

namespace Hadron {
  //----------------------------------------------------------------------------
  //! Convert a correlator key
  Hadron::KeyHadronSUNClassNPartIrrepOp_t
  convertHadronNPart(const Hadron::KeyHadronNPartIrrepOp_t &corr);

  //----------------------------------------------------------------------------
  //! Convert a correlator key
  Hadron::KeyHadronSUNClassNPartIrrep_t
  convertHadronNPart(const Hadron::KeyHadronNPartIrrep_t &corr);

  //----------------------------------------------------------------------------
  //! Convert a correlator key
  Hadron::KeyHadronSUNClassNPartNPtCorr_t
  convertHadronNPart(const Hadron::KeyHadronNPartNPtCorr_t &corr);

} // end namespace Hadron

#endif

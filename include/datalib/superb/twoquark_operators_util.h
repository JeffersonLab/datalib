// -*- C++ -*-
/*! \file
 * \brief Two quark operators
 */

#ifndef __redstar_datalib_twoquark_operators_util_h__
#define __redstar_datalib_twoquark_operators_util_h__


#include "datalib/superb/types.h"
#include "datalib/graph/single_hadron_coeffs.h"

#include <vector>

namespace Hadron
{

  //----------------------------------------------------------------------------------
  Array<int> constructMom4d(const Array<int>& mom, int decay_dir = 3);

  //----------------------------------------------------------------------------------
  //! Make a momentum phase from a phase mask
  std::complex<double> momPhase(const Array<int>& nf, const Array<int>& mask, const Array<int>& latt_size);

  //----------------------------------------------------------------------------------
  //! Compute integer phase associated with a reverse displacement
  Array<int> dispMomMask(const std::vector<int>& path);

} // namespace Hadron

#endif

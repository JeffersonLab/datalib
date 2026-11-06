// -*- C++ -*-
/*! \file
 * \brief Unsmeared meson elementals
 */

#ifndef __redstar_datalib_unsmeared_meson_elem_h__
#define __redstar_datalib_unsmeared_meson_elem_h__

#include "datalib/superb/types.h"
#include "datalib/codec/binary.h"
#include "datalib/model/arrays.h"


#include <string>
#include <vector>
#include <iostream>
#include <complex>

namespace Hadron
{

  //----------------------------------------------------------------------------
  //! Unsmeared meson operator
  struct KeyUnsmearedMesonElementalOperator_t
  { 
    int                t_sink;       /*!< Time sink */
    int                t_slice;      /*!< Meson operator time slice */
    int                t_source;     /*!< Time source */
    int                colorvec_src; /*!< Colorvec source component */
    int                gamma;        /*!< The "gamma" in Gamma(gamma) */
    bool               derivP;       /*!< Uses derivatives */
    std::vector<int>   displacement; /*!< Displacement dirs of right colorvector */
    Array<int>         mom;          /*!< D-1 momentum of this operator */
    std::string        mass;         /*!< Mass label */
  };

  //! Meson operator
  struct ValUnsmearedMesonElementalOperator_t
  {
    Array3d< std::complex<double> >  op;          /*!< Colorvector source and spin sink */
  };


  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const KeyUnsmearedMesonElementalOperator_t& d);
	
  //----------------------------------------------------------------------------
  //! KeyUnsmearedMesonElementalOperator reader
  void read(BinaryReader& bin, KeyUnsmearedMesonElementalOperator_t& param);

  //! UnsmearedMesonElementalOperator write
  void write(BinaryWriter& bin, const KeyUnsmearedMesonElementalOperator_t& param);

  //! UnsmearedMesonElementalOperator reader
  void read(XMLReader& xml, const std::string& path, KeyUnsmearedMesonElementalOperator_t& param);

  //! UnsmearedMesonElementalOperator writer
  void write(XMLWriter& xml, const std::string& path, const KeyUnsmearedMesonElementalOperator_t& param);

  //----------------------------------------------------------------------------
  //! UnsmearedMesonElementalOperator reader
  void read(BinaryReader& bin, ValUnsmearedMesonElementalOperator_t& param);

  //! UnsmearedMesonElementalOperator write
  void write(BinaryWriter& bin, const ValUnsmearedMesonElementalOperator_t& param);

} // namespace HadronSUN

#endif

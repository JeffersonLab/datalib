// -*- C++ -*-
/*! \file
 * \brief Glueball elemental operators
 */

#ifndef __glueball_elem_type_h__
#define __glueball_elem_type_h__

#include "datalib/superb/types.h"
#include <datalib/model/xml_array.h>
#include <datalib/model/xml_array2d.h>


#include <iostream>
#include <complex>

namespace Hadron
{
  using namespace XMLArray;

  //----------------------------------------------------------------------------
  //! Key for Meson operator
  struct KeyGlueElementalOperator_t
  {
    int              t_slice;             /*!< Glueball operator time slice */ 
    int              left;                /*!< Left B field direction */ 
    int              right;               /*!< Right B field direction */ 
    std::vector<int> displacement;        /*!< Displacement dirs of right colorvector */
    Array<int>       mom;                 /*!< D-1 momentum of this operator */

    bool operator==(const KeyGlueElementalOperator_t& ) const = default;
  };

  //! Value for Glue operator
  struct ValGlueElementalOperator_t
  {
    Array< std::complex<double> >  op;                /*!< Colorvector source/sink with momentum projection */
  };


  //----------------------------------------------------------------------------
  //! Used for error output
  std::ostream& operator<<(std::ostream& os, const KeyGlueElementalOperator_t& d);
	
  //----------------------------------------------------------------------------
  //! Read a key
  void read(XMLReader& xml, const std::string& path, KeyGlueElementalOperator_t& param);

  //! Write a key
  void write(XMLWriter& xml, const std::string& path, const KeyGlueElementalOperator_t& param);

  //----------------------------------------------------------------------------
  //! KeyGlueElementalOperator reader
  void read(BinaryReader& bin, KeyGlueElementalOperator_t& param);

  //! KeyGlueElementalOperator writer
  void write(BinaryWriter& bin, const KeyGlueElementalOperator_t& param);

  //----------------------------------------------------------------------------
  //! ValGlueElementalOperator reader
  void read(BinaryReader& bin, ValGlueElementalOperator_t& param);

  //! ValGlueElementalOperator reader
  void write(BinaryWriter& bin, const ValGlueElementalOperator_t& param);

} // namespace ColorVec

#endif
